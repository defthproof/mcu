#ifndef IMAGE_DATA_H
#define IMAGE_DATA_H

#include <stdint.h>

#define DEMO_IMAGE_W 16
#define DEMO_IMAGE_H 16

#define BK 0x0000
#define WH 0xFFFF
#define BL 0x001F
#define YL 0xFFE0
#define RD 0xF800

static const uint16_t demo_image[DEMO_IMAGE_W * DEMO_IMAGE_H] =
{
    BK, BK, BK, BK, BK, BK, BK, BK, BK, BK, BK, BK, BK, BK, BK, BK,
    BK, BK, BK, BK, YL, YL, YL, YL, YL, YL, YL, YL, BK, BK, BK, BK,
    BK, BK, BK, YL, YL, YL, YL, YL, YL, YL, YL, YL, YL, BK, BK, BK,
    BK, BK, YL, YL, BL, BL, YL, YL, YL, YL, BL, BL, YL, YL, BK, BK,
    BK, YL, YL, YL, BL, BL, YL, YL, YL, YL, BL, BL, YL, YL, YL, BK,
    BK, YL, YL, YL, YL, YL, YL, YL, YL, YL, YL, YL, YL, YL, YL, BK,
    BK, YL, YL, RD, YL, YL, YL, YL, YL, YL, YL, YL, RD, YL, YL, BK,
    BK, YL, YL, YL, YL, YL, YL, YL, YL, YL, YL, YL, YL, YL, YL, BK,
    BK, YL, YL, YL, YL, BK, BK, YL, YL, BK, BK, YL, YL, YL, YL, BK,
    BK, YL, YL, YL, BK, BK, BK, BK, BK, BK, BK, BK, YL, YL, YL, BK,
    BK, YL, YL, YL, YL, BK, BK, BK, BK, BK, BK, YL, YL, YL, YL, BK,
    BK, BK, YL, YL, YL, YL, YL, BK, BK, YL, YL, YL, YL, YL, BK, BK,
    BK, BK, BK, YL, YL, YL, YL, YL, YL, YL, YL, YL, YL, BK, BK, BK,
    BK, BK, BK, BK, YL, YL, YL, YL, YL, YL, YL, YL, BK, BK, BK, BK,
    BK, BK, BK, BK, BK, BK, BK, BK, BK, BK, BK, BK, BK, BK, BK, BK,
    BK, BK, BK, BK, BK, BK, BK, BK, BK, BK, BK, BK, BK, BK, BK, BK
};

#endif