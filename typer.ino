#include "display.h"
#include "document.h"

const int TEXT_LEFT = 0;
const int TEXT_TOP = 0;
const uint8_t TEXT_SIZE = DISPLAY_TEXT_MEDIUM;

void setup() {
    display.begin();
    const bool documentReady = document.begin();
    const char* text = documentReady
        ? document.getText().c_str()
        : "Could not mount LittleFS. Check the filesystem partition and upload data/document.md.";

    display.printText(
        TEXT_LEFT,
        TEXT_TOP,
        text,
        TEXT_SIZE
    );
}

void loop() {
    // Keyboard editing will go here later.
}
