#include "core/powerSave.h"
#include "core/utils.h"
#include <Arduino.h>
#include <interface.h>

#if defined(TOUCH_GT911_I2C)
#include "TouchDrvGT911.hpp"
TouchDrvGT911 touch;
struct TouchPointPro {
    int16_t x = 0;
    int16_t y = 0;
};
#endif

/***************************************************************************************
** Function name: _setup_gpio()
** Location: main.cpp
** Description:   initial setup for the Waveshare ESP32-S3-Touch-LCD-4.3
***************************************************************************************/
void _setup_gpio() {
#if defined(TOUCH_GT911_I2C)
    pinMode(BOARD_TOUCH_INT, INPUT);
    touch.setPins(-1, BOARD_TOUCH_INT);

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
    // Brightness control
    pinMode(TFT_BL, OUTPUT);
    ledcAttach(TFT_BL, TFT_BRIGHT_FREQ, TFT_BRIGHT_Bits);
    ledcWrite(TFT_BL, 255); // Full brightness
}

/*********************************************************************
** Function: setBrightness
** location: settings.cpp
** set brightness value for Waveshare display
**********************************************************************/
void _setBrightness(uint8_t brightval) {
    int dutyCycle;
    if (brightval == 100) dutyCycle = 255;
    else if (brightval == 75) dutyCycle = 130;
    else if (brightval == 50) dutyCycle = 70;
    else if (brightval == 25) dutyCycle = 20;
    else if (brightval == 0) dutyCycle = 0;
    else dutyCycle = ((brightval * 255) / 100);

    ledcWrite(TFT_BL, dutyCycle);
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
