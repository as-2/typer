#pragma once

#include <Arduino.h>

class KeyboardTest
{
public:
    bool begin();
    void update();
};

extern KeyboardTest keyboardTest;
