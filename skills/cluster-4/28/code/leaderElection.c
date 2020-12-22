
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/rmt.h"
#include "soc/rmt_reg.h"
#include "driver/uart.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"
#include <sys/param.h>
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include "esp_types.h"
#include "freertos/queue.h"
#include <ctype.h>
#include <stdlib.h>
#include "esp_vfs_dev.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "freertos/event_groups.h"

#define MAX 100

// LED Output pins definitions
#define BLUEPIN 14
#define GREENPIN 32
#define REDPIN 15
#define ONBOARD 13
#define TIMER_DIVIDER 16                             //  Hardware timer clock divider
#define TIMER_SCALE (TIMER_BASE_CLK / TIMER_DIVIDER) // to seconds
#define TIMER_INTERVAL_1_SEC (1)
#define TIMER_INTERVAL_10_SEC (10)
#define TEST_WITH_RELOAD 1 // Testing will be done with auto reload

//Timer Variables
#define ELECTION_TIMEOUT 6
#define LEADER_TIMEOUT 30
#define HEARTBEAT 1
#define UDP_TIMER 3

int UDPFlag = 0;

int timeout = ELECTION_TIMEOUT;
int udpTimer = UDP_TIMER;

static const char *TAG = "example";
static const char *payload = "";

typedef enum
{
    ELECTION_STATE,
    LEADER_STATE,
    FOLLOWER_STATE
} state_e;

char status[MAX] = "No_Leader";
char myID[MAX] = "1";
char myID_CHAR = '1';
char deviceAge[MAX] = "New";
char data[MAX];
char leaderHeartbeat[MAX] = "Dead";
state_e deviceState = LEADER_STATE;
char transmitting[MAX] = "Yes";
char leaderIP[MAX] = "";

char IPtable[2][20] = {
    "192.168.7.245",
    "192.168.7.176"};

#define NUM_FOBS 2

// Mutex (for resources), and Queues (for button)
SemaphoreHandle_t mux = NULL;
static xQueueHandle timer_queue;

// A simple structure to pass "events" to main task
typedef struct
{
    int flag; // flag for enabling stuff in timer task
} timer_event_t;

// System tags
static const char *TAG_SYSTEM = "system"; // For debug logs

// ISR handler
void IRAM_ATTR timer_group0_isr(void *para)
{

    // Prepare basic event data, aka set flag
    timer_event_t evt;
    evt.flag = 1;

    // Clear the interrupt, Timer 0 in group 0
    TIMERG0.int_clr_timers.t0 = 1;

    // After the alarm triggers, we need to re-enable it to trigger it next time
    TIMERG0.hw_timer[TIMER_0].config.alarm_en = TIMER_ALARM_EN;

    // Send the event data back to the main program task
    xQueueSendFromISR(timer_queue, &evt, NULL);
}
// Init Functions //////////////////////////////////////////////////////////////
// GPIO init for LEDs
static void led_init()
{
    gpio_pad_select_gpio(BLUEPIN);
    gpio_pad_select_gpio(GREENPIN);
    gpio_pad_select_gpio(REDPIN);

    gpio_set_direction(BLUEPIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(GREENPIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(REDPIN, GPIO_MODE_OUTPUT);
}

// Configure timer
static void alarm_init()
{
    // Select and initialize basic parameters of the timer
    timer_config_t config;
    config.divider = TIMER_DIVIDER;
    config.counter_dir = TIMER_COUNT_UP;
    config.counter_en = TIMER_PAUSE;
    config.alarm_en = TIMER_ALARM_EN;
    config.intr_type = TIMER_INTR_LEVEL;
    config.auto_reload = TEST_WITH_RELOAD;
    timer_init(TIMER_GROUP_0, TIMER_0, &config);

    // Timer's counter will initially start from value below
    timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0x00000000ULL);

    // Configure the alarm value and the interrupt on alarm
    timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, TIMER_INTERVAL_1_SEC * TIMER_SCALE);
    timer_enable_intr(TIMER_GROUP_0, TIMER_0);
    timer_isr_register(TIMER_GROUP_0, TIMER_0, timer_group0_isr,
                       (void *)TIMER_0, ESP_INTR_FLAG_IRAM, NULL);

    // Start timer
    timer_start(TIMER_GROUP_0, TIMER_0);
}

////////////////////////////////////////////////////////////////////////////////
// Tasks ///////////////////////////////////////////////////////////////////////

// LED task to light LED based on traffic state
void led_task()
{
    while (1)
    {

        switch (deviceState)
        {
        case LEADER_STATE: //Green
            gpio_set_level(GREENPIN, 1);
            gpio_set_level(REDPIN, 0);
            gpio_set_level(BLUEPIN, 0);
            break;
        case FOLLOWER_STATE: // Red
            gpio_set_level(GREENPIN, 0);
            gpio_set_level(REDPIN, 1);
            gpio_set_level(BLUEPIN, 0);
            break;
        case ELECTION_STATE: // Blue
            gpio_set_level(GREENPIN, 0);
            gpio_set_level(REDPIN, 0);
            gpio_set_level(BLUEPIN, 1);
            break;
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

//sets up the timer to trigger every 1 second
static void timer_evt_task(void *arg)
{
    while (1)
    {
        // Create dummy structure to store structure from queue
        timer_event_t evt;

        // Transfer from queue
        xQueueReceive(timer_queue, &evt, portMAX_DELAY);

        // Do something if triggered!
        if (evt.flag == 1)
        {
            printf("timeout: %d\t udptimer: %d\n", timeout, udpTimer);
            timeout--;
            udpTimer--;
            if (timeout <= 0 && (deviceState == ELECTION_STATE || deviceState == FOLLOWER_STATE)) // if timed out waiting for a leader, elect YOURSELF
            {
                printf("GOING TO LEADER STATE\n");
                deviceState = LEADER_STATE; // Change to leader state (Last remaining device in election state)
            }

            if (deviceState == LEADER_STATE)
            {
                if (udpTimer < 0)
                {
                    udpTimer = HEARTBEAT;
                }
                strcpy(leaderHeartbeat, "Alive"); // Change leaderHeartbeat parameter in payload to "Alive" upon being elected leader
                strcpy(status, "Leader");         // Change status to "Leader"
            }
        }
    }
}

//UDP server port
#define PORT 1131
static void udp_server_task(void *pvParameters)
{
    char rx_buffer[128];
    char addr_str[128];
    int addr_family = (int)pvParameters;
    int ip_protocol = 0;
    struct sockaddr_in6 dest_addr;

    while (1)
    {
        if (addr_family == AF_INET)
        {
            struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
            dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
            dest_addr_ip4->sin_family = AF_INET;
            dest_addr_ip4->sin_port = htons(PORT);
            ip_protocol = IPPROTO_IP;
        }
        else if (addr_family == AF_INET6)
        {
            bzero(&dest_addr.sin6_addr.un, sizeof(dest_addr.sin6_addr.un));
            dest_addr.sin6_family = AF_INET6;
            dest_addr.sin6_port = htons(PORT);
            ip_protocol = IPPROTO_IPV6;
        }

        int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
        if (sock < 0)
        {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket created");

#if defined(CONFIG_EXAMPLE_IPV4) && defined(CONFIG_EXAMPLE_IPV6)
        if (addr_family == AF_INET6)
        {
            // Note that by default IPV6 binds to both protocols, it is must be disabled
            // if both protocols used at the same time (used in CI)
            int opt = 1;
            setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
            setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt));
        }
#endif

        int err = bind(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err < 0)
        {
            ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        }
        ESP_LOGI(TAG, "Socket bound, port %d", PORT);

        while (1)
        {
            char recv_status[MAX];
            char recv_ID[MAX];
            char recv_deviceAge[MAX];
            char recv_leaderHeartbeat[MAX];
            char tempBuffer[MAX];

            int irMsgFlag = 0;

            ESP_LOGI(TAG, "Waiting for data");
            struct sockaddr_in6 source_addr; // Large enough for both IPv4 or IPv6
            socklen_t socklen = sizeof(source_addr);
            int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);

            // Error occurred during receiving
            if (len < 0)
            {
                ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
                break;
            }
            // Data received
            else
            {
                // Get the sender's ip address as string
                if (source_addr.sin6_family == PF_INET)
                {
                    inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);
                }
                else if (source_addr.sin6_family == PF_INET6)
                {
                    inet6_ntoa_r(source_addr.sin6_addr, addr_str, sizeof(addr_str) - 1);
                }

                rx_buffer[len] = 0;

                strcpy(tempBuffer, rx_buffer);
                // Returns first token
                char *token = strtok(rx_buffer, ",");

                printf("%s\n", token);

                for (int i = 0; i < 4; i++)
                {
                    if (i == 0)
                    {

                        strcpy(recv_status, token);
                    }
                    else if (i == 1)
                    {
                        if (strcmp(token, "R") == 0 || strcmp(token, "B") == 0 || strcmp(token, "G") == 0)
                        {
                            irMsgFlag = 1;
                            break;
                        }
                        strcpy(recv_ID, token);
                    }
                    else if (i == 2)
                    {
                        strcpy(recv_deviceAge, token);
                    }
                    else if (i == 3)
                    {
                        strcpy(recv_leaderHeartbeat, token);
                    }

                    token = strtok(NULL, ",");
                }

                char *token2 = strtok(tempBuffer, ",");

                // Check device state and handle incoming data accordingly
                int myID_num = atoi(myID);
                int recv_ID_num = atoi(recv_ID);
                if (deviceState == ELECTION_STATE)
                {
                    printf("ELECTION STATE\n");
                    strcpy(status, "No_Leader"); // Status is "No_Leader"
                    if (myID_num < recv_ID_num)
                    {
                        deviceState = ELECTION_STATE; // Stay in election state
                        strcpy(status, "No_Leader");  // Status is "No_Leader"
                        timeout = ELECTION_TIMEOUT;   // Reset election timeout
                    }
                    else if (myID_num > recv_ID_num)
                    {
                        deviceState = FOLLOWER_STATE; // Change to follower state
                        timeout = LEADER_TIMEOUT;     // Change timeout variable to leader timeout constant
                    }
                }
                else if (deviceState == FOLLOWER_STATE)
                {
                    printf("FOLLOWER STATE\n");
                    udpTimer = UDP_TIMER * 2;
                    if (strcmp(recv_deviceAge, "New") == 0)
                    {
                        deviceState = ELECTION_STATE; // Change to election state
                    }
                    else if (strcmp(recv_leaderHeartbeat, "Alive") == 0)
                    {
                        timeout = LEADER_TIMEOUT; // Reset leader timeout upon receiving leader heartbeat
                        strcpy(status, "Leader"); // Update status to leader upon receiving leader heartbeat
                        printf("leaderIP is originally %s \n", leaderIP);
                        strcpy(leaderIP, IPtable[recv_ID_num]);
                        printf("leaderIP is now changed to %s with the recv_ID_num of %d \n", leaderIP, recv_ID_num);
                    }
                    else if (timeout <= 0)
                    {
                        deviceState = ELECTION_STATE; // Change to election state
                        timeout = ELECTION_TIMEOUT;   // Change timeout variable to election timeout constant
                    }
                }
                else if (deviceState == LEADER_STATE)
                {
                    if (strcmp(recv_deviceAge, "New") == 0)
                    {
                        deviceState = ELECTION_STATE; // Change state to election state
                        timeout = ELECTION_TIMEOUT;   // Change timeout variable to election timeout constant
                    }
                    if (strcmp(recv_status, "Leader") == 0) // avoid weird middle state when multiple are deeming themselves leader
                    {
                        deviceState = ELECTION_STATE;
                    }
                }

                memset(recv_deviceAge, 0, sizeof(data));
                memset(recv_ID, 0, sizeof(data));
                memset(recv_status, 0, sizeof(data));
                memset(recv_leaderHeartbeat, 0, sizeof(data));

                //ERROR CHECKING
                int err = sendto(sock, payload, len, 0, (struct sockaddr *)&source_addr, sizeof(source_addr));
                if (err < 0)
                {
                    ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                    break;
                }
            }
        }

        if (sock != -1)
        {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
    }
    vTaskDelete(NULL);
}

//UDP client
char HOST_IP_ADDR[MAX] = "192.168.7.245";
#define NODE_IP_ADDR "192.168.7.196"
#define PORT2 1131

static void
udp_client_task(void *pvParameters)
{
    // char rx_buffer[128];
    // char host_ip[] = HOST_IP_ADDR;
    int addr_family = 0;
    int ip_protocol = 0;

    while (1)
    {

        struct sockaddr_in dest_addr;
        dest_addr.sin_addr.s_addr = inet_addr(HOST_IP_ADDR);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(PORT2);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;

        int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
        if (sock < 0)
        {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket created, sending to %s:%d", HOST_IP_ADDR, PORT2);

        while (1)
        {
            if (udpTimer <= 0)
            {
                for (int i = 0; i < NUM_FOBS; i++)
                {
                    if (IPtable[i] != HOST_IP_ADDR)
                    {

                        strcpy(HOST_IP_ADDR, IPtable[i]);
                        dest_addr.sin_addr.s_addr = inet_addr(HOST_IP_ADDR);

                        memset(data, 0, sizeof(data));
                        strcat(data, status);
                        strcat(data, ",");
                        strcat(data, myID);
                        strcat(data, ",");
                        strcat(data, deviceAge);
                        strcat(data, ",");
                        strcat(data, leaderHeartbeat);

                        payload = data;
                        int err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
                        strcpy(deviceAge, "Old");
                        if (err < 0)
                        {
                            ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                            break;
                        }
                        printf("sending to ip addess %s \n", HOST_IP_ADDR);
                        ESP_LOGI(TAG, "Message sent");
                        udpTimer = UDP_TIMER;
                    }
                }
            }

            vTaskDelay(100 / portTICK_PERIOD_MS);
        }

        if (sock != -1)
        {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }

        vTaskDelete(NULL);
    }
}

// ----------------------------- WiFi -------------------------------------//
#define EXAMPLE_ESP_WIFI_SSID "SUPREME-WiFi"
#define EXAMPLE_ESP_WIFI_PASS "gh7a84VSeW7TgYC"

#define EXAMPLE_ESP_MAXIMUM_RETRY 5
static EventGroupHandle_t s_wifi_event_group;
/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
static const char *WIFI_TAG = "wifi station";

static int s_retry_num = 0;
// -----------------------------------------------------------------------//
// ----------------------------- WiFi -------------------------------------//
static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY)
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(WIFI_TAG, "retry to connect to the AP");
        }
        else
        {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(WIFI_TAG, "connect to the AP fail");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(WIFI_TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
            /* Setting a password implies station will connect to all security modes including WEP/WPA.
             * However these modes are deprecated and not advisable to be used. Incase your Access point
             * doesn't support WPA2, these mode can be enabled by commenting below line */
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,

            .pmf_cfg = {
                .capable = true,
                .required = false},
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(WIFI_TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI(WIFI_TAG, "connected to ap SSID:%s password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGI(WIFI_TAG, "Failed to connect to SSID:%s, password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    }
    else
    {
        ESP_LOGE(WIFI_TAG, "UNEXPECTED EVENT");
    }

    /* The event will not be processed after unregister */
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    vEventGroupDelete(s_wifi_event_group);
}

void initalizeWiFi()
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(WIFI_TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();
}

void app_main()
{
    initalizeWiFi();
    // Mutex for current values when sending
    mux = xSemaphoreCreateMutex();

    // Create a FIFO queue for timer-based events
    timer_queue = xQueueCreate(10, sizeof(timer_event_t));

    // Create task to handle timer-based events
    xTaskCreate(timer_evt_task, "timer_evt_task", 2048, NULL, configMAX_PRIORITIES, NULL);

    // Initialize all the things
    led_init();
    alarm_init();

    // Create tasks for receive, send, set gpio, and button
    xTaskCreate(led_task, "set_traffic_task", 1024 * 2, NULL, configMAX_PRIORITIES, NULL);
    xTaskCreate(udp_server_task, "udp_server", 4096, (void *)AF_INET, configMAX_PRIORITIES, NULL);
    xTaskCreate(udp_client_task, "udp_client", 4096, NULL, 5, NULL);
}
