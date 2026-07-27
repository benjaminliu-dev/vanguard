#include <Arduino.h>
#include <RadioLib.h>
#include "config.h"


// GPIOS
#define BUTTON_PIN PIN_PC5
#define STROBE_PIN PIN_PC2
#define CHARGE_STATUS_PIN PIN_PB0
// RADIO
#define LR_CS PIN_PA4
#define LR_DIO1 PIN_PB5
#define LR_RST PIN_PB4
#define LR_BUSY PIN_PB6


ArduinoHal hal;

SX1262 radio = new Module(&hal, LR_CS, LR_DIO1, LR_RST, LR_BUSY);
Config config = {};

void setup() {

    // Basic setup
    pinMode(BUTTON_PIN, INPUT);
    pinMode(STROBE_PIN, OUTPUT);
    pinMode(CHARGE_STATUS_PIN, INPUT);

    // Sense debug mode
    bool debug = false;
    if (digitalRead(BUTTON_PIN) == HIGH) {
        debug = true;
        Serial0.begin(9600);
    }


    // Radio setup
    int state = radio.begin(915.0, 125.0, 9, 7, 0x12, 10, 8);
    bool radio = true;
    bool strobe = false;

    if (state != RADIOLIB_ERR_NONE) {
        radio = false;
        strobe = true;
        for (int i = 0; i < 10; i++) {
            digitalWrite(STROBE_PIN, HIGH);
            delay(250);
            digitalWrite(STROBE_PIN, LOW);
            delay(250);
        }    
    }

    // Power setup
    bool is_charging = digitalRead(CHARGE_STATUS_PIN);
    
    // Config setup
    config = {
        .charging = is_charging,
        .strobe_on = strobe,
        .button = false,
        .radio_ok = radio,
        .log_radio = LOG_RADIO,
        .log_sysinfo = LOG_SYSINFO,
        .debug_mode = debug
    };
}

void loop() {

}