#ifndef SOFTWARE_SERIAL_H
#define SOFTWARE_SERIAL_H

#ifdef __cplusplus
#include <Arduino.h>
class SoftwareSerial {
public:
    SoftwareSerial(int rx, int tx) {}
    void begin(long speed) {}
    bool available() { return false; }
    int read() { return -1; }
    size_t write(uint8_t b) { return 0; }
};
#endif

#endif
