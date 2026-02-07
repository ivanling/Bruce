#include "core/powerSave.h"
#include "core/utils.h"
#include <Arduino.h>
#include <Wire.h>
#include <interface.h>

// CH422G I/O Expander addresses (controls backlight and touch reset)
#define CH422G_ADDR_MODE 0x24 // Configuration register
#define CH422G_ADDR_DATA 0x38 // Data output register

#if defined(TOUCH_GT911_I2C)
#include "TouchDrvGT911.hpp"
TouchDrvGT911 touch;
struct TouchPointPro {
    int16_t x = 0;
    int16_t y = 0;
};
#endif

/***************************************************************************************
** Function name: ch422g_write
** Description:   Write a byte to CH422G I/O expander via I2C
***************************************************************************************/
static void ch422g_write(uint8_t addr, uint8_t data) {
    Wire.beginTransmission(addr);
    Wire.write(data);
    Wire.endTransmission();
}

/***************************************************************************************
** Function name: ch422g_init
** Description:   Initialize CH422G I/O expander for backlight and touch control
***************************************************************************************/
static void ch422g_init() {
    // Set CH422G to output mode
    ch422g_write(CH422G_ADDR_MODE, 0x01);
    delay(10);
}

/***************************************************************************************
** Function name: ch422g_backlight_on
** Description:   Turn on backlight via CH422G (bit 4 = backlight)
***************************************************************************************/
static void ch422g_backlight_on() {
    // 0x1E = 0001 1110 - enables backlight and other outputs
    ch422g_write(CH422G_ADDR_DATA, 0x1E);
}

/***************************************************************************************
** Function name: ch422g_touch_reset
** Description:   Reset GT911 touch controller via CH422G I/O expander
***************************************************************************************/
static void ch422g_touch_reset() {
    // Set CH422G to output mode
    ch422g_write(CH422G_ADDR_MODE, 0x01);

    // Assert touch reset (EXIO1 low) - 0x2C keeps backlight on, touch reset
    ch422g_write(CH422G_ADDR_DATA, 0x2C);
    delay(100);

    // Set GPIO4 (touch INT) low during reset
    pinMode(4, OUTPUT);
    digitalWrite(4, LOW);
    delay(100);

    // Release touch reset - 0x2E keeps backlight on, touch released
    ch422g_write(CH422G_ADDR_DATA, 0x2E);
    delay(200);

    // Return GPIO4 to input
    pinMode(4, INPUT);
}

/***************************************************************************************
** Function name: _setup_gpio()
** Location: main.cpp
** Description:   initial setup for the Waveshare ESP32-S3-Touch-LCD-4.3
***************************************************************************************/
void _setup_gpio() {
    // Initialize I2C for CH422G and GT911 (shared bus)
    Wire.begin(GT911_I2C_CONFIG_SDA_IO_NUM, GT911_I2C_CONFIG_SCL_IO_NUM, 100000);
    delay(50);

    // Initialize CH422G I/O expander
    ch422g_init();
    Serial.println("CH422G I/O expander initialized");

    // Turn on backlight via CH422G
    ch422g_backlight_on();
    Serial.println("Backlight enabled via CH422G");

#if defined(TOUCH_GT911_I2C)
    // Reset touch controller via CH422G
    ch422g_touch_reset();
    Serial.println("GT911 touch reset via CH422G");

    // Don't call setPins - the library's setPins(-1, X) tries to pinMode(-1) which fails
    // Instead, configure the interrupt pin manually
    pinMode(BOARD_TOUCH_INT, INPUT);

    if (!touch.begin(Wire, GT911_SLAVE_ADDRESS_L, GT911_I2C_CONFIG_SDA_IO_NUM, GT911_I2C_CONFIG_SCL_IO_NUM)) {
        Serial.println("Failed to find GT911 - check your wiring!");
    } else {
        Serial.println("GT911 Touch initialized successfully");
    }
#endif

    bruceConfig.colorInverted = 0;
}

/***************************************************************************************
** Function name: _post_setup_gpio()
** Location: main.cpp
** Description:   second stage gpio setup for Waveshare display
***************************************************************************************/
void _post_setup_gpio() {
    // Backlight is controlled by CH422G I/O expander, not GPIO
    // Ensure it's still on after display init
    ch422g_backlight_on();
    Serial.println("Post-setup: Backlight confirmed via CH422G");
}

/*********************************************************************
** Function: setBrightness
** location: settings.cpp
** set brightness value for Waveshare display
** Note: True brightness control not available via CH422G (on/off only)
**********************************************************************/
void _setBrightness(uint8_t brightval) {
    // CH422G only supports on/off for backlight - no PWM dimming
    if (brightval > 0) {
        ch422g_backlight_on();
    } else {
        // Turn off backlight - keep other outputs
        ch422g_write(CH422G_ADDR_DATA, 0x0E); // 0x0E = backlight off, others on
    }
}

/*********************************************************************
** Function: InputHandler
** Handles touch input for GT911 capacitive touch
**********************************************************************/
void InputHandler(void) {
    static long d_tmp = 0;

    if (millis() - d_tmp > 200 || LongPress) {
#if defined(TOUCH_GT911_I2C)
        static unsigned long tm = millis();
        TouchPointPro t;
        uint8_t touched = 0;
        static uint8_t rot = 5;

        // Configure touch orientation based on screen rotation
        if (rot != bruceConfigPins.rotation) {
            if (bruceConfigPins.rotation == 1) {
                touch.setMaxCoordinates(TFT_HEIGHT, TFT_WIDTH);
                touch.setSwapXY(true);
                touch.setMirrorXY(false, true);
            }
            if (bruceConfigPins.rotation == 3) {
                touch.setMaxCoordinates(TFT_HEIGHT, TFT_WIDTH);
                touch.setSwapXY(true);
                touch.setMirrorXY(true, false);
            }
            if (bruceConfigPins.rotation == 0) {
                touch.setMaxCoordinates(TFT_WIDTH, TFT_HEIGHT);
                touch.setSwapXY(false);
                touch.setMirrorXY(false, false);
            }
            if (bruceConfigPins.rotation == 2) {
                touch.setMaxCoordinates(TFT_WIDTH, TFT_HEIGHT);
                touch.setSwapXY(false);
                touch.setMirrorXY(true, true);
            }
            rot = bruceConfigPins.rotation;
        }

        // Track touch state to prevent double events
        static bool lastTouchState = false;
        static unsigned long lastTouchTime = 0;

        touched = touch.getPoint(&t.x, &t.y);
        bool currentTouchState = touched > 0;

        // Debouncing - only process new touch presses
        if (currentTouchState && !lastTouchState && (millis() - lastTouchTime) > 100) {
            lastTouchTime = millis();
        } else if (!currentTouchState || lastTouchState) {
            touched = 0;
        }
        lastTouchState = currentTouchState;

        if (((millis() - tm) > 190 || LongPress) && touched) {
            tm = millis();

            checkPowerSaveTime();

            // Map touch coordinates to screen buttons
            // This needs to be adjusted based on your UI layout
            if (t.y < TFT_HEIGHT / 3) {
                PrevPress = true;
            } else if (t.y > (2 * TFT_HEIGHT) / 3) {
                NextPress = true;
            } else {
                SelPress = true;
            }

            // Store touch point for other uses
            touchPoint.x = t.x;
            touchPoint.y = t.y;
            touchPoint.pressed = true;

            AnyKeyPress = true;
            d_tmp = millis();
        }
#endif
    }
}

/*********************************************************************
** Function: checkReboot
** Verifies if the touch is used in special setup Reboot
**********************************************************************/
#if REBOOT_PIN >= 0
void checkReboot() {
#if defined(TOUCH_GT911_I2C)
    TouchPointPro t;
    uint8_t touched = touch.getPoint(&t.x, &t.y);
    if (touched > 0) {
        int countDown;
        uint32_t time_count = millis();
        while (touch.getPoint(&t.x, &t.y) > 0) {
            if (millis() - time_count > 500) {
                tft.setTextSize(1);
                tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
                countDown = (millis() - time_count) / 1000 + 1;
                if (countDown < 3)
                    tft.drawCentreString("PWR OFF IN " + String(countDown) + "/2", tftWidth / 2, 12, 1);
                else {
                    tft.fillScreen(bruceConfig.bgColor);
                    while (touch.getPoint(&t.x, &t.y) > 0);
                    delay(200);
                    powerOff();
                }
                delay(10);
            }
        }
        delay(30);
        tft.fillRect(60, 12, tftWidth - 60, tft.fontHeight(), bruceConfig.bgColor);
    }
#else
    if (digitalRead(REBOOT_PIN) == BTN_ACT) {
        int countDown;
        uint32_t time_count = millis();
        while (digitalRead(REBOOT_PIN) == BTN_ACT) {
            if (millis() - time_count > 500) {
                tft.setTextSize(1);
                tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
                countDown = (millis() - time_count) / 1000 + 1;
                if (countDown < 3)
                    tft.drawCentreString("PWR OFF IN " + String(countDown) + "/2", tftWidth / 2, 12, 1);
                else {
                    tft.fillScreen(bruceConfig.bgColor);
                    while (digitalRead(REBOOT_PIN) == BTN_ACT);
                    delay(200);
                    powerOff();
                }
                delay(10);
            }
        }
        delay(30);
        tft.fillRect(60, 12, tftWidth - 60, tft.fontHeight(1), bruceConfig.bgColor);
    }
#endif
}
#endif
