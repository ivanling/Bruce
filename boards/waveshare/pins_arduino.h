#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include "soc/soc_caps.h"
#include <stdint.h>

// ============================================================================
// Waveshare ESP32-S3-Touch-LCD-4.3
// Display: 800x480 RGB LCD (ST7262)
// Touch: GT911 I2C (via CH422G I/O expander for reset)
// ============================================================================

// SPI pins for SD Card and peripherals
#define SPI_SS_PIN 10
#define SPI_MOSI_PIN 11
#define SPI_MISO_PIN 13
#define SPI_SCK_PIN 12

// SD Card (directly on SPI)
#define SDCARD_CS SPI_SS_PIN
#define SDCARD_SCK SPI_SCK_PIN
#define SDCARD_MISO SPI_MISO_PIN
#define SDCARD_MOSI SPI_MOSI_PIN

// CC1101 Module pins (directly on SPI)
#define CC1101_GDO0_PIN -1
#define CC1101_SS_PIN SPI_SS_PIN
#define CC1101_MOSI_PIN SPI_MOSI_PIN
#define CC1101_SCK_PIN SPI_SCK_PIN
#define CC1101_MISO_PIN SPI_MISO_PIN

// NRF24 Module pins (directly on SPI)
#define NRF24_CE_PIN -1
#define NRF24_SS_PIN SPI_SS_PIN
#define NRF24_MOSI_PIN SPI_MOSI_PIN
#define NRF24_SCK_PIN SPI_SCK_PIN
#define NRF24_MISO_PIN SPI_MISO_PIN

// W5500 Ethernet (optional, via SPI)
#define USE_W5500_VIA_SPI
#define W5500_SS_PIN SPI_SS_PIN
#define W5500_MOSI_PIN SPI_MOSI_PIN
#define W5500_SCK_PIN SPI_SCK_PIN
#define W5500_MISO_PIN SPI_MISO_PIN
#define W5500_INT_PIN -1

// I2C Bus for Touch (GT911) and CH422G
// These are the actual I2C pins on Waveshare 4.3
#define GROVE_SDA 8
#define GROVE_SCL 9
static const uint8_t SDA = GROVE_SDA;
static const uint8_t SCL = GROVE_SCL;

// Serial pins
#define SERIAL_TX 43
#define SERIAL_RX 44
#define GPS_SERIAL_TX SERIAL_TX
#define GPS_SERIAL_RX SERIAL_RX

// LED pins (directly on GPIO, optional)
#define TXLED 17
#define RXLED 18

// Arduino standard SPI pins
static const uint8_t SS = SPI_SS_PIN;
static const uint8_t MOSI = SPI_MOSI_PIN;
static const uint8_t SCK = SPI_SCK_PIN;
static const uint8_t MISO = SPI_MISO_PIN;

// Arduino standard Serial pins
static const uint8_t TX = SERIAL_TX;
static const uint8_t RX = SERIAL_RX;

// ============================================================================
// Display Configuration - Arduino_GFX RGB Panel
// ============================================================================
#define HAS_SCREEN
#define ROTATION 1
#define MINBRIGHT (uint8_t)1

// Font Sizes
#define FP 1
#define FM 2
#define FG 3

// Smooth fonts for TFT_eSPI compatibility
#define SMOOTH_FONT 1

// Touch Configuration
#define HAS_TOUCH 1

// ============================================================================
// Button & Navigation (Touch-only device)
// ============================================================================
#define BTN_ALIAS "\"TAP\""
#define REBOOT_PIN -1

// No physical buttons - touch only
#define SEL_BTN -1
#define UP_BTN -1
#define DW_BTN -1
#define BTN_ACT LOW

// ============================================================================
// IR/RF pins (directly on GPIO, optional modules)
// ============================================================================
#define LED_ON HIGH
#define LED_OFF LOW

// ============================================================================
// BadUSB / USB HID
// ============================================================================
#define USB_as_HID 1

// ============================================================================
// Mic (not available on this board)
// ============================================================================
#define PIN_CLK -1
#define PIN_DATA -1

// ============================================================================
// Battery (not available - USB powered only)
// ============================================================================
#define PWR_EN_PIN -1
#define PWR_ON_PIN -1
#define ANALOG_BAT_PIN -1

#endif /* Pins_Arduino_h */
