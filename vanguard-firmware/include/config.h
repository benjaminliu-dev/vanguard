#pragma once
#include <Arduino.h>

enum I2CFunction {
    GPS = 1, // Adafruit Mini GPS PA1010D - 1
    CRYPTO = 2, // ATECC608A Cryptography Co-Processor - 2
    ETD = 3, // SGP30 AQ Sensor - 3
    NONE = 4 // 4 in build flags
};

struct Config {
    // General configuration
    uint8_t charging    : 1;
    uint8_t strobe_on   : 1;
    uint8_t button      : 1;
    uint8_t radio_ok    : 1;
    uint8_t log_radio   : 1;
    uint8_t log_sysinfo : 1;
    uint8_t debug_mode  : 1;

    // GPS configuration
    uint8_t use_gps_ping        : 1;
    uint8_t log_gps             : 1;
    uint16_t gps_ping_interval_s : 10;
    uint8_t gps_ok              : 1;

    // Crypto configuration
    uint8_t encrypt_comms       : 1;
    byte key[16];
    uint8_t crypto_ok           : 1;

    // Environmental thread detection configuration
    uint8_t use_aq_sensor       : 1;
    uint8_t log_aq_sensor       : 1;
    uint16_t aq_sensor_interval_s : 10;
    uint8_t aq_sensor_ok          : 1;

    char messages[10][20]; 
};
