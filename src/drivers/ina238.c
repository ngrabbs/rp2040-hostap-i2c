/*
 * INA238 I2C Driver Implementation
 */

#include "ina238.h"
#include "pico/stdlib.h"
#include <string.h>
#include <stdio.h>

/* I2C timeout in microseconds */
#define I2C_TIMEOUT_US  100000

/* Helper function to write a 16-bit register */
static bool ina238_write_reg(ina238_t *dev, uint8_t reg, uint16_t value) {
    uint8_t buf[3];
    buf[0] = reg;
    buf[1] = (value >> 8) & 0xFF;   /* MSB first */
    buf[2] = value & 0xFF;
    
    int ret = i2c_write_timeout_us(dev->i2c, dev->addr, buf, 3, false, I2C_TIMEOUT_US);
    return (ret == 3);
}

/* Helper function to read a 16-bit register */
static bool ina238_read_reg(ina238_t *dev, uint8_t reg, uint16_t *value) {
    uint8_t buf[2];
    
    /* Write register address */
    int ret = i2c_write_timeout_us(dev->i2c, dev->addr, &reg, 1, true, I2C_TIMEOUT_US);
    if (ret != 1) {
        return false;
    }
    
    /* Read register value */
    ret = i2c_read_timeout_us(dev->i2c, dev->addr, buf, 2, false, I2C_TIMEOUT_US);
    if (ret != 2) {
        return false;
    }
    
    *value = ((uint16_t)buf[0] << 8) | buf[1];
    return true;
}

bool ina238_init(ina238_t *dev, i2c_inst_t *i2c, uint8_t addr,
                 float shunt_mohms, float max_current_a) {
    if (!dev || !i2c) {
        return false;
    }
    
    dev->i2c = i2c;
    dev->addr = addr;
    dev->initialized = false;
    
    /* Verify device ID */
    if (!ina238_verify_id(dev)) {
        return false;
    }
    
    /* Reset device */
    if (!ina238_reset(dev)) {
        return false;
    }
    
    /* Wait for reset to complete */
    sleep_ms(100);
    

    
    /* Calculate calibration value
     * SHUNT_CAL = 819.2 * 10^6 * CURRENT_LSB * Rshunt
     * where CURRENT_LSB = max_expected_current / 2^15
     * and Rshunt is in Ohms
     */
    dev->current_lsb = max_current_a / 32768.0f;
    dev->power_lsb = dev->current_lsb * 0.2f;   /* Power LSB = 0.2 * Current LSB */
    
    float shunt_ohms = shunt_mohms / 1000.0f;
    uint16_t shunt_cal = (uint16_t)(819.2e6f * dev->current_lsb * shunt_ohms);
    
    /* Write calibration register */
    if (!ina238_write_reg(dev, INA238_REG_SHUNT_CAL, shunt_cal)) {
        return false;
    }
    
    /* Configure ADC for continuous measurements
     * Bits [15:12] MODE = 1011 (0xB) = Continuous shunt and bus voltage
     * Bits [11:9]  VBUSCT = 111 = 4120us conversion time
     * Bits [8:6]   VSHCT  = 111 = 4120us conversion time  
     * Bits [5:3]   VTCT   = 111 = 4120us conversion time
     * Bits [2:0]   AVG    = 000 = 1 sample
     * = 0xBFF8
     */
    uint16_t adc_config = 0xBFF8;
    if (!ina238_write_reg(dev, INA238_REG_ADC_CONFIG, adc_config)) {
        return false;
    }
    
    dev->initialized = true;
    return true;
}

bool ina238_verify_id(ina238_t *dev) {
    uint16_t mfg_id, device_id;
    
    if (!ina238_read_reg(dev, INA238_REG_MFG_ID, &mfg_id)) {
        return false;
    }
    
    if (!ina238_read_reg(dev, INA238_REG_DEVICE_ID, &device_id)) {
        return false;
    }
    
    /* Verify manufacturer ID (TI = 0x5449) */
    if (mfg_id != INA238_MFG_ID_VALUE) {
        return false;
    }
    
    /* Device ID should be 0x238x for INA238 */
    if ((device_id & 0xFFF0) != 0x2380) {
        return false;
    }
    
    return true;
}

bool ina238_reset(ina238_t *dev) {
    return ina238_write_reg(dev, INA238_REG_CONFIG, INA238_CONFIG_RST);
}

bool ina238_read(ina238_t *dev, ina238_data_t *data) {
    if (!dev || !data || !dev->initialized) {
        if (data) data->valid = false;
        return false;
    }
    
    memset(data, 0, sizeof(ina238_data_t));
    
    uint16_t vbus_raw, current_raw, power_raw, temp_raw;
    
    /* Read bus voltage register */
    if (!ina238_read_reg(dev, INA238_REG_VBUS, &vbus_raw)) {
        data->valid = false;
        return false;
    }
    
    /* Read current register */
    if (!ina238_read_reg(dev, INA238_REG_CURRENT, &current_raw)) {
        data->valid = false;
        return false;
    }
    
    /* Read power register */
    if (!ina238_read_reg(dev, INA238_REG_POWER, &power_raw)) {
        data->valid = false;
        return false;
    }
    
    /* Read die temperature register */
    if (!ina238_read_reg(dev, INA238_REG_DIETEMP, &temp_raw)) {
        data->valid = false;
        return false;
    }
    
    /* Convert bus voltage: LSB = 3.125mV 
     * Register is 16-bit value, unsigned for positive voltages
     */
    data->bus_voltage_v = (float)vbus_raw * 3.125f / 1000.0f;
    

    
    /* Convert current using calibrated LSB, result in mA */
    data->current_ma = (float)(int16_t)current_raw * dev->current_lsb * 1000.0f;
    
    /* Convert power using calibrated LSB, result in mW */
    data->power_mw = (float)power_raw * dev->power_lsb * 1000.0f;
    
    /* Convert temperature: LSB = 7.8125 m°C, upper 12 bits */
    int16_t temp_signed = (int16_t)temp_raw >> 4;
    data->die_temp_c = (float)temp_signed * 0.0078125f * 16.0f;
    
    data->valid = true;
    return true;
}

bool ina238_read_voltage(ina238_t *dev, float *voltage_v) {
    if (!dev || !voltage_v || !dev->initialized) {
        return false;
    }
    
    uint16_t vbus_raw;
    if (!ina238_read_reg(dev, INA238_REG_VBUS, &vbus_raw)) {
        return false;
    }
    
    /* Convert bus voltage: LSB = 3.125mV */
    *voltage_v = (float)(int16_t)vbus_raw * 3.125f / 1000.0f;
    return true;
}

bool ina238_read_current(ina238_t *dev, float *current_ma) {
    if (!dev || !current_ma || !dev->initialized) {
        return false;
    }
    
    uint16_t current_raw;
    if (!ina238_read_reg(dev, INA238_REG_CURRENT, &current_raw)) {
        return false;
    }
    
    /* Convert current using calibrated LSB, result in mA */
    *current_ma = (float)(int16_t)current_raw * dev->current_lsb * 1000.0f;
    return true;
}
