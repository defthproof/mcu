#include "pico/stdlib.h"

#include <stdio.h>
#include <stdint.h>

#include "hardware/gpio.h"
#include "hardware/i2c.h"

#include "stdio-task/stdio-task.h"
#include "protocol-task.h"
#include "bme280-driver.h"

#define DEVICE_NAME "my-pico-device"
#define DEVICE_VRSN "v0.0.14"

#define BME280_I2C i2c1
#define BME280_I2C_ADDR 0x76
#define BME280_SDA_PIN 14
#define BME280_SCL_PIN 15
void temp_callback(const char* args);
void pres_callback(const char* args);
void hum_callback(const char* args);
void version_callback(const char* args);
void read_regs_callback(const char* args);
void write_reg_callback(const char* args);
void temp_raw_callback(const char* args);
void pres_raw_callback(const char* args);
void hum_raw_callback(const char* args);

void rp2040_i2c_read(uint8_t* buffer, uint16_t length);
void rp2040_i2c_write(uint8_t* data, uint16_t size);

api_t device_api[] =
{
    {"version", version_callback, "get device name and firmware version"},
    {"read_regs", read_regs_callback, "read BME280 registers: read_regs addr N"},
    {"write_reg", write_reg_callback, "write BME280 register: write_reg addr value"},
    {"temp_raw", temp_raw_callback, "read raw temperature"},
    {"pres_raw", pres_raw_callback, "read raw pressure"},
    {"hum_raw", hum_raw_callback, "read raw humidity"},
    {"temp", temp_callback, "read temperature in C"},
    {"pres", pres_callback, "read pressure in Pa"},
    {"hum", hum_callback, "read humidity in %RH"},
    {NULL, NULL, NULL},
};

void version_callback(const char* args)
{
    (void)args;
    printf("device name: '%s', firmware version: %s\n", DEVICE_NAME, DEVICE_VRSN);
}

void read_regs_callback(const char* args)
{
    unsigned int addr = 0;
    unsigned int N = 0;

    if (sscanf(args, "%x %x", &addr, &N) != 2)
    {
        printf("error: invalid arguments\n");
        return;
    }

    if (addr > 0xFF)
    {
        printf("error: addr must be <= 0xFF\n");
        return;
    }

    if (N > 0xFF)
    {
        printf("error: N must be <= 0xFF\n");
        return;
    }

    if (N == 0)
    {
        printf("error: N must be > 0\n");
        return;
    }

    if ((addr + N) > 0x100)
    {
        printf("error: addr + N must be <= 0x100\n");
        return;
    }

    uint8_t buffer[256] = {0};

    bme280_read_regs((uint8_t)addr, buffer, (uint8_t)N);

    for (unsigned int i = 0; i < N; i++)
    {
        printf("bme280 register [0x%02X] = 0x%02X\n",
               (unsigned int)((uint8_t)addr + i),
               buffer[i]);
    }
}
void temp_callback(const char* args)
{
    (void)args;
    float value = bme280_read_temp();
    printf("%f\n", value);
}

void pres_callback(const char* args)
{
    (void)args;
    float value = bme280_read_pres();
    printf("%f\n", value);
}

void hum_callback(const char* args)
{
    (void)args;
    float value = bme280_read_hum();
    printf("%f\n", value);
}
void write_reg_callback(const char* args)
{
    unsigned int addr = 0;
    unsigned int value = 0;

    if (sscanf(args, "%x %x", &addr, &value) != 2)
    {
        printf("error: invalid arguments\n");
        return;
    }

    if (addr > 0xFF)
    {
        printf("error: addr must be <= 0xFF\n");
        return;
    }

    if (value > 0xFF)
    {
        printf("error: value must be <= 0xFF\n");
        return;
    }

    bme280_write_reg((uint8_t)addr, (uint8_t)value);

    printf("write 0x%02X to bme280 register [0x%02X]\n",
           (unsigned int)((uint8_t)value),
           (unsigned int)((uint8_t)addr));
}

void temp_raw_callback(const char* args)
{
    (void)args;
    uint16_t value = bme280_read_temp_raw();
    printf("%u\n", value);
}

void pres_raw_callback(const char* args)
{
    (void)args;
    uint16_t value = bme280_read_pres_raw();
    printf("%u\n", value);
}

void hum_raw_callback(const char* args)
{
    (void)args;
    uint16_t value = bme280_read_hum_raw();
    printf("%u\n", value);
}

void rp2040_i2c_read(uint8_t* buffer, uint16_t length)
{
    i2c_read_timeout_us(BME280_I2C, BME280_I2C_ADDR, buffer, length, false, 100000);
}

void rp2040_i2c_write(uint8_t* data, uint16_t size)
{
    i2c_write_timeout_us(BME280_I2C, BME280_I2C_ADDR, data, size, false, 100000);
}

int main()
{
    stdio_init_all();
    stdio_task_init();
    protocol_task_init(device_api);

    i2c_init(BME280_I2C, 100000);

    gpio_set_function(BME280_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(BME280_SCL_PIN, GPIO_FUNC_I2C);

    bme280_init(rp2040_i2c_read, rp2040_i2c_write);

    while (true)
    {
        char* command = stdio_task_handle();
        protocol_task_handle(command);
    }
}