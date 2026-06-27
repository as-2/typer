#include "display.h"

#include <LittleFS.h>
// #include <LITTLEFS.h> 

const int TEXT_LEFT = 0;
const int TEXT_TOP = 0;
const uint8_t TEXT_SIZE = DISPLAY_TEXT_MEDIUM;
const char* DOCUMENT_PATH = "/document.md";

String readDocument() {
    if (!LittleFS.begin()) {
        Serial.println("Failed to mount LittleFS.");
        return "Could not mount LittleFS. Upload the data folder to the ESP32.";
    }

    File document = LittleFS.open(DOCUMENT_PATH, "r");

    if (!document) {
        Serial.println("Failed to open /document.md.");
        return "Could not open /document.md. Make sure data/document.md was uploaded.";
    }

    String text = document.readString();
    document.close();

    return text;
}

void setup() {
    display.begin();

    String documentText = readDocument();

    display.printText(
        TEXT_LEFT,
        TEXT_TOP,
        documentText.c_str(),
        TEXT_SIZE
    );
}

void loop() {
}
