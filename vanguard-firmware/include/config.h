#include <Arduino.h>

struct Config { 
    uint8_t charging    : 1;
    uint8_t strobe_on   : 1;
    uint8_t button      : 1;
    uint8_t radio_ok    : 1;
    uint8_t log_radio   : 1;
    uint8_t log_sysinfo : 1;
    uint8_t debug_mode  : 1;
    char config_path[16]; 
    char messages[10][20]; 
};
