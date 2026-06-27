#include "display.h"
#include "document.h"

#include <LittleFS.h>

const int TEXT_LEFT = 0;
const int TEXT_TOP = 0;
const uint8_t TEXT_SIZE = DISPLAY_TEXT_MEDIUM;

void setup() {
    display.begin();
    document.begin();

    // String documentText = readDocument();

    display.printText(
        TEXT_LEFT,
        TEXT_TOP,
        document.getText().c_str(),
        TEXT_SIZE
    );
}

void loop() {
}
