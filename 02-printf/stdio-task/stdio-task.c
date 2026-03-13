#include "stdio-task.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#define LED_PIN 25
#define DEVICE_NAME "my-pico-device"
#define DEVICE_VRSN "v0.0.1"

void stdio_task_init(void)
{
}

void stdio_task_handle(void)
{
    int symbol = getchar_timeout_us(0);
    if (symbol == PICO_ERROR_TIMEOUT)
    {
        return;
    }

    switch (symbol)
    {
    case 'e':
        gpio_put(LED_PIN, true);
        printf("led enable done\n");
        break;

    case 'd':
        gpio_put(LED_PIN, false);
        printf("led disable done\n");
        break;

    case 'v':
        printf("device name: '%s', firmware version: %s\n", DEVICE_NAME, DEVICE_VRSN);
        break;

    default:
        break;
    }
}