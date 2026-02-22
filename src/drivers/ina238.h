/*
 * INA238 I2C Driver for Pico
 * 
 * The INA238 is a digital power monitor with I2C interface.
 * It measures bus voltage and current using a shunt resistor.
 */

#ifndef INA238_H
#define INA238_H

#include <stdint.h>
#include <stdbool.h>
#include "hardware/i2c.h"

/* INA238 Register Addresses */
#define INA238_REG_CONFIG       0x00    /* Configuration register */
#define INA238_REG_ADC_CONFIG   0x01    /* ADC Configuration register */
#define INA238_REG_SHUNT_CAL    0x02    /* Shunt Calibration register */
#define INA238_REG_VSHUNT       0x04    /* Shunt Voltage measurement */
#define INA238_REG_VBUS         0x05    /* Bus Voltage measurement */
#define INA238_REG_DIETEMP      0x06    /* Die Temperature measurement */
#define INA238_REG_CURRENT      0x07    /* Current measurement */
#define INA238_REG_POWER        0x08    /* Power measurement */
#define INA238_REG_DIAG_ALRT    0x0B    /* Diagnostic and Alert */
#define INA238_REG_SOVL         0x0C    /* Shunt Over-Voltage Threshold */
#define INA238_REG_SUVL         0x0D    /* Shunt Under-Voltage Threshold */
#define INA238_REG_BOVL         0x0E    /* Bus Over-Voltage Threshold */
#define INA238_REG_BUVL         0x0F    /* Bus Under-Voltage Threshold */
#define INA238_REG_TEMP_LIMIT   0x10    /* Temperature Over-Limit Threshold */
#define INA238_REG_PWR_LIMIT    0x11    /* Power Over-Limit Threshold */
#define INA238_REG_MFG_ID       0x3E    /* Manufacturer ID */
#define INA238_REG_DEVICE_ID    0x3F    /* Device ID */

/* INA238 Configuration bits */
#define INA238_CONFIG_RST       (1 << 15)   /* System Reset */

/* ADC Configuration defaults - continuous mode, all measurements */
#define INA238_ADC_CONFIG_DEFAULT   0xFB68  /* Continuous, 1024 averages, 1.1ms */

/* Expected IDs */
#define INA238_MFG_ID_VALUE     0x5449      /* "TI" */
#define INA238_DEVICE_ID_VALUE  0x2381      /* INA238 */

/* Conversion factors */
#define INA238_VBUS_LSB_UV      3125        /* Bus voltage LSB = 3.125mV = 3125uV */
#define INA238_VSHUNT_LSB_NV    5000        /* Shunt voltage LSB = 5uV = 5000nV */
#define INA238_TEMP_LSB_MDC     7812        /* Temperature LSB = 7.8125 m°C = 7812 */

/* INA238 context structure */
typedef struct {
    i2c_inst_t *i2c;
    uint8_t addr;
    float current_lsb;      /* Current LSB in Amps */
    float power_lsb;        /* Power LSB in Watts */
    bool initialized;
} ina238_t;

/* INA238 measurement results */
typedef struct {
    float bus_voltage_v;    /* Bus voltage in Volts */
    float current_ma;       /* Current in milliAmps */
    float power_mw;         /* Power in milliWatts */
    float die_temp_c;       /* Die temperature in Celsius */
    bool valid;             /* True if readings are valid */
} ina238_data_t;

/**
 * @brief Initialize the INA238 driver
 * 
 * @param dev Pointer to INA238 device structure
 * @param i2c I2C instance (i2c0 or i2c1)
 * @param addr I2C address of the device
 * @param shunt_mohms Shunt resistor value in milliohms
 * @param max_current_a Maximum expected current in Amps
 * @return true if initialization successful
 */
bool ina238_init(ina238_t *dev, i2c_inst_t *i2c, uint8_t addr, 
                 float shunt_mohms, float max_current_a);

/**
 * @brief Read all measurements from INA238
 * 
 * @param dev Pointer to INA238 device structure
 * @param data Pointer to data structure to fill
 * @return true if read successful
 */
bool ina238_read(ina238_t *dev, ina238_data_t *data);

/**
 * @brief Read bus voltage only
 * 
 * @param dev Pointer to INA238 device structure
 * @param voltage_v Pointer to store voltage in Volts
 * @return true if read successful
 */
bool ina238_read_voltage(ina238_t *dev, float *voltage_v);

/**
 * @brief Read current only
 * 
 * @param dev Pointer to INA238 device structure
 * @param current_ma Pointer to store current in milliAmps
 * @return true if read successful
 */
bool ina238_read_current(ina238_t *dev, float *current_ma);

/**
 * @brief Verify device identity
 * 
 * @param dev Pointer to INA238 device structure
 * @return true if device ID matches expected value
 */
bool ina238_verify_id(ina238_t *dev);

/**
 * @brief Reset the INA238 device
 * 
 * @param dev Pointer to INA238 device structure
 * @return true if reset successful
 */
bool ina238_reset(ina238_t *dev);

#endif /* INA238_H */
