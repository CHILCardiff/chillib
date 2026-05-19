#include "chillib.h"
#ifdef ARDUINO_PIO

#include "Arduino.h"
#include "Wire.h"

#include "sht30.h"


CHIL_STATUS sht30_init(sht30_config* config, TwoWire* i2c) {
    // Initialise default values to struct
    config->repeatability = REPEAT_HIGH;
    config->clock_stretch = ENABLED;
    config->i2c_address   = SHT30_I2C_ADDR;
    config->i2c_handle    = i2c;
    i2c->begin();
    return CHIL_OK;
}

uint8_t sht30_calculate_crc(uint16_t result) {

}

CHIL_STATUS sht30_read(sht30_config* config, sht30_result* results) {

    results->valid = 0;
    
    // Begin transmission
    config->i2c_handle->beginTransmission(config->i2c_address);
    config->i2c_handle->write((uint8_t) config->clock_stretch);
    config->i2c_handle->write((uint8_t) config->repeatability);

    uint8_t received = config->i2c_handle->endTransmission();

    if (received == 0) {

        // We can request a measurement (max of 12.5ms)
        delay(15);

        // See how much data is returned
        received = config->i2c_handle->requestFrom(config->i2c_address, 6);

        if (received == 6) {
            
            uint8_t crc;
            // Read temperature
            uint16_t tmp_buffer = config->i2c_handle->read();
            tmp_buffer <<= 8;
            tmp_buffer |= config->i2c_handle->read();
            results->temperature = tmp_buffer;
            // Read CRC byte for temperature
            crc = config->i2c_handle->read();

            // Read relative humidity
            tmp_buffer = config->i2c_handle->read();
            tmp_buffer <<= 8;
            tmp_buffer |= config->i2c_handle->read();
            results->humidity = tmp_buffer;
            // Read CRC byte for relative humdity
            crc = config->i2c_handle->read();

            // until we've deciphered the datasheet we'll ignore the CRC
            results->valid = 1;

            return CHIL_OK;
            
        } else return CHIL_ERROR;

    } else return CHIL_ERROR;
}

float sht30_temperature_degC(sht30_result* results) {
    if (results->valid) 
        return ((float)results->temperature)*175.0/(65535.0) - 45.0;
    else
        return (float)0xffffffff;
}

float sht30_relative_humidity(sht30_result* results) {
    if (results->valid)
        return Serial.println(((float)results->humidity)*175.0/(65535.0)-45);
    else 
        return (float)0xffffffff;
}

#endif