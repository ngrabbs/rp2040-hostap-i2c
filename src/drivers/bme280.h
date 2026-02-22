/*
 * BME280 I2C Driver for Pico
 * 
 * The BME280 is a combined humidity and pressure sensor with I2C interface.
 * This driver focuses on temperature and humidity readings.
 */

#ifndef BME280_H
#define BME280_H

#include <stdint.h>
#include <stdbool.h>
#include "hardware/i2c.h"

/* BME280 Register Addresses */
#define BME280_REG_CHIP_ID      0xD0
#define BME280_REG_RESET        0xE0
#define BME280_REG_CTRL_HUM     0xF2
#define BME280_REG_STATUS       0xF3
#define BME280_REG_CTRL_MEAS    0xF4
#define BME280_REG_CONFIG       0xF5
#define BME280_REG_DATA_START   0xF7    /* Start of 8-byte data block */

/* Calibration data registers */
#define BME280_REG_CALIB_T1     0x88    /* Temperature calibration (6 bytes) */
#define BME280_REG_CALIB_P1     0x8E    /* Pressure calibration (18 bytes) */
#define BME280_REG_CALIB_H1     0xA1    /* Humidity calibration H1 (1 byte) */
#define BME280_REG_CALIB_H2     0xE1    /* Humidity calibration H2-H6 (7 bytes) */

/* Expected chip ID */
#define BME280_CHIP_ID_VALUE    0x60

/* Reset value */
#define BME280_RESET_VALUE      0xB6

/* Operating modes */
#define BME280_MODE_SLEEP       0x00
#define BME280_MODE_FORCED      0x01
#define BME280_MODE_NORMAL      0x03

/* Oversampling settings */
#define BME280_OVERSAMP_SKIP    0x00
#define BME280_OVERSAMP_1X      0x01
#define BME280_OVERSAMP_2X      0x02
#define BME280_OVERSAMP_4X      0x03
#define BME280_OVERSAMP_8X      0x04
#define BME280_OVERSAMP_16X     0x05

/* Standby time (for normal mode) */
#define BME280_STANDBY_0_5MS    0x00
#define BME280_STANDBY_62_5MS   0x01
#define BME280_STANDBY_125MS    0x02
#define BME280_STANDBY_250MS    0x03
#define BME280_STANDBY_500MS    0x04
#define BME280_STANDBY_1000MS   0x05
#define BME280_STANDBY_10MS     0x06
#define BME280_STANDBY_20MS     0x07

/* Filter coefficient */
#define BME280_FILTER_OFF       0x00
#define BME280_FILTER_2         0x01
#define BME280_FILTER_4         0x02
#define BME280_FILTER_8         0x03
#define BME280_FILTER_16        0x04

/* Calibration data structure */
typedef struct {
    /* Temperature calibration */
    uint16_t dig_T1;
    int16_t dig_T2;
    int16_t dig_T3;
    
    /* Pressure calibration (not used but read for completeness) */
    uint16_t dig_P1;
    int16_t dig_P2;
    int16_t dig_P3;
    int16_t dig_P4;
    int16_t dig_P5;
    int16_t dig_P6;
    int16_t dig_P7;
    int16_t dig_P8;
    int16_t dig_P9;
    
    /* Humidity calibration */
    uint8_t dig_H1;
    int16_t dig_H2;
    uint8_t dig_H3;
    int16_t dig_H4;
    int16_t dig_H5;
    int8_t dig_H6;
} bme280_calib_t;

/* BME280 context structure */
typedef struct {
    i2c_inst_t *i2c;
    uint8_t addr;
    bme280_calib_t calib;
    int32_t t_fine;         /* Fine temperature for compensation */
    bool initialized;
} bme280_t;

/* BME280 measurement results */
typedef struct {
    float temperature_c;    /* Temperature in Celsius */
    float humidity_pct;     /* Relative humidity in percent */
    float pressure_hpa;     /* Pressure in hPa (optional) */
    bool valid;             /* True if readings are valid */
} bme280_data_t;

/**
 * @brief Initialize the BME280 driver
 * 
 * @param dev Pointer to BME280 device structure
 * @param i2c I2C instance (i2c0 or i2c1)
 * @param addr I2C address of the device (0x76 or 0x77)
 * @return true if initialization successful
 */
bool bme280_init(bme280_t *dev, i2c_inst_t *i2c, uint8_t addr);

/**
 * @brief Read temperature and humidity from BME280
 * 
 * @param dev Pointer to BME280 device structure
 * @param data Pointer to data structure to fill
 * @return true if read successful
 */
bool bme280_read(bme280_t *dev, bme280_data_t *data);

/**
 * @brief Read temperature only
 * 
 * @param dev Pointer to BME280 device structure
 * @param temperature_c Pointer to store temperature in Celsius
 * @return true if read successful
 */
bool bme280_read_temperature(bme280_t *dev, float *temperature_c);

/**
 * @brief Read humidity only (also reads temperature internally)
 * 
 * @param dev Pointer to BME280 device structure
 * @param humidity_pct Pointer to store humidity in percent
 * @return true if read successful
 */
bool bme280_read_humidity(bme280_t *dev, float *humidity_pct);

/**
 * @brief Verify device identity
 * 
 * @param dev Pointer to BME280 device structure
 * @return true if chip ID matches expected value
 */
bool bme280_verify_id(bme280_t *dev);

/**
 * @brief Reset the BME280 device
 * 
 * @param dev Pointer to BME280 device structure
 * @return true if reset successful
 */
bool bme280_reset(bme280_t *dev);

#endif /* BME280_H */
