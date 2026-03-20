#include "pico/stdlib.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "hardware/gpio.h"
#include "hardware/spi.h"

#include "stdio-task/stdio-task.h"
#include "protocol-task.h"
#include "led-task/led-task.h"

#include "ili9341-driver.h"
#include "ili9341-display.h"
#include "ili9341-font.h"
#include "font-jetbrains.h"

#define DEVICE_NAME "my-pico-device"
#define DEVICE_VRSN "v0.0.4"

#define ILI9341_PIN_MISO   4
#define ILI9341_PIN_CS     10
#define ILI9341_PIN_SCK    6
#define ILI9341_PIN_MOSI   7
#define ILI9341_PIN_DC     8
#define ILI9341_PIN_RESET  9

#define DISPLAY_WIDTH   320
#define DISPLAY_HEIGHT  240

static ili9341_display_t ili9341_display = {0};

void version_callback(const char* args);
void led_on_callback(const char* args);
void led_off_callback(const char* args);
void disp_screen_callback(const char* args);
void disp_px_callback(const char* args);
void disp_line_callback(const char* args);
void disp_rect_callback(const char* args);
void disp_frect_callback(const char* args);
void disp_text_callback(const char* args);

void rp2040_spi_write(const uint8_t* data, uint32_t size);
void rp2040_spi_read(uint8_t* buffer, uint32_t length);
void rp2040_gpio_cs_write(bool level);
void rp2040_gpio_dc_write(bool level);
void rp2040_gpio_reset_write(bool level);
void rp2040_delay_ms(uint32_t ms);

api_t device_api[] =
{
    {"version", version_callback, "get device name and firmware version"},
    {"on", led_on_callback, "switch on led"},
    {"off", led_off_callback, "switch off led"},
    {"disp_screen", disp_screen_callback, "fill whole screen: disp_screen RRGGBB"},
    {"disp_px", disp_px_callback, "draw pixel: disp_px x y RRGGBB"},
    {"disp_line", disp_line_callback, "draw line: disp_line x0 y0 x1 y1 RRGGBB"},
    {"disp_rect", disp_rect_callback, "draw rectangle: disp_rect x y w h RRGGBB"},
    {"disp_frect", disp_frect_callback, "draw filled rectangle: disp_frect x y w h RRGGBB"},
    {"disp_text", disp_text_callback, "draw text: disp_text x y FG BG text"},
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

void disp_screen_callback(const char* args)
{
    uint32_t c = 0;
    int result = sscanf(args, "%x", &c);

    uint16_t color = COLOR_BLACK;

    if (result == 1)
    {
        color = RGB888_2_RGB565(c);
    }

    ili9341_fill_screen(&ili9341_display, color);
}

void disp_px_callback(const char* args)
{
    unsigned int x = 0;
    unsigned int y = 0;
    uint32_t c = 0;

    int result = sscanf(args, "%u %u %x", &x, &y, &c);

    if (result != 3)
    {
        printf("error: use disp_px x y RRGGBB\n");
        return;
    }

    if (x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT)
    {
        printf("error: pixel is out of screen\n");
        return;
    }

    uint16_t color = RGB888_2_RGB565(c);
    ili9341_draw_pixel(&ili9341_display, (uint16_t)x, (uint16_t)y, color);
}

void disp_line_callback(const char* args)
{
    unsigned int x0 = 0;
    unsigned int y0 = 0;
    unsigned int x1 = 0;
    unsigned int y1 = 0;
    uint32_t c = 0;

    int result = sscanf(args, "%u %u %u %u %x", &x0, &y0, &x1, &y1, &c);

    if (result != 5)
    {
        printf("error: use disp_line x0 y0 x1 y1 RRGGBB\n");
        return;
    }

    if (x0 >= DISPLAY_WIDTH || y0 >= DISPLAY_HEIGHT ||
        x1 >= DISPLAY_WIDTH || y1 >= DISPLAY_HEIGHT)
    {
        printf("error: line point is out of screen\n");
        return;
    }

    uint16_t color = RGB888_2_RGB565(c);

    ili9341_draw_line(
        &ili9341_display,
        (uint16_t)x0,
        (uint16_t)y0,
        (uint16_t)x1,
        (uint16_t)y1,
        color
    );
}

void disp_rect_callback(const char* args)
{
    unsigned int x = 0;
    unsigned int y = 0;
    unsigned int w = 0;
    unsigned int h = 0;
    uint32_t c = 0;

    int result = sscanf(args, "%u %u %u %u %x", &x, &y, &w, &h, &c);

    if (result != 5)
    {
        printf("error: use disp_rect x y w h RRGGBB\n");
        return;
    }

    if (x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT)
    {
        printf("error: rectangle start is out of screen\n");
        return;
    }

    if (w == 0 || h == 0)
    {
        printf("error: width and height must be > 0\n");
        return;
    }

    uint16_t color = RGB888_2_RGB565(c);

    ili9341_draw_rect(
        &ili9341_display,
        (uint16_t)x,
        (uint16_t)y,
        (uint16_t)w,
        (uint16_t)h,
        color
    );
}

void disp_frect_callback(const char* args)
{
    unsigned int x = 0;
    unsigned int y = 0;
    unsigned int w = 0;
    unsigned int h = 0;
    uint32_t c = 0;

    int result = sscanf(args, "%u %u %u %u %x", &x, &y, &w, &h, &c);

    if (result != 5)
    {
        printf("error: use disp_frect x y w h RRGGBB\n");
        return;
    }

    if (x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT)
    {
        printf("error: rectangle start is out of screen\n");
        return;
    }

    if (w == 0 || h == 0)
    {
        printf("error: width and height must be > 0\n");
        return;
    }

    uint16_t color = RGB888_2_RGB565(c);

    ili9341_draw_filled_rect(
        &ili9341_display,
        (uint16_t)x,
        (uint16_t)y,
        (uint16_t)w,
        (uint16_t)h,
        color
    );
}

void disp_text_callback(const char* args)
{
    unsigned int x = 0;
    unsigned int y = 0;
    uint32_t fg = 0;
    uint32_t bg = 0;
    int text_offset = 0;

    int result = sscanf(args, "%u %u %x %x %n", &x, &y, &fg, &bg, &text_offset);

    if (result != 4)
    {
        printf("error: use disp_text x y FG BG text\n");
        return;
    }

    if (x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT)
    {
        printf("error: text position is out of screen\n");
        return;
    }

    const char* text = args + text_offset;

    while (*text == ' ')
    {
        text++;
    }

    if (*text == '\0')
    {
        printf("error: text is empty\n");
        return;
    }

    uint16_t fg_color = RGB888_2_RGB565(fg);
    uint16_t bg_color = RGB888_2_RGB565(bg);

    ili9341_draw_text(
        &ili9341_display,
        (uint16_t)x,
        (uint16_t)y,
        text,
        &jetbrains_font,
        fg_color,
        bg_color
    );
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

    while (true)
    {
        char* command = stdio_task_handle();
        protocol_task_handle(command);
        led_task_handle();
    }
}