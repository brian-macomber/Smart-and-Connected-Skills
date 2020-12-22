/*
   Author:  Brian Macomber
*/

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_attr.h"
#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"

//defines
#define DRIVE_MIN_PULSEWIDTH 900      //Minimum pulse width in microsecond - not using this since we aren't going backwards
#define DRIVE_NEUTRAL_PULSEWIDTH 1200 //Neutral pulse width in microsecond
#define DRIVE_MAX_PULSEWIDTH 1400     //Maximum pulse width in microsecond

// these were tuned based on my car
#define STEERING_MIN_PULSEWIDTH 1100 //Minimum pulse width in microsecond
#define STEERING_MAX_PULSEWIDTH 1800 //Maximum pulse width in microsecond
#define STEERING_MAX_DEGREE 50       //Maximum angle in degree upto which servo can rotate

static void mcpwm_example_gpio_initialize(void)
{
    printf("initializing mcpwm servo control gpio......\n");
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, 12); //Set GPIO 12 as PWM0A, ESC is connected
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, 25); //Set GPIO 25 as PWM0B, Steering servo is connected (A1)
}

void pwm_init()
{
    //1. mcpwm gpio initialization
    mcpwm_example_gpio_initialize();

    //2. initial mcpwm configuration
    printf("Configuring Initial Parameters of mcpwm......\n");
    mcpwm_config_t pwm_config;
    pwm_config.frequency = 50; //frequency = 50Hz, i.e. for every servo motor time period should be 20ms
    pwm_config.cmpr_a = 0;     //duty cycle of PWMxA = 0
    pwm_config.cmpr_b = 0;     //duty cycle of PWMxb = 0
    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config); //Configure PWM0A & PWM0B with above settings
}

void calibrateESC()
{
    vTaskDelay(3000 / portTICK_PERIOD_MS);                                                    // Give yourself time to turn on crawler
    mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, DRIVE_NEUTRAL_PULSEWIDTH); // set neutral for 3 sec calibration
    vTaskDelay(3100 / portTICK_PERIOD_MS);                                                    //wait 3 seconds to finish calibration
}

// Controls the ESC as a servo using mcpwm unit 0 - A
void drive_control(void *arg)
{
    uint32_t count;
    while (1)
    {
        // start at neutral, go forward to 900 microsecond pwm
        for (count = DRIVE_NEUTRAL_PULSEWIDTH; count < DRIVE_MAX_PULSEWIDTH; count += 5)
        {
            mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, count);
            vTaskDelay(100 / portTICK_RATE_MS);
        }

        vTaskDelay(5000 / portTICK_RATE_MS);

        // go back to neutral at 1200 microsecond pwm
        for (count = DRIVE_MAX_PULSEWIDTH; count > DRIVE_NEUTRAL_PULSEWIDTH; count -= 5)
        {
            mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, count);
            vTaskDelay(100 / portTICK_RATE_MS);
        }

        mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, DRIVE_NEUTRAL_PULSEWIDTH);
        vTaskDelay(5000 / portTICK_RATE_MS);
    }
}

/**
 * @brief Use this function to calcute pulse width for per degree rotation
 *
 * @param  degree_of_rotation the angle in degree to which servo has to rotate
 *
 * @return
 *     - calculated pulse width
 */
static uint32_t steering_per_degree_init(uint32_t degree_of_rotation)
{
    uint32_t cal_pulsewidth = 0;
    cal_pulsewidth = (STEERING_MIN_PULSEWIDTH + (((STEERING_MAX_PULSEWIDTH - STEERING_MIN_PULSEWIDTH) * (degree_of_rotation)) / (STEERING_MAX_DEGREE)));
    return cal_pulsewidth;
}

// Controls the Steering servo using mcpwm unit 0 - B
void steering_control(void *arg)
{
    uint32_t angle, count;

    while (1)
    {

        for (count = 0; count < STEERING_MAX_DEGREE; count += 2)
        {
            printf("Angle of rotation: %d\n", count);
            angle = steering_per_degree_init(count);
            printf("pulse width: %dus\n", angle);
            mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, angle);
            vTaskDelay(100 / portTICK_RATE_MS); //Add delay, since it takes time for servo to rotate, generally 100ms/60degree rotation at 5V
        }

        vTaskDelay(1000 / portTICK_RATE_MS);

        for (count = STEERING_MAX_DEGREE; count > 0; count -= 2)
        {
            printf("Angle of rotation: %d\n", count);
            angle = steering_per_degree_init(count);
            printf("pulse width: %dus\n", angle);
            mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, angle);
            vTaskDelay(100 / portTICK_RATE_MS); //Add delay, since it takes time for servo to rotate, generally 100ms/60degree rotation at 5V
        }

        vTaskDelay(1000 / portTICK_RATE_MS);

        for (count = 0; count < (STEERING_MAX_DEGREE / 2); count += 2)
        {
            printf("Angle of rotation: %d\n", count);
            angle = steering_per_degree_init(count);
            printf("pulse width: %dus\n", angle);
            mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, angle);
            vTaskDelay(100 / portTICK_RATE_MS); //Add delay, since it takes time for servo to rotate, generally 100ms/60degree rotation at 5V
        }

        vTaskDelay(5000 / portTICK_RATE_MS);
    }
}

void app_main(void)
{
    pwm_init();
    calibrateESC();

    xTaskCreate(steering_control, "steering_control", 4096, NULL, 5, NULL);
    xTaskCreate(drive_control, "drive_control", 4096, NULL, 4, NULL);
}
