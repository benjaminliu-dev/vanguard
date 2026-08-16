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

void setup()
{
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

    if (state != RADIOLIB_ERR_NONE)
    {
        config.radio_ok = false;
        config.strobe_on = true;
        for (uint8_t i = 0; i < 10; i++)
        {
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
    if (Wire.endTransmission() != 0)
    {
        Serial0.println(F("GPS Hardware Failure"));
        config.gps_ok = false;
    }
    else
    {
        gps.begin(0x10);
        gps.sendCommand(PMTK_SET_NMEA_OUTPUT_OFF);
        gps.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);

        config.gps_ok = true;
        Serial0.println(F("GPS Ready"));
        delay(250);
    }

#elif I2CDEV == CRYPTO
    if (strlen(KEY) != 16)
    {
        Serial0.println(F("Invalid encryption key"));
        config.crypto_ok = false;
    }
    else
    {
        config.encrypt_comms = true;
        memcpy(config.key, KEY, 16);

        if (atcab_init(&cfg_ateccx08a_i2c_default) != ATCA_SUCCESS)
        {
            Serial0.println(F("Failed to initialize ATCA"));
            config.crypto_ok = false;
        }
        else
        {
            config.crypto_ok = true;
            Serial0.println(F("Encryption Ready"));
        }
    }

#elif I2CDEV == ETD
    if (!aq.begin())
    {
        Serial0.println(F("AQ initialization failed"));
        config.aq_sensor_ok = false;
    }
    else
    {
        config.use_aq_sensor = true;
        config.log_aq_sensor = LOG_AQ;
        config.aq_sensor_interval_s = LOG_AQ_INTERVAL_S;
        Serial0.println(F("ETD Sensing Ready"));
    }

#else
    // Default fallback
    Wire.end();
#endif

    if (config.debug_mode)
    {
        Serial0.println(F("Debug Mode Enabled"));
        Serial0.println(F("Debug Information:"));
        Serial0.print(F("Radio: "));
        Serial0.println(config.radio_ok ? F("OK") : F("FAIL"));
        Serial0.print(F("GPS: "));
        Serial0.println(config.gps_ok ? F("OK") : F("FAIL"));
        Serial0.print(F("Crypto: "));
        Serial0.println(config.crypto_ok ? F("OK") : F("FAIL"));
        Serial0.print(F("AQ Sensor: "));
        Serial0.println(config.aq_sensor_ok ? F("OK") : F("FAIL"));

        while (true)
        {
            Serial0.print("> ");
            while (!Serial0.available())
            {
                delay(10);
            }
            String input = Serial0.readStringUntil('\n');
            input.trim();
            if (input.equalsIgnoreCase("exit"))
            {
                break;
            }
            else if (input == "cfgdump")
            {
                Serial0.println(F("Current Configuration:"));
                Serial0.print(F("Charging: "));
                Serial0.println(config.charging);
                Serial0.print(F("Strobe On: "));
                Serial0.println(config.strobe_on);
                Serial0.print(F("Button Pressed: "));
                Serial0.println(config.button);
                Serial0.print(F("Radio OK: "));
                Serial0.println(config.radio_ok);
                Serial0.print(F("Log Radio: "));
                Serial0.println(config.log_radio);
                Serial0.print(F("Log Sysinfo: "));
                Serial0.println(config.log_sysinfo);
                Serial0.print(F("Debug Mode: "));
                Serial0.println(config.debug_mode);
                Serial0.print(F("Use GPS Ping: "));
                Serial0.println(config.use_gps_ping);
                Serial0.print(F("Log GPS: "));
                Serial0.println(config.log_gps);
                Serial0.print(F("GPS Ping Interval (s): "));
                Serial0.println(config.gps_ping_interval_s);
                Serial0.print(F("GPS OK: "));
                Serial0.println(config.gps_ok);
                Serial0.print(F("Encrypt Comms: "));
                Serial0.println(config.encrypt_comms);
                Serial0.print(F("Crypto OK: "));
                Serial0.println(config.crypto_ok);
                Serial0.print(F("Use AQ Sensor: "));
                Serial0.println(config.use_aq_sensor);
                Serial0.print(F("Log AQ Sensor: "));
                Serial0.println(config.log_aq_sensor);
                Serial0.print(F("AQ Sensor Interval (s): "));
                Serial0.println(config.aq_sensor_interval_s);
                Serial0.print(F("AQ Sensor OK: "));
                Serial0.println(config.aq_sensor_ok);
            }
            else if (input == "gpsdump")
            {
#if I2CDEV == GPS
                Serial0.println(F("Current GPS Information:"));
                if (config.gps_ok)
                {
                    char c = gps.read();

                    if (gps.newNMEAreceived())
                    {
                        if (!gps.parse(gps.lastNMEA()))
                        {
                            return;
                        }
                    }

                    // Check if the GPS has a satellite lock
                    if (gps.fix)
                    {
                        Serial0.print("Latitude: ");
                        Serial0.print(gps.latitudeDegrees, 6); // Use latitudeDegrees for standard decimal format
                        Serial0.print(" ");
                        Serial0.println(gps.lat); // N or S

                        Serial0.print("Longitude: ");
                        Serial0.print(gps.longitudeDegrees, 6); // Use longitudeDegrees for standard decimal format
                        Serial0.print(" ");
                        Serial0.println(gps.lon); // E or W
                    }
                    else
                    {
                        Serial0.println("Waiting for GPS fix...");
                    }
                }
#else
                Serial0.println(F("GPS functionality is not enabled in this build."));
#endif
            }
            else if (input == "aqdump")
            {
#if I2CDEV == ETD
                Serial0.println(F("Current AQ Sensor Information:"));
                if (config.aq_sensor_ok)
                {
                    uint16_t eCO2, TVOC;
                    if (aq.IAQmeasure(&eCO2, &TVOC))
                    {
                        Serial0.print(F("eCO2: "));
                        Serial0.print(eCO2);
                        Serial0.print(F(" ppm, TVOC: "));
                        Serial0.print(TVOC);
                        Serial0.println(F(" ppb"));
                    }
                    else
                    {
                        Serial0.println(F("Failed to read from AQ sensor."));
                    }
                }
#else
                Serial0.println(F("AQ Sensor functionality is not enabled in this build."));
#endif
            }
            else if (input == "help")
            {
                Serial0.println(F("Available commands:"));
                Serial0.println(F("cfgdump - Dump current configuration"));
                Serial0.println(F("gpsdump - Dump current GPS information (if enabled)"));
                Serial0.println(F("aqdump - Dump current AQ sensor information (if enabled)"));
                Serial0.println(F("erase_messages - Erase all stored messages"));
                Serial0.println(F("help - Show available commands"));
                Serial0.println(F("exit - Exit debug mode"));
            }
            else if (input == "erase_messages") {
                memset(config.messages, 0, sizeof(config.messages));
                Serial0.println(F("Messages erased."));
            }
            else
            {
                Serial0.println(F("Unknown command. Run 'help' for a list of available commands."));
            }
        }
    }
}

void loop()
{
    // Main logic
}