#pragma once

#include <Arduino.h>

struct KeyboardEvent {
    char character;
    uint8_t keycode;
    bool ctrl;
    bool shift;
};

class Keyboard
{
public:
    bool begin();
    void update();
    bool isConnected() const;
    bool readEvent(KeyboardEvent& event);
};

extern Keyboard keyboard;
