#pragma once

#include <Arduino.h>

enum DisplayTextSize : uint8_t
{
    DISPLAY_TEXT_SMALL = 1,
    DISPLAY_TEXT_MEDIUM = 100,
    DISPLAY_TEXT_LARGE = 2
};

class Display
{
public:
    void begin();
    void clear();
    void printText(int x, int y, const char* text, uint8_t textSize = DISPLAY_TEXT_LARGE);
    void printTextPartial(int x, int y, const char* text, uint8_t textSize = DISPLAY_TEXT_LARGE);
    void printCentered(const char* text);
    void refresh();
};

extern Display display;
