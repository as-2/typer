#pragma once
#include <Arduino.h>

class Document {
public:
    bool begin();
    const String& getText() const;

    void insertChar(char c);
    void backspace();

    bool save();

private:
    String text;
};

extern Document document;
