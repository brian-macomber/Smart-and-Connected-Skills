// #include <stdio.h>
// #include "driver/uart.h"

// #define EX_UART_NUM UART_NUM_0

// #define BUF_SIZE (1024)

// void app_main(void)
// {
//     uart_param_config(EX_UART_NUM, &uart_config);
//     uart_set_pin(EX_UART_NUM, ECHO_TEST_TXD, ECHO_TEST_RXD, ECHO_TEST_RTS, ECHO_TEST_CTS);
//     uart_driver_install(EX_UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0);

//     // Configure a temporary buffer for the incoming data
//     // uint8_t *data = (uint8_t *)malloc(BUF_SIZE);

//     int mode = 0, LED_f = 0;

//     while (1)
//     {
//         char buffer[10];
//         // Read data from the UART
//         // uart_read_bytes(EX_UART_NUM, buffer, BUF_SIZE, 20 / portTICK_RATE_MS);
//         gets(buffer);
//         // 3 different modes
//         if (buffer[0] == 's')
//         {
//             mode++;
//         }

//         if (mode % 3 == 0) //toggle LED
//         {
//             printf("Entered Toggle LED mode!\n");
//             if (buffer[0] == 't')
//             {
//                 if (LED_f == 0)
//                 {
//                     //set

//                     LED_f = 1;
//                 }
//                 else if (LED_f == 1)
//                 {

//                     LED_f = 0;
//                 }
//             }
//         }
//         if (mode % 3 == 1) //echo
//         {
//             printf("Entered Echo keyboard input to console!\n");
//         }
//         if (mode % 3 == 2) //conversion
//         {
//             printf("Entered Conversion to hex mode!\n");
//         }
//         vTaskDelay(50 / portTICK_RATE_MS);
//     }
// }
#include <stdio.h>
#include <string.h>
#include "driver/uart.h"
#include "esp_vfs_dev.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

#define ONBOARD_GPIO 13

void app_main()
{
    // install uart driver
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_0, 256, 0, 0, NULL, 0));

    esp_vfs_dev_uart_use_driver(UART_NUM_0);
    //select the onboard gpio
    gpio_pad_select_gpio(ONBOARD_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(ONBOARD_GPIO, GPIO_MODE_OUTPUT);

    int num = 0, mode = 0, LED_f = 0;

    while (1)
    {
        // initialzing here empties the str buffer each pass thru
        char str[5];
        gets(str);
        if (str[0] == 's')
        {
            mode++;
        }

        if (mode % 3 == 0)
        {
            printf("integer mode!\n");
            printf("Enter an integer:\n");

            num = atoi(str);
            printf("Hex: %X\n", num);
        }

        if (mode % 3 == 1)
        {

            // toggle LED mode
            printf("toggle mode\n");
            printf("Read: ");
            if (str[0] == 't')
            {
                // add the blinking here
                if (LED_f == 0)
                {
                    gpio_set_level(ONBOARD_GPIO, 1);
                    vTaskDelay(1000 / portTICK_PERIOD_MS);
                    LED_f = 1;
                    // printf("LED on!\n");
                }
                else if (LED_f == 1)
                {
                    gpio_set_level(ONBOARD_GPIO, 0);
                    vTaskDelay(1000 / portTICK_PERIOD_MS);
                    LED_f = 0;
                    // printf("LED off!\n");
                }
            }
        }

        if (mode % 3 == 2)
        {
            printf("echo mode\n");
            printf("echo: ");
            if (str[0] != '\0')
            {
                printf("%s\n", str);
            }
            else
            {
                printf("\n");
            }
        }
        vTaskDelay(50 / portTICK_RATE_MS);
    }
}