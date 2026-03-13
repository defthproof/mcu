#include "adc-task.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/adc.h"

static const uint ADC_GPIO_PIN = 26;
static const uint ADC_INPUT_CHANNEL = 0;
static const uint ADC_TEMP_SENSOR_CHANNEL = 4;

static adc_task_state_t adc_state = ADC_TASK_STATE_IDLE;
static uint64_t adc_ts = 0;
static uint32_t ADC_TASK_MEAS_PERIOD_US = 100000;

void adc_task_init(void)
{
    adc_init();
    adc_gpio_init(ADC_GPIO_PIN);
    adc_set_temp_sensor_enabled(true);

    adc_state = ADC_TASK_STATE_IDLE;
    adc_ts = 0;
}

float adc_task_get_voltage(void)
{
    adc_select_input(ADC_INPUT_CHANNEL);

    uint16_t voltage_counts = adc_read();
    float voltage_V = (3.3f * voltage_counts) / 4095.0f;

    return voltage_V;
}

float adc_task_get_temp(void)
{
    adc_select_input(ADC_TEMP_SENSOR_CHANNEL);

    uint16_t temp_counts = adc_read();
    float temp_V = (3.3f * temp_counts) / 4095.0f;
    float temp_C = 27.0f - (temp_V - 0.706f) / 0.001721f;

    return temp_C;
}

void adc_task_set_state(adc_task_state_t state)
{
    adc_state = state;

    if (state == ADC_TASK_STATE_RUN)
    {
        adc_ts = 0;
    }
}

void adc_task_handle(void)
{
    switch (adc_state)
    {
        case ADC_TASK_STATE_IDLE:
            break;

        case ADC_TASK_STATE_RUN:
            if (time_us_64() > adc_ts)
            {
                adc_ts = time_us_64() + ADC_TASK_MEAS_PERIOD_US;

                float voltage_V = adc_task_get_voltage();
                float temp_C = adc_task_get_temp();

                printf("%f %f\n", voltage_V, temp_C);
            }
            break;

        default:
            break;
    }
}