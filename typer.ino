#include "display.h"

void setup() {
    display.begin();
    display.printText(
        20,
        60,
        "Hello World!"
    );
}

void loop() {
}
