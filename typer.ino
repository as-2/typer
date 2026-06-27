#include "display.h"
#include "document.h"
#include "keyboard.h"
#include "config.h"

static bool documentVisible = false;
static bool documentReady = false;

static void showDocument() {
    const char* text = documentReady
        ? document.getText().c_str()
        : "Could not mount LittleFS. Check the filesystem partition and upload data/document.md.";

    display.printText(
        TEXT_LEFT,
        TEXT_TOP,
        text,
        TEXT_SIZE
    );

    documentVisible = true;
}

void setup() {
    display.begin();
    documentReady = document.begin();

    display.printText(
        TEXT_LEFT,
        TEXT_TOP,
        "Connecting to keyboard...\n\nPlease turn on keyboard pairing mode.",
        TEXT_SIZE
    );

    keyboard.begin();
}

void loop() {
    keyboard.update();

    if (keyboard.isConnected() && !documentVisible) {
        showDocument();
    }
}
