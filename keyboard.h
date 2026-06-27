#pragma once

#include <Arduino.h>

class Keyboard
{
public:
    bool begin();
    void update();
};

extern Keyboard keyboard;
