#include "bme280-driver.h"
#include "bme280-regs.h"

#include <stdio.h>

typedef struct
{
    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;

    uint16_t dig_P1;
    int16_t  dig_P2;
    int16_t  dig_P3;
    int16_t  dig_P4;
    int16_t  dig_P5;
    int16_t  dig_P6;
    int16_t  dig_P7;
    int16_t  dig_P8;
    int16_t  dig_P9;

    uint8_t  dig_H1;
    int16_t  dig_H2;
    uint8_t  dig_H3;
    int16_t  dig_H4;
    int16_t  dig_H5;
    int8_t   dig_H6;

    int32_t  t_fine;
} bme280_calib_t;

typedef struct
{
    bme280_i2c_read i2c_read;
    bme280_i2c_write i2c_write;
    bme280_calib_t calib;
} bme280_ctx_t;

static bme280_ctx_t bme280_ctx = {0};

static uint16_t u16_le(const uint8_t* p)
{
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static int16_t s16_le(const uint8_t* p)
{
    return (int16_t)u16_le(p);
}

static void bme280_read_calibration(void)
{
    uint8_t calib1[BME280_REG_CALIB1_LEN] = {0};
    uint8_t calib2[BME280_REG_CALIB2_LEN] = {0};

    bme280_read_regs(BME280_REG_CALIB1_START, calib1, sizeof(calib1));
    bme280_read_regs(BME280_REG_CALIB2_START, calib2, sizeof(calib2));

    bme280_ctx.calib.dig_T1 = u16_le(&calib1[0]);
    bme280_ctx.calib.dig_T2 = s16_le(&calib1[2]);
    bme280_ctx.calib.dig_T3 = s16_le(&calib1[4]);

    bme280_ctx.calib.dig_P1 = u16_le(&calib1[6]);
    bme280_ctx.calib.dig_P2 = s16_le(&calib1[8]);
    bme280_ctx.calib.dig_P3 = s16_le(&calib1[10]);
    bme280_ctx.calib.dig_P4 = s16_le(&calib1[12]);
    bme280_ctx.calib.dig_P5 = s16_le(&calib1[14]);
    bme280_ctx.calib.dig_P6 = s16_le(&calib1[16]);
    bme280_ctx.calib.dig_P7 = s16_le(&calib1[18]);
    bme280_ctx.calib.dig_P8 = s16_le(&calib1[20]);
    bme280_ctx.calib.dig_P9 = s16_le(&calib1[22]);

    bme280_ctx.calib.dig_H1 = calib1[25];

    bme280_ctx.calib.dig_H2 = s16_le(&calib2[0]);
    bme280_ctx.calib.dig_H3 = calib2[2];
    bme280_ctx.calib.dig_H4 = (int16_t)((((int16_t)calib2[3]) << 4) | (calib2[4] & 0x0F));
    bme280_ctx.calib.dig_H5 = (int16_t)((((int16_t)calib2[5]) << 4) | ((calib2[4] >> 4) & 0x0F));
    bme280_ctx.calib.dig_H6 = (int8_t)calib2[6];
}

void bme280_read_regs(uint8_t start_reg_address, uint8_t* buffer, uint8_t length)
{
    if (buffer == 0 || length == 0)
    {
        return;
    }

    uint8_t data[1] = {start_reg_address};

    bme280_ctx.i2c_write(data, sizeof(data));
    bme280_ctx.i2c_read(buffer, length);
}

void bme280_write_reg(uint8_t reg_address, uint8_t value)
{
    uint8_t data[2] = {reg_address, value};
    bme280_ctx.i2c_write(data, sizeof(data));
}

void bme280_init(bme280_i2c_read i2c_read, bme280_i2c_write i2c_write)
{
    bme280_ctx.i2c_read = i2c_read;
    bme280_ctx.i2c_write = i2c_write;

    uint8_t id_reg_buf[1] = {0};
    bme280_read_regs(BME280_REG_id, id_reg_buf, sizeof(id_reg_buf));

    if (id_reg_buf[0] != BME280_ID)
    {
        printf("error: invalid BME280 id = 0x%02X\n", id_reg_buf[0]);
        return;
    }

    bme280_read_calibration();

    uint8_t ctrl_hum_reg_value = 0;
    ctrl_hum_reg_value |= (0b001 << 0);   // osrs_h = 1
    bme280_write_reg(BME280_REG_ctrl_hum, ctrl_hum_reg_value);

    uint8_t config_reg_value = 0;
    config_reg_value |= (0b0   << 0);     // spi3w_en = 0
    config_reg_value |= (0b000 << 2);     // filter = off
    config_reg_value |= (0b001 << 5);     // t_sb = 62.5 ms
    bme280_write_reg(BME280_REG_config, config_reg_value);

    uint8_t ctrl_meas_reg_value = 0;
    ctrl_meas_reg_value |= (0b11  << 0);  // mode = normal
    ctrl_meas_reg_value |= (0b001 << 2);  // osrs_p = 1
    ctrl_meas_reg_value |= (0b001 << 5);  // osrs_t = 1
    bme280_write_reg(BME280_REG_ctrl_meas, ctrl_meas_reg_value);
}

uint32_t bme280_read_temp_raw(void)
{
    uint8_t read[3] = {0};
    bme280_read_regs(BME280_REG_temp_msb, read, sizeof(read));

    uint32_t value = ((uint32_t)read[0] << 12) |
                     ((uint32_t)read[1] << 4)  |
                     ((uint32_t)read[2] >> 4);
    return value;
}

uint32_t bme280_read_pres_raw(void)
{
    uint8_t read[3] = {0};
    bme280_read_regs(BME280_REG_press_msb, read, sizeof(read));

    uint32_t value = ((uint32_t)read[0] << 12) |
                     ((uint32_t)read[1] << 4)  |
                     ((uint32_t)read[2] >> 4);
    return value;
}

uint16_t bme280_read_hum_raw(void)
{
    uint8_t read[2] = {0};
    bme280_read_regs(BME280_REG_hum_msb, read, sizeof(read));

    uint16_t value = ((uint16_t)read[0] << 8) | ((uint16_t)read[1]);
    return value;
}

float bme280_read_temp(void)
{
    float adc_T = (float)bme280_read_temp_raw();

    float var1 = (adc_T / 16384.0f - (float)bme280_ctx.calib.dig_T1 / 1024.0f) *
                 (float)bme280_ctx.calib.dig_T2;

    float var2 = ((adc_T / 131072.0f - (float)bme280_ctx.calib.dig_T1 / 8192.0f) *
                  (adc_T / 131072.0f - (float)bme280_ctx.calib.dig_T1 / 8192.0f)) *
                 (float)bme280_ctx.calib.dig_T3;

    float temp = var1 + var2;
    bme280_ctx.calib.t_fine = (int32_t)temp;

    return temp / 5120.0f;
}

float bme280_read_pres(void)
{
    float adc_P = (float)bme280_read_pres_raw();

    /* обязательно сначала обновить t_fine через температуру */
    bme280_read_temp();

    float var1 = ((float)bme280_ctx.calib.t_fine / 2.0f) - 64000.0f;
    float var2 = var1 * var1 * ((float)bme280_ctx.calib.dig_P6) / 32768.0f;
    var2 = var2 + var1 * ((float)bme280_ctx.calib.dig_P5) * 2.0f;
    var2 = (var2 / 4.0f) + (((float)bme280_ctx.calib.dig_P4) * 65536.0f);

    var1 = (((float)bme280_ctx.calib.dig_P3) * var1 * var1 / 524288.0f +
            ((float)bme280_ctx.calib.dig_P2) * var1) / 524288.0f;
    var1 = (1.0f + var1 / 32768.0f) * ((float)bme280_ctx.calib.dig_P1);

    if (var1 == 0.0f)
    {
        return 0.0f;
    }

    float p = 1048576.0f - adc_P;
    p = (p - (var2 / 4096.0f)) * 6250.0f / var1;

    var1 = ((float)bme280_ctx.calib.dig_P9) * p * p / 2147483648.0f;
    var2 = p * ((float)bme280_ctx.calib.dig_P8) / 32768.0f;

    p = p + (var1 + var2 + ((float)bme280_ctx.calib.dig_P7)) / 16.0f;

    return p; /* Па */
}

float bme280_read_hum(void)
{
    float adc_H = (float)bme280_read_hum_raw();

    /* обязательно сначала обновить t_fine через температуру */
    bme280_read_temp();

    float var_H = ((float)bme280_ctx.calib.t_fine) - 76800.0f;

    var_H = (adc_H - (((float)bme280_ctx.calib.dig_H4) * 64.0f +
             ((float)bme280_ctx.calib.dig_H5) / 16384.0f * var_H)) *
            (((float)bme280_ctx.calib.dig_H2) / 65536.0f *
             (1.0f + ((float)bme280_ctx.calib.dig_H6) / 67108864.0f * var_H *
             (1.0f + ((float)bme280_ctx.calib.dig_H3) / 67108864.0f * var_H)));

    var_H = var_H * (1.0f - ((float)bme280_ctx.calib.dig_H1) * var_H / 524288.0f);

    if (var_H > 100.0f)
    {
        var_H = 100.0f;
    }
    else if (var_H < 0.0f)
    {
        var_H = 0.0f;
    }

    return var_H; /* %RH */
}