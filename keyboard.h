#pragma once

#include <Arduino.h>

class Keyboard
{
public:
    bool begin();
    void update();
    bool isConnected() const;
};

extern Keyboard keyboard;
