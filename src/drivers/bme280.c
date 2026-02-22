/*
 * BME280 I2C Driver Implementation
 * 
 * Based on Bosch BME280 datasheet compensation formulas.
 */

#include "bme280.h"
#include "pico/stdlib.h"
#include <string.h>

/* I2C timeout in microseconds */
#define I2C_TIMEOUT_US  100000

/* Helper function to write a single byte register */
static bool bme280_write_reg(bme280_t *dev, uint8_t reg, uint8_t value) {
    uint8_t buf[2] = {reg, value};
    int ret = i2c_write_timeout_us(dev->i2c, dev->addr, buf, 2, false, I2C_TIMEOUT_US);
    return (ret == 2);
}

/* Helper function to read bytes from a register */
static bool bme280_read_regs(bme280_t *dev, uint8_t reg, uint8_t *buf, size_t len) {
    /* Write register address */
    int ret = i2c_write_timeout_us(dev->i2c, dev->addr, &reg, 1, true, I2C_TIMEOUT_US);
    if (ret != 1) {
        return false;
    }
    
    /* Read data */
    ret = i2c_read_timeout_us(dev->i2c, dev->addr, buf, len, false, I2C_TIMEOUT_US);
    return (ret == (int)len);
}

/* Read calibration data from device */
static bool bme280_read_calibration(bme280_t *dev) {
    uint8_t calib_tp[26];   /* Temperature and pressure calibration */
    uint8_t calib_h[7];     /* Humidity calibration */
    uint8_t h1;
    
    /* Read temperature and pressure calibration (0x88-0xA1) */
    if (!bme280_read_regs(dev, BME280_REG_CALIB_T1, calib_tp, 26)) {
        return false;
    }
    
    /* Read humidity calibration H1 (0xA1) */
    if (!bme280_read_regs(dev, BME280_REG_CALIB_H1, &h1, 1)) {
        return false;
    }
    
    /* Read humidity calibration H2-H6 (0xE1-0xE7) */
    if (!bme280_read_regs(dev, BME280_REG_CALIB_H2, calib_h, 7)) {
        return false;
    }
    
    /* Parse temperature calibration */
    dev->calib.dig_T1 = (uint16_t)(calib_tp[1] << 8 | calib_tp[0]);
    dev->calib.dig_T2 = (int16_t)(calib_tp[3] << 8 | calib_tp[2]);
    dev->calib.dig_T3 = (int16_t)(calib_tp[5] << 8 | calib_tp[4]);
    
    /* Parse pressure calibration */
    dev->calib.dig_P1 = (uint16_t)(calib_tp[7] << 8 | calib_tp[6]);
    dev->calib.dig_P2 = (int16_t)(calib_tp[9] << 8 | calib_tp[8]);
    dev->calib.dig_P3 = (int16_t)(calib_tp[11] << 8 | calib_tp[10]);
    dev->calib.dig_P4 = (int16_t)(calib_tp[13] << 8 | calib_tp[12]);
    dev->calib.dig_P5 = (int16_t)(calib_tp[15] << 8 | calib_tp[14]);
    dev->calib.dig_P6 = (int16_t)(calib_tp[17] << 8 | calib_tp[16]);
    dev->calib.dig_P7 = (int16_t)(calib_tp[19] << 8 | calib_tp[18]);
    dev->calib.dig_P8 = (int16_t)(calib_tp[21] << 8 | calib_tp[20]);
    dev->calib.dig_P9 = (int16_t)(calib_tp[23] << 8 | calib_tp[22]);
    
    /* Parse humidity calibration */
    dev->calib.dig_H1 = h1;
    dev->calib.dig_H2 = (int16_t)(calib_h[1] << 8 | calib_h[0]);
    dev->calib.dig_H3 = calib_h[2];
    dev->calib.dig_H4 = (int16_t)((calib_h[3] << 4) | (calib_h[4] & 0x0F));
    dev->calib.dig_H5 = (int16_t)((calib_h[5] << 4) | ((calib_h[4] >> 4) & 0x0F));
    dev->calib.dig_H6 = (int8_t)calib_h[6];
    
    return true;
}

/* Compensate temperature using calibration data (from Bosch datasheet) */
static int32_t bme280_compensate_temp(bme280_t *dev, int32_t adc_T) {
    int32_t var1, var2, T;
    
    var1 = ((((adc_T >> 3) - ((int32_t)dev->calib.dig_T1 << 1))) *
            ((int32_t)dev->calib.dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)dev->calib.dig_T1)) *
              ((adc_T >> 4) - ((int32_t)dev->calib.dig_T1))) >> 12) *
            ((int32_t)dev->calib.dig_T3)) >> 14;
    
    dev->t_fine = var1 + var2;
    T = (dev->t_fine * 5 + 128) >> 8;
    
    return T;  /* Returns temperature in 0.01°C */
}

/* Compensate humidity using calibration data (from Bosch datasheet) */
static uint32_t bme280_compensate_humidity(bme280_t *dev, int32_t adc_H) {
    int32_t v_x1_u32r;
    
    v_x1_u32r = (dev->t_fine - ((int32_t)76800));
    v_x1_u32r = (((((adc_H << 14) - (((int32_t)dev->calib.dig_H4) << 20) -
                   (((int32_t)dev->calib.dig_H5) * v_x1_u32r)) + ((int32_t)16384)) >> 15) *
                (((((((v_x1_u32r * ((int32_t)dev->calib.dig_H6)) >> 10) *
                     (((v_x1_u32r * ((int32_t)dev->calib.dig_H3)) >> 11) +
                      ((int32_t)32768))) >> 10) + ((int32_t)2097152)) *
                  ((int32_t)dev->calib.dig_H2) + 8192) >> 14));
    v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) *
                               ((int32_t)dev->calib.dig_H1)) >> 4));
    v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
    v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
    
    return (uint32_t)(v_x1_u32r >> 12);  /* Returns humidity in Q22.10 format (22 int, 10 frac) */
}

bool bme280_init(bme280_t *dev, i2c_inst_t *i2c, uint8_t addr) {
    if (!dev || !i2c) {
        return false;
    }
    
    dev->i2c = i2c;
    dev->addr = addr;
    dev->initialized = false;
    dev->t_fine = 0;
    
    /* Verify chip ID */
    if (!bme280_verify_id(dev)) {
        return false;
    }
    
    /* Reset device */
    if (!bme280_reset(dev)) {
        return false;
    }
    
    /* Wait for reset to complete */
    sleep_ms(10);
    
    /* Read calibration data */
    if (!bme280_read_calibration(dev)) {
        return false;
    }
    
    /* Configure humidity oversampling (must be written before ctrl_meas) */
    if (!bme280_write_reg(dev, BME280_REG_CTRL_HUM, BME280_OVERSAMP_1X)) {
        return false;
    }
    
    /* Configure filter and standby time */
    uint8_t config = (BME280_STANDBY_1000MS << 5) | (BME280_FILTER_4 << 2);
    if (!bme280_write_reg(dev, BME280_REG_CONFIG, config)) {
        return false;
    }
    
    /* Configure measurement control: temp oversamp x1, press oversamp x1, normal mode */
    uint8_t ctrl_meas = (BME280_OVERSAMP_1X << 5) | (BME280_OVERSAMP_1X << 2) | BME280_MODE_NORMAL;
    if (!bme280_write_reg(dev, BME280_REG_CTRL_MEAS, ctrl_meas)) {
        return false;
    }
    
    dev->initialized = true;
    return true;
}

bool bme280_verify_id(bme280_t *dev) {
    uint8_t chip_id;
    
    if (!bme280_read_regs(dev, BME280_REG_CHIP_ID, &chip_id, 1)) {
        return false;
    }
    
    return (chip_id == BME280_CHIP_ID_VALUE);
}

bool bme280_reset(bme280_t *dev) {
    return bme280_write_reg(dev, BME280_REG_RESET, BME280_RESET_VALUE);
}

bool bme280_read(bme280_t *dev, bme280_data_t *data) {
    if (!dev || !data || !dev->initialized) {
        if (data) data->valid = false;
        return false;
    }
    
    memset(data, 0, sizeof(bme280_data_t));
    
    /* Read all data registers (pressure, temperature, humidity = 8 bytes) */
    uint8_t raw_data[8];
    if (!bme280_read_regs(dev, BME280_REG_DATA_START, raw_data, 8)) {
        data->valid = false;
        return false;
    }
    
    /* Parse raw ADC values */
    int32_t adc_P = (int32_t)((raw_data[0] << 12) | (raw_data[1] << 4) | (raw_data[2] >> 4));
    int32_t adc_T = (int32_t)((raw_data[3] << 12) | (raw_data[4] << 4) | (raw_data[5] >> 4));
    int32_t adc_H = (int32_t)((raw_data[6] << 8) | raw_data[7]);
    
    (void)adc_P;  /* Pressure not used in this application */
    
    /* Compensate temperature (must be done first for t_fine) */
    int32_t temp_raw = bme280_compensate_temp(dev, adc_T);
    data->temperature_c = (float)temp_raw / 100.0f;
    
    /* Compensate humidity */
    uint32_t hum_raw = bme280_compensate_humidity(dev, adc_H);
    data->humidity_pct = (float)hum_raw / 1024.0f;
    
    /* Clamp humidity to valid range */
    if (data->humidity_pct > 100.0f) {
        data->humidity_pct = 100.0f;
    }
    if (data->humidity_pct < 0.0f) {
        data->humidity_pct = 0.0f;
    }
    
    data->pressure_hpa = 0.0f;  /* Not computed */
    data->valid = true;
    
    return true;
}

bool bme280_read_temperature(bme280_t *dev, float *temperature_c) {
    bme280_data_t data;
    
    if (!bme280_read(dev, &data)) {
        return false;
    }
    
    *temperature_c = data.temperature_c;
    return true;
}

bool bme280_read_humidity(bme280_t *dev, float *humidity_pct) {
    bme280_data_t data;
    
    if (!bme280_read(dev, &data)) {
        return false;
    }
    
    *humidity_pct = data.humidity_pct;
    return true;
}
