/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

/* Can use project configuration menu (idf.py menuconfig) to choose the GPIO to blink,
   or you can edit the following line and set a number here.
*/
#define BLINK_GPIO_1 26
#define BLINK_GPIO_2 25
#define BLINK_GPIO_3 33
#define BLINK_GPIO_4 27

void app_main(void)
{
    /* Configure the IOMUX register for pad BLINK_GPIO (some pads are
       muxed to GPIO on reset already, but some default to other
       functions and need to be switched to GPIO. Consult the
       Technical Reference for a list of pads and their default
       functions.)
    */
    gpio_reset_pin(BLINK_GPIO_1);
    gpio_reset_pin(BLINK_GPIO_2);
    gpio_reset_pin(BLINK_GPIO_3);
    gpio_reset_pin(BLINK_GPIO_4);

    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO_1, GPIO_MODE_OUTPUT);
    gpio_set_direction(BLINK_GPIO_2, GPIO_MODE_OUTPUT);
    gpio_set_direction(BLINK_GPIO_3, GPIO_MODE_OUTPUT);
    gpio_set_direction(BLINK_GPIO_4, GPIO_MODE_OUTPUT);

    int i;

    while (1)
    {
        int nums[4];
        int gp1 = 0, gp2 = 0, gp3 = 0, gp4 = 0;
        //set all back to 0
        gpio_set_level(BLINK_GPIO_1, gp1);
        gpio_set_level(BLINK_GPIO_2, gp2);
        gpio_set_level(BLINK_GPIO_3, gp3);
        gpio_set_level(BLINK_GPIO_4, gp4);

        for (i = 0; i <= 15; i++)
        {
            nums[0] = i & 0x1;
            nums[1] = i & 0x2;
            nums[2] = i & 0x4;
            nums[3] = i & 0x8;

            for (int j = 0; j < 4; j++)
            {
                printf("%d ", nums[j]);
            }
            printf("\n");
            //set each LED

            //led 1
            if (nums[0] > 0)
            {
                if (gp1 == 0)
                {
                    gp1 = 1;
                    gpio_set_level(BLINK_GPIO_1, gp1);
                }
            }
            else
            {
                if (gp1 == 1)
                {
                    gp1 = 0;
                    gpio_set_level(BLINK_GPIO_1, gp1);
                }
            }

            //led 2
            if (nums[1] > 0)
            {
                if (gp2 == 0)
                {
                    gp2 = 1;
                    gpio_set_level(BLINK_GPIO_2, gp2);
                }
            }
            else
            {
                if (gp2 == 1)
                {
                    gp2 = 0;
                    gpio_set_level(BLINK_GPIO_2, gp2);
                }
            }

            //led 3
            if (nums[2] > 0)
            {
                if (gp3 == 0)
                {
                    gp3 = 1;
                    gpio_set_level(BLINK_GPIO_3, gp3);
                }
            }
            else
            {
                if (gp3 == 1)
                {
                    gp3 = 0;
                    gpio_set_level(BLINK_GPIO_3, gp3);
                }
            }

            //led 4
            if (nums[3] > 0)
            {
                if (gp4 == 0)
                {
                    gp4 = 1;
                    gpio_set_level(BLINK_GPIO_4, gp4);
                }
            }
            else
            {
                if (gp4 == 1)
                {
                    gp4 = 0;
                    gpio_set_level(BLINK_GPIO_4, gp4);
                }
            }

            vTaskDelay(100);
        }
    }

    // while (1)
    // {
    //     /* Blink on (output high) */
    //     printf("Turning on the first LED\n");
    //     gpio_set_level(BLINK_GPIO_1, 1);
    //     vTaskDelay(500 / portTICK_PERIOD_MS);
    //     /* Blink off (output low) */
    //     printf("Turning off the first LED\n");
    //     gpio_set_level(BLINK_GPIO_1, 0);
    //     vTaskDelay(500 / portTICK_PERIOD_MS);

    //     /* Blink on (output high) */
    //     printf("Turning on the second LED\n");
    //     gpio_set_level(BLINK_GPIO_2, 1);
    //     vTaskDelay(500 / portTICK_PERIOD_MS);
    //     /* Blink off (output low) */
    //     printf("Turning off the second LED\n");
    //     gpio_set_level(BLINK_GPIO_2, 0);
    //     vTaskDelay(500 / portTICK_PERIOD_MS);

    //     /* Blink on (output high) */
    //     printf("Turning on the third LED\n");
    //     gpio_set_level(BLINK_GPIO_3, 1);
    //     vTaskDelay(500 / portTICK_PERIOD_MS);
    //     /* Blink off (output low) */
    //     printf("Turning off the third LED\n");
    //     gpio_set_level(BLINK_GPIO_3, 0);
    //     vTaskDelay(500 / portTICK_PERIOD_MS);

    //     /* Blink on (output high) */
    //     printf("Turning on the fourth LED\n");
    //     gpio_set_level(BLINK_GPIO_4, 1);
    //     vTaskDelay(500 / portTICK_PERIOD_MS);
    //     /* Blink off (output low) */
    //     printf("Turning off the fourth LED\n");
    //     gpio_set_level(BLINK_GPIO_4, 0);
    //     vTaskDelay(500 / portTICK_PERIOD_MS);
    // }
}
