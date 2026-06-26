#pragma once

class Display
{
public:
    void begin();
    void clear();
    void printText(int x, int y, const char* text);
    void printCentered(const char* text);
    void refresh();
};

extern Display display;
