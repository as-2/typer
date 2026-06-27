#pragma once

#define SCREEN_WIDTH 296
#define SCREEN_HEIGHT 128

#define TEXT_LEFT 0
#define TEXT_TOP 0
#define TEXT_SIZE DISPLAY_TEXT_MEDIUM

#define DEBUG_BLE_KEYBOARD_REPORTS false
#define DOCUMENT_SAVE_IDLE_MS 2000

#define PIN_BUSY 9
#define PIN_RST 10
#define PIN_DC 11
#define PIN_CS 12
#define PIN_CLK 13
#define PIN_DIN 14

// Board-specific USB host VBUS power enable.
// Set PIN_USB_VBUS_EN to the GPIO that turns on 5V power for the OTG port.
// Leave at -1 if the board powers VBUS automatically or the pin is unknown.
#define PIN_USB_VBUS_EN -1
#define USB_VBUS_EN_ACTIVE_HIGH true
