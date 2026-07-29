#include <Arduino.h>
#include <RadioLib.h>
#include <Wire.h>
#include "config.h" // MUST be included before the #if checks below

// GPIOs
#define BUTTON_PIN PIN_PC5
#define STROBE_PIN PIN_PC2
#define CHARGE_STATUS_PIN PIN_PB0

// RADIO
#define LR_CS PIN_PA4
#define LR_DIO1 PIN_PB5
#define LR_RST PIN_PB4
#define LR_BUSY PIN_PB6


// --- COMPILE-TIME OPTIMIZATION ---
// We only include heavy libraries if they are actively selected in config.h.
// NOTE: Ensure your config.h defines I2CDEV using `#define I2CDEV GPS`
// rather than `const int` or `enum`, otherwise the preprocessor cannot read it.

#if I2CDEV == GPS
    #include <Adafruit_GPS.h>
    Adafruit_GPS gps(&Wire);
#elif I2CDEV == CRYPTO
    #include <cryptoauthlib.h>
#elif I2CDEV == ETD
    #include <Adafruit_SGP30.h>
    Adafruit_SGP30 aq;
#endif

ArduinoHal hal;
SX1262 radio = new Module(&hal, LR_CS, LR_DIO1, LR_RST, LR_BUSY);
Config config = {};

void setup() {
    // Basic hardware setup
    pinMode(BUTTON_PIN, INPUT);
    pinMode(STROBE_PIN, OUTPUT);
    pinMode(CHARGE_STATUS_PIN, INPUT);

    Serial0.begin(9600);

    // Initialize base config directly (saves Stack Memory vs aggregate {} initialization)
    config.debug_mode = (digitalRead(BUTTON_PIN) == HIGH);
    config.charging = (digitalRead(CHARGE_STATUS_PIN) == HIGH);
    config.button = false;
    config.log_radio = LOG_RADIO;
    config.log_sysinfo = LOG_SYSINFO;
    config.strobe_on = false;
    config.radio_ok = true;

    // Radio setup
    int16_t state = radio.begin(915.0, 125.0, 9, 7, 0x12, 10, 8);

    if (state != RADIOLIB_ERR_NONE) {
        config.radio_ok = false;
        config.strobe_on = true;
        for (uint8_t i = 0; i < 10; i++) {
            digitalWrite(STROBE_PIN, HIGH);
            delay(250);
            digitalWrite(STROBE_PIN, LOW);
            delay(250);
        }
    }

    // I2C setup
    Wire.begin();

#if I2CDEV == GPS
    config.use_gps_ping = true;
    config.log_gps = true;
    config.gps_ping_interval_s = 600;

    Wire.beginTransmission(0x10);
    if (Wire.endTransmission() != 0) {
        Serial0.println(F("GPS Hardware Failure"));
        config.gps_ok = false;
    } else {
        gps.begin(0x10);
        gps.sendCommand(PMTK_SET_NMEA_OUTPUT_OFF);
        gps.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);

        config.gps_ok = true;
        Serial0.println(F("GPS Ready"));
        delay(250);
    }

#elif I2CDEV == CRYPTO
    if (strlen(KEY) != 16) {
        Serial0.println(F("Invalid encryption key"));
        config.crypto_ok = false;
    } else {
        config.encrypt_comms = true;
        memcpy(config.key, KEY, 16);

        if (atcab_init(&cfg_ateccx08a_i2c_default) != ATCA_SUCCESS) {
            Serial0.println(F("Failed to initialize ATCA"));
            config.crypto_ok = false;
        } else {
            config.crypto_ok = true;
            Serial0.println(F("Encryption Ready"));
        }
    }

#elif I2CDEV == ETD
    if (!aq.begin()) {
        Serial0.println(F("AQ initialization failed"));
        config.aq_sensor_ok = false;
    } else {
        config.use_aq_sensor = true;
        config.log_aq_sensor = LOG_AQ;
        config.aq_sensor_interval_s = LOG_AQ_INTERVAL_S;
        Serial0.println(F("ETD Sensing Ready"));
    }

#else
    // Default fallback
    Wire.end();
#endif

}

void loop() {
    // Main logic
}