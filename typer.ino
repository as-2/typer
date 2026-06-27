#include "display.h"
#include "document.h"
#include "keyboard_test.h"
#include "config.h"

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

    keyboardTest.begin();
}

void loop() {
    keyboardTest.update();
}
