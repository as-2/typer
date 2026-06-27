#include "display.h"
#include "config.h"

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Fonts/FreeSans9pt7b.h>
#include <GxEPD2_BW.h>
#include <vector>

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

uint16_t getTextWidth(const String& text) {
    int16_t x1;
    int16_t y1;
    uint16_t width;
    uint16_t height;

    epd.getTextBounds(text.c_str(), 0, 0, &x1, &y1, &width, &height);
    return width;
}

uint16_t getTextHeight(const String& text) {
    int16_t x1;
    int16_t y1;
    uint16_t width;
    uint16_t height;

    epd.getTextBounds(text.c_str(), 0, 0, &x1, &y1, &width, &height);
    return height;
}

int getTextTopOffset(const String& text) {
    int16_t x1;
    int16_t y1;
    uint16_t width;
    uint16_t height;

    epd.getTextBounds(text.c_str(), 0, 0, &x1, &y1, &width, &height);
    return y1;
}

uint16_t getLineHeight() {
    return getTextHeight("Ag") + 4;
}

void configureTextSize(uint8_t textSize) {
    if (textSize == DISPLAY_TEXT_MEDIUM) {
        epd.setFont(&FreeSans9pt7b);
        epd.setTextSize(1);
    }
    else {
        epd.setFont();
        epd.setTextSize(textSize);
    }
}

std::vector<String> wrapText(int x, const char* text) {
    std::vector<String> lines;
    const int maxWidth = epd.width() - x;

    String line;
    String word;

    auto pushLine = [&]() {
        lines.push_back(line);
        line = "";
    };

    auto addWord = [&]() {
        if (word.length() == 0) {
            return;
        }

        String candidate = line.length() == 0 ? word : line + " " + word;

        if (line.length() == 0 || getTextWidth(candidate) <= maxWidth) {
            line = candidate;
        }
        else {
            pushLine();
            line = word;
        }

        word = "";
    };

    for (const char* cursor = text; *cursor != '\0'; cursor++) {
        const char c = *cursor;

        if (c == '\n') {
            addWord();
            pushLine();
        }
        else if (c == ' ' || c == '\t' || c == '\r') {
            addWord();
        }
        else {
            word += c;
        }
    }

    addWord();

    if (line.length() > 0 || lines.empty()) {
        pushLine();
    }

    return lines;
}

void drawWrappedText(int x, int y, const char* text) {
    const std::vector<String> lines = wrapText(x, text);
    const int lineHeight = getLineHeight();
    const int visibleHeight = epd.height() - y;
    const int totalHeight = lines.size() * lineHeight;
    const int endAnchorY = y + ((visibleHeight * 3) / 5);

    int startY = y;

    if (totalHeight > visibleHeight) {
        startY = endAnchorY - ((lines.size() - 1) * lineHeight);
    }

    for (size_t i = 0; i < lines.size(); i++) {
        const int lineTop = startY + (i * lineHeight);

        if (lineTop + lineHeight < y) {
            continue;
        }

        if (lineTop >= epd.height()) {
            break;
        }

        if (lines[i].length() == 0) {
            continue;
        }

        epd.setCursor(x, lineTop - getTextTopOffset(lines[i]));
        epd.print(lines[i]);
    }
}

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

void Display::printText(int x, int y, const char* text, uint8_t textSize) {
    epd.setFullWindow();

    epd.firstPage();

    do {
        epd.fillScreen(GxEPD_WHITE);

        epd.setTextColor(GxEPD_BLACK);
        configureTextSize(textSize);
        epd.setTextWrap(false);

        drawWrappedText(x, y, text);

    }
    while (epd.nextPage());
}

void Display::printCentered(const char* text) {
    int16_t x1;
    int16_t y1;
    uint16_t width;
    uint16_t height;

    configureTextSize(DISPLAY_TEXT_LARGE);
    epd.getTextBounds(text, 0, 0, &x1, &y1, &width, &height);

    const int x = ((epd.width() - width) / 2) - x1;
    const int y = ((epd.height() - height) / 2) - y1;

    printText(x, y, text);
}

void Display::refresh() {
    // Empty for now.
    // We'll implement partial refresh later.
}
