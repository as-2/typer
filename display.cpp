#include "display.h"
#include "config.h"

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <GxEPD2_BW.h>

// Create the one global display object
Display display;

// Create the e-paper driver
GxEPD2_BW<GxEPD2_290_T94_V2, GxEPD2_290_T94_V2::HEIGHT> epd(
    GxEPD2_290_T94_V2(
        PIN_CS,
        PIN_DC,
        PIN_RST,
        PIN_BUSY
    )
);

void Display::begin() {
    Serial.begin(115200);
    Serial.println("Initializing display...");

#if defined(ESP32)
    SPI.begin(
        PIN_CLK,    // SCK
        -1,         // MISO (unused)
        PIN_DIN,    // MOSI
        PIN_CS      // SS
    );
#else
    SPI.begin();
#endif

    epd.init();
    epd.setRotation(-1);
    Serial.println("Display ready!");
}

void Display::clear() {
    epd.setFullWindow();
    epd.firstPage();

    do
    {
        epd.fillScreen(GxEPD_WHITE);
    }
    while (epd.nextPage());
}

void Display::printText(int x, int y, const char* text) {
    epd.setFullWindow();

    epd.firstPage();

    do {
        epd.fillScreen(GxEPD_WHITE);

        epd.setTextColor(GxEPD_BLACK);
        epd.setTextSize(2);

        epd.setCursor(x, y);
        epd.println(text);

    }
    while (epd.nextPage());
}

void Display::printCentered(const char* text) {
    int16_t x1;
    int16_t y1;
    uint16_t width;
    uint16_t height;

    epd.setTextSize(2);
    epd.getTextBounds(text, 0, 0, &x1, &y1, &width, &height);

    const int x = ((epd.width() - width) / 2) - x1;
    const int y = ((epd.height() - height) / 2) - y1;

    printText(x, y, text);
}

void Display::refresh() {
    // Empty for now.
    // We'll implement partial refresh later.
}
