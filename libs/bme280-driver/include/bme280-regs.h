#ifndef BME280_REGS_H
#define BME280_REGS_H

#define BME280_REG_id         0xD0

#define BME280_REG_ctrl_hum   0xF2
#define BME280_REG_status     0xF3
#define BME280_REG_ctrl_meas  0xF4
#define BME280_REG_config     0xF5

#define BME280_REG_press_msb  0xF7
#define BME280_REG_press_lsb  0xF8
#define BME280_REG_press_xlsb 0xF9

#define BME280_REG_temp_msb   0xFA
#define BME280_REG_temp_lsb   0xFB
#define BME280_REG_temp_xlsb  0xFC

#define BME280_REG_hum_msb    0xFD
#define BME280_REG_hum_lsb    0xFE

#define BME280_REG_CALIB1_START 0x88
#define BME280_REG_CALIB1_LEN   26
#define BME280_REG_CALIB2_START 0xE1
#define BME280_REG_CALIB2_LEN   7

#define BME280_ID             0x60

#endif