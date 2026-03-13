#include "pico/stdlib.h"

#include <stdio.h>
#include <stdint.h>

#include "stdio-task/stdio-task.h"
#include "protocol-task/protocol-task.h"
#include "led-task/led-task.h"

#define DEVICE_NAME "my-pico-device"
#define DEVICE_VRSN "v0.0.1"

/* прототипы callback-функций */
void version_callback(const char* args);
void led_on_callback(const char* args);
void led_off_callback(const char* args);
void led_blink_callback(const char* args);
void led_blink_set_period_ms_callback(const char* args);
void mem_callback(const char* args);
void wmem_callback(const char* args);

api_t device_api[] =
{
    {"version", version_callback, "get device name and firmware version"},
    {"on", led_on_callback, "switch on led"},
    {"off", led_off_callback, "switch off led"},
    {"blink", led_blink_callback, "provide unblocking blinking"},
    {"led_blink_set_period_ms", led_blink_set_period_ms_callback, "set led blink period in ms"},
    {"mem", mem_callback, "read 32-bit value from memory by hex address"},
    {"wmem", wmem_callback, "write 32-bit value to memory by hex address"},
    {NULL, NULL, NULL},
};

void version_callback(const char* args)
{
    (void)args;
    printf("device name: '%s', firmware version: %s\n", DEVICE_NAME, DEVICE_VRSN);
}

void led_on_callback(const char* args)
{
    (void)args;
    led_task_state_set(LED_STATE_ON);
}

void led_off_callback(const char* args)
{
    (void)args;
    led_task_state_set(LED_STATE_OFF);
}

void led_blink_callback(const char* args)
{
    (void)args;
    led_task_state_set(LED_STATE_BLINK);
}

void led_blink_set_period_ms_callback(const char* args)
{
    unsigned int period_ms = 0;

    if (sscanf(args, "%u", &period_ms) != 1)
    {
        printf("error: invalid period\n");
        return;
    }

    if (period_ms == 0)
    {
        printf("error: period must be greater than 0\n");
        return;
    }

    led_task_set_blink_period_ms((uint32_t)period_ms);
    printf("blink period set to %u ms\n", period_ms);
}

void mem_callback(const char* args)
{
    unsigned int addr = 0;

    if (sscanf(args, "%x", &addr) != 1)
    {
        printf("error: invalid address\n");
        return;
    }

    uint32_t value = *(volatile uint32_t*)addr;

    printf("mem[0x%08lX] = 0x%08lX\n",
           (unsigned long)addr,
           (unsigned long)value);
}

void wmem_callback(const char* args)
{
    unsigned int addr = 0;
    unsigned int value = 0;

    if (sscanf(args, "%x %x", &addr, &value) != 2)
    {
        printf("error: invalid arguments\n");
        return;
    }

    *(volatile uint32_t*)addr = (uint32_t)value;

    printf("write 0x%08lX -> 0x%08lX\n",
           (unsigned long)value,
           (unsigned long)addr);
}

int main()
{
    stdio_init_all();
    stdio_task_init();
    protocol_task_init(device_api);
    led_task_init();

    while (true)
    {
        char* command = stdio_task_handle();
        protocol_task_handle(command);
        led_task_handle();
    }
}