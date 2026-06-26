#include "display.h"

const int TEXT_LEFT = 0;
const int TEXT_TOP = 0;
const uint8_t TEXT_SIZE = DISPLAY_TEXT_MEDIUM;

const char* SAMPLE_TEXT =
    "This is a longer test passage for the ESP32-S3 typewriter. "
    "It should begin near the top-left corner of the e-ink display, "
    "using a smaller font so more words fit on each page. "
    "Try changing TEXT_LEFT, TEXT_TOP, and TEXT_SIZE to tune the layout.";

void setup() {
    display.begin();
    display.printText(
        TEXT_LEFT,
        TEXT_TOP,
        SAMPLE_TEXT,
        TEXT_SIZE
    );
}

void loop() {
}
