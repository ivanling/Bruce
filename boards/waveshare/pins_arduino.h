#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include <stdint.h>

// Serial pins
static const uint8_t TX = 43;
static const uint8_t RX = 44;

// I2C pins (for Touch GT911)
static const uint8_t SDA = 6;
static const uint8_t SCL = 5;

// SPI pins (for SD Card and other peripherals)
static const uint8_t SS = 10;
static const uint8_t MOSI = 11;
static const uint8_t MISO = 13;
static const uint8_t SCK = 12;

// Analog pins
static const uint8_t A0 = 1;
static const uint8_t A1 = 2;
static const uint8_t A2 = 3;
static const uint8_t A3 = 4;
static const uint8_t A4 = 5;
static const uint8_t A5 = 6;

#endif /* Pins_Arduino_h */
