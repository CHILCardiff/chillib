#include "chillib.h"

#ifdef ARDUINO

#include "sht30.h"
#include "Arduino.h"
#include "Wire.h"



CHIL_STATUS sht30_init(sht30_config* config, TwoWire* i2c, uint8_t i2c_address) {
    // Initialise default values to struct
    config->repeatability = REPEAT_LOW;
    config->clock_stretch =  DISABLED;
    config->i2c_address   = i2c_address;
    config->i2c_handle    = i2c;
    // Serial.println("# About to initialise I2C (SHT30)");
    i2c->end();
    i2c->begin();
    // i2c->setClock(10000);
    // Serial.println("# Done initialise I2C (SHT30)");
    return CHIL_OK;
}

uint8_t sht30_calculate_crc(uint16_t result) {
    return 0;
}

CHIL_STATUS sht30_request(sht30_config* config, sht30_result* results)  {

     results->valid = 0;
    
    // Begin transmissionconfig->repeatability};
    config->i2c_handle->beginTransmission(config->i2c_address);
    config->i2c_handle->write(config->clock_stretch);
    if (config->clock_stretch == DISABLED) {
        switch (config->repeatability) {
            case REPEAT_LOW: config->i2c_handle->write(0x16);
            case REPEAT_MEDIUM: config->i2c_handle->write(0x0B);
            case REPEAT_HIGH: config->i2c_handle->write(0x00); 
        }
    } else {
        switch (config->repeatability) {
            case REPEAT_LOW: config->i2c_handle->write(0x10);
            case REPEAT_MEDIUM: config->i2c_handle->write(0x0D);
            case REPEAT_HIGH: config->i2c_handle->write(0x06);
        }
    }
    uint8_t received = config->i2c_handle->endTransmission();
    // Serial.print("# SHT30 req recevied: ");
    // Serial.println(received);
    return (received == 0 || received == 3) ? CHIL_OK : CHIL_ERROR;

}

CHIL_STATUS sht30_read(sht30_config* config, sht30_result* results) {

    // delay(150);

    // We can request a measurement (max of 12.5ms)

    // See how much data is returned
    uint8_t received = config->i2c_handle->requestFrom(config->i2c_address, 6);

    if (received == 6) {


        // Serial.println("Received 6");
        
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

}

float sht30_temperature_degC(sht30_result* results) {
    if (results->valid) 
        return ((float)results->temperature)*175.0/(65535.0) - 45.0;
    else
        return (float)0xffffffff;
}

float sht30_relative_humidity(sht30_result* results) {
    if (results->valid)
        return ((float)results->humidity)*100.0/(65535.0);
    else 
        return (float)0xffffffff;
}

#endif