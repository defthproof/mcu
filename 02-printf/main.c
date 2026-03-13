#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "stdio-task.h"

#define LED_PIN 25

int main()
{
    stdio_init_all();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, false);

    stdio_task_init();

    sleep_ms(2000);

    while (1)
    {
        stdio_task_handle();
    }
}