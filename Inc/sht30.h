#ifndef SHT30_H
#define SHT30_H

// #include "chillib.h"
#include "Arduino.h"
#include "Wire.h"

#ifndef SHT30_I2C_ADDR
#define SHT30_I2C_ADDR 0x44
#endif

#define SHT30_CRC_POLYNOMIAL 0b10011001

// See datasheet at https://sensirion.com/media/documents/213E6A3B/63A5A569/Datasheet_SHT3x_DIS.pdf

typedef struct sht30_data {
  uint16_t temperature;
  uint16_t humidity;
  uint8_t  valid;
} sht30_result;

typedef enum sht30_repeatability_t {
    REPEAT_LOW    = 0x06,
    REPEAT_MEDIUM = 0x0D,
    REPEAT_HIGH   = 0x10
} sht30_repeatability_t;

typedef enum sht30_clock_stretch_t {
    ENABLED  = 0x2C,
    DISABLED = 0x24,
} sht30_clock_stretch_t;

typedef struct sht30_config {

    uint8_t i2c_address;
    sht30_repeatability_t repeatability;
    sht30_clock_stretch_t clock_stretch;
    TwoWire* i2c_handle;

} sht30_config;

CHIL_STATUS sht30_init(sht30_config* config, TwoWire* i2c);
CHIL_STATUS sht30_request(sht30_config* config, sht30_result* results);
CHIL_STATUS sht30_read(sht30_config* config, sht30_result* results);
float sht30_temperature_degC(sht30_result* results);
float sht30_relative_humidity(sht30_result* results);

#endif 