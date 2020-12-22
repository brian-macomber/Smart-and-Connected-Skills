
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_err.h"

#define LEDC_HS_TIMER LEDC_TIMER_0
#define LEDC_HS_MODE LEDC_HIGH_SPEED_MODE
#define LEDC_HS_CH0_GPIO (12)
#define LEDC_HS_CH0_CHANNEL LEDC_CHANNEL_0

#define LEDC_LS_TIMER LEDC_TIMER_1
#define LEDC_LS_MODE LEDC_LOW_SPEED_MODE

#define LEDC_TEST_CH_NUM (1)
#define LEDC_TEST_DUTY (4000)
#define LEDC_TEST_FADE_TIME (3000)

#define LEDC_STEP_SIZE (250)

void app_main(void)
{
    int ch = 0;

    /*
     * Prepare and set configuration of timers
     * that will be used by LED Controller
     */
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_13_BIT, // resolution of PWM duty
        .freq_hz = 5000,                      // frequency of PWM signal
        .speed_mode = LEDC_LS_MODE,           // timer mode
        .timer_num = LEDC_LS_TIMER,           // timer index
        .clk_cfg = LEDC_AUTO_CLK,             // Auto select the source clock
    };
    // Set configuration of timer0 for high speed channels
    ledc_timer_config(&ledc_timer);

    // Prepare and set configuration of timer1 for low speed channels
    ledc_timer.speed_mode = LEDC_HS_MODE;
    ledc_timer.timer_num = LEDC_HS_TIMER;
    ledc_timer_config(&ledc_timer);
    /*
     * Prepare individual configuration
     * for each channel of LED Controller
     * by selecting:
     * - controller's channel number
     * - output duty cycle, set initially to 0
     * - GPIO number where LED is connected to
     * - speed mode, either high or low
     * - timer servicing selected channel
     *   Note: if different channels use one timer,
     *         then frequency and bit_num of these channels
     *         will be the same
     */
    ledc_channel_config_t ledc_channel[LEDC_TEST_CH_NUM] = {
        {.channel = LEDC_HS_CH0_CHANNEL,
         .duty = 0,
         .gpio_num = LEDC_HS_CH0_GPIO,
         .speed_mode = LEDC_HS_MODE,
         .hpoint = 0,
         .timer_sel = LEDC_HS_TIMER},
    };

    // Set LED Controller with previously prepared configuration
    ledc_channel_config(&ledc_channel[ch]);

    // Initialize fade service.
    ledc_fade_func_install(0);

    while (1)
    {
        // only 90%
        for (int i = 0; i <= 9; i++)
        {
            printf("%%%d Intensity\n", i * 10);
            ledc_set_duty(ledc_channel[ch].speed_mode, ledc_channel[ch].channel, (LEDC_TEST_DUTY / 10) * i);
            ledc_update_duty(ledc_channel[ch].speed_mode, ledc_channel[ch].channel);
            vTaskDelay(LEDC_STEP_SIZE / portTICK_PERIOD_MS);
        }
        for (int i = 9; i >= 0; i--)
        {
            printf("%%%d Intensity\n", i * 10);
            ledc_set_duty(ledc_channel[ch].speed_mode, ledc_channel[ch].channel, (LEDC_TEST_DUTY / 10) * i);
            ledc_update_duty(ledc_channel[ch].speed_mode, ledc_channel[ch].channel);
            vTaskDelay(LEDC_STEP_SIZE / portTICK_PERIOD_MS);
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
