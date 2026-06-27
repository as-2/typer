#include "document.h"
#include <LittleFS.h>

const char* DOCUMENT_PATH = "/document.md";
const char* DEFAULT_DOCUMENT_TEXT =
    "# ESP32 Typewriter Draft\n\n"
    "This is the default document text.\n"
    "Not actually read from the file.";

Document document;

bool Document::begin() {
    if (!LittleFS.begin()) {
        Serial.println("LittleFS mount failed.");
        text = "";
        return false;
    }

    File file = LittleFS.open(DOCUMENT_PATH, "r");

    if (!file) {
        Serial.println("No document found. Creating default document.");
        text = DEFAULT_DOCUMENT_TEXT;
        save();
        return true;
    }

    text = file.readString();
    file.close();

    return true;
}

const String& Document::getText() const {
    return text;
}

void Document::insertChar(char c) {
    text += c;
}

void Document::backspace() {
    if (text.length() > 0) {
        text.remove(text.length() - 1);
    }
}

bool Document::save() {
    File file = LittleFS.open(DOCUMENT_PATH, "w");

    if (!file) {
        Serial.println("Failed to open document for writing.");
        return false;
    }

    file.print(text);
    file.close();

    Serial.println("Document saved.");
    return true;
}
