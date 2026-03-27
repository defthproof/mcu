#include "pico/stdlib.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"

#include "stdio-task/stdio-task.h"
#include "protocol-task.h"
#include "led-task/led-task.h"

#include "ili9341-driver.h"
#include "ili9341-display.h"
#include "ili9341-font.h"
#include "font-jetbrains.h"

#include "bme280-driver.h"

#define DEVICE_NAME "my-pico-device"
#define DEVICE_VRSN "v1.0.0"

#define ILI9341_PIN_MISO   4
#define ILI9341_PIN_CS     10
#define ILI9341_PIN_SCK    6
#define ILI9341_PIN_MOSI   7
#define ILI9341_PIN_DC     8
#define ILI9341_PIN_RESET  9

#define DISPLAY_WIDTH   320
#define DISPLAY_HEIGHT  240

#define BME280_I2C       i2c1
#define BME280_I2C_ADDR  0x76
#define BME280_SDA_PIN   14
#define BME280_SCL_PIN   15

static ili9341_display_t ili9341_display = {0};

static float g_temp = 0.0f;
static float g_pres = 0.0f;
static float g_hum  = 0.0f;

static uint32_t g_measure_period_ms = 1000;
static uint32_t g_last_measure_ms = 0;
static bool g_sensor_ok = false;

void version_callback(const char* args);
void help_callback(const char* args);
void status_callback(const char* args);
void measure_callback(const char* args);
void period_callback(const char* args);
void led_on_callback(const char* args);
void led_off_callback(const char* args);

void rp2040_spi_write(const uint8_t* data, uint32_t size);
void rp2040_spi_read(uint8_t* buffer, uint32_t length);
void rp2040_gpio_cs_write(bool level);
void rp2040_gpio_dc_write(bool level);
void rp2040_gpio_reset_write(bool level);
void rp2040_delay_ms(uint32_t ms);

void rp2040_i2c_read(uint8_t* buffer, uint16_t length);
void rp2040_i2c_write(uint8_t* data, uint16_t size);

void display_draw_static(void);
void display_draw_measurements(void);
void measure_once(void);

api_t device_api[] =
{
    {"help",    help_callback,    "show available commands"},
    {"version", version_callback, "show device name and firmware version"},
    {"status",  status_callback,  "show current measurements and period"},
    {"measure", measure_callback, "perform measurement immediately"},
    {"period",  period_callback,  "set measurement period in ms: period 1000"},
    {"on",      led_on_callback,  "switch on led"},
    {"off",     led_off_callback, "switch off led"},
    {NULL, NULL, NULL},
};

void version_callback(const char* args)
{
    (void)args;
    printf("device name: '%s', firmware version: %s\n", DEVICE_NAME, DEVICE_VRSN);
}

void help_callback(const char* args)
{
    (void)args;

    printf("available commands:\n");

    for (int i = 0; device_api[i].command_name != NULL; i++)
    {
        printf("  %s - %s\n",
               device_api[i].command_name,
               device_api[i].command_help);
    }
}

void status_callback(const char* args)
{
    (void)args;

    printf("sensor: %s\n", g_sensor_ok ? "ok" : "not initialized");
    printf("period_ms: %u\n", g_measure_period_ms);
    printf("temperature_c: %.2f\n", g_temp);
    printf("pressure_pa: %.2f\n", g_pres);
    printf("humidity_rh: %.2f\n", g_hum);
}

void measure_callback(const char* args)
{
    (void)args;

    measure_once();

    printf("temperature_c: %.2f\n", g_temp);
    printf("pressure_pa: %.2f\n", g_pres);
    printf("humidity_rh: %.2f\n", g_hum);
}

void period_callback(const char* args)
{
    unsigned int value = 0;

    if (sscanf(args, "%u", &value) != 1)
    {
        printf("error: use period <ms>\n");
        return;
    }

    if (value < 100)
    {
        printf("error: period must be >= 100 ms\n");
        return;
    }

    g_measure_period_ms = (uint32_t)value;
    printf("measurement period set to %u ms\n", g_measure_period_ms);
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

void rp2040_spi_write(const uint8_t* data, uint32_t size)
{
    spi_write_blocking(spi0, data, size);
}

void rp2040_spi_read(uint8_t* buffer, uint32_t length)
{
    spi_read_blocking(spi0, 0, buffer, length);
}

void rp2040_gpio_cs_write(bool level)
{
    gpio_put(ILI9341_PIN_CS, level);
}

void rp2040_gpio_dc_write(bool level)
{
    gpio_put(ILI9341_PIN_DC, level);
}

void rp2040_gpio_reset_write(bool level)
{
    gpio_put(ILI9341_PIN_RESET, level);
}

void rp2040_delay_ms(uint32_t ms)
{
    sleep_ms(ms);
}

void rp2040_i2c_read(uint8_t* buffer, uint16_t length)
{
    i2c_read_timeout_us(BME280_I2C, BME280_I2C_ADDR, buffer, length, false, 100000);
}

void rp2040_i2c_write(uint8_t* data, uint16_t size)
{
    i2c_write_timeout_us(BME280_I2C, BME280_I2C_ADDR, data, size, false, 100000);
}

void display_draw_static(void)
{
    ili9341_fill_screen(&ili9341_display, COLOR_YELLOW);

    ili9341_draw_text(
        &ili9341_display, 10, 10,
        "BME280 monitor",
        &jetbrains_font,
        COLOR_BLACK,
        COLOR_YELLOW
    );

    ili9341_draw_text(
        &ili9341_display, 10, 40,
        "Temp:",
        &jetbrains_font,
        COLOR_BLACK,
        COLOR_YELLOW
    );

    ili9341_draw_text(
        &ili9341_display, 10, 80,
        "Pres:",
        &jetbrains_font,
        COLOR_BLACK,
        COLOR_YELLOW
    );

    ili9341_draw_text(
        &ili9341_display, 10, 120,
        "Hum :",
        &jetbrains_font,
        COLOR_BLACK,
        COLOR_YELLOW
    );

    ili9341_draw_text(
        &ili9341_display, 10, 160,
        "Period:",
        &jetbrains_font,
        COLOR_BLACK,
        COLOR_YELLOW
    );
}

void display_draw_measurements(void)
{
    char buffer[64];

    ili9341_draw_filled_rect(&ili9341_display, 90, 40, 220, 24, COLOR_YELLOW);
    ili9341_draw_filled_rect(&ili9341_display, 90, 80, 220, 24, COLOR_YELLOW);
    ili9341_draw_filled_rect(&ili9341_display, 90, 120, 220, 24, COLOR_YELLOW);
    ili9341_draw_filled_rect(&ili9341_display, 90, 160, 220, 24, COLOR_YELLOW);

    snprintf(buffer, sizeof(buffer), "%.2f C", g_temp);
    ili9341_draw_text(&ili9341_display, 90, 40, buffer, &jetbrains_font, COLOR_BLACK, COLOR_YELLOW);

    snprintf(buffer, sizeof(buffer), "%.2f Pa", g_pres);
    ili9341_draw_text(&ili9341_display, 90, 80, buffer, &jetbrains_font, COLOR_BLACK, COLOR_YELLOW);

    snprintf(buffer, sizeof(buffer), "%.2f %%", g_hum);
    ili9341_draw_text(&ili9341_display, 90, 120, buffer, &jetbrains_font, COLOR_BLACK, COLOR_YELLOW);

    snprintf(buffer, sizeof(buffer), "%u ms", g_measure_period_ms);
    ili9341_draw_text(&ili9341_display, 90, 160, buffer, &jetbrains_font, COLOR_BLACK, COLOR_YELLOW);
}

void measure_once(void)
{
    g_temp = bme280_read_temp();
    g_pres = bme280_read_pres();
    g_hum  = bme280_read_hum();

    display_draw_measurements();
}

int main()
{
    stdio_init_all();
    stdio_task_init();
    protocol_task_init(device_api);
    led_task_init();

    spi_init(spi0, 62500000);

    gpio_set_function(ILI9341_PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(ILI9341_PIN_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(ILI9341_PIN_SCK, GPIO_FUNC_SPI);

    gpio_init(ILI9341_PIN_CS);
    gpio_set_dir(ILI9341_PIN_CS, GPIO_OUT);

    gpio_init(ILI9341_PIN_DC);
    gpio_set_dir(ILI9341_PIN_DC, GPIO_OUT);

    gpio_init(ILI9341_PIN_RESET);
    gpio_set_dir(ILI9341_PIN_RESET, GPIO_OUT);

    gpio_put(ILI9341_PIN_CS, 1);
    gpio_put(ILI9341_PIN_DC, 1);
    gpio_put(ILI9341_PIN_RESET, 1);

    ili9341_hal_t ili9341_hal = {0};
    ili9341_hal.spi_write = rp2040_spi_write;
    ili9341_hal.spi_read = rp2040_spi_read;
    ili9341_hal.gpio_cs_write = rp2040_gpio_cs_write;
    ili9341_hal.gpio_dc_write = rp2040_gpio_dc_write;
    ili9341_hal.gpio_reset_write = rp2040_gpio_reset_write;
    ili9341_hal.delay_ms = rp2040_delay_ms;

    ili9341_init(&ili9341_display, &ili9341_hal);
    ili9341_set_rotation(&ili9341_display, ILI9341_ROTATION_90);

    display_draw_static();

    i2c_init(BME280_I2C, 100000);
    gpio_set_function(BME280_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(BME280_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(BME280_SDA_PIN);
    gpio_pull_up(BME280_SCL_PIN);

    sleep_ms(100);

    bme280_init(rp2040_i2c_read, rp2040_i2c_write);
    g_sensor_ok = true;

    measure_once();
    g_last_measure_ms = to_ms_since_boot(get_absolute_time());

    printf("device started\n");
    printf("type 'help' to see commands\n");

    while (true)
    {
        char* command = stdio_task_handle();
        protocol_task_handle(command);
        led_task_handle();

        uint32_t now_ms = to_ms_since_boot(get_absolute_time());

        if ((now_ms - g_last_measure_ms) >= g_measure_period_ms)
        {
            measure_once();
            g_last_measure_ms = now_ms;
        }
    }
}