#include "document.h"
#include <LittleFS.h>
#include <algorithm>

const char* DOCUMENT_PATH = "/document.md";
const char* DEFAULT_DOCUMENT_TEXT =
    "# ESP32 Typewriter Draft\n\n"
    "This is the default document text.\n"
    "Not actually read from the file.";

Document document;

bool Document::begin() {
    if (!LittleFS.begin()) {
        Serial.println("LittleFS mount failed.");
        currentPath = "";
        text = "";
        return false;
    }

    if (listDocuments().empty()) {
        Serial.println("No document found. Creating default document.");
        currentPath = DOCUMENT_PATH;
        text = DEFAULT_DOCUMENT_TEXT;
        save();
    }

    return true;
}

std::vector<String> Document::listDocuments() const {
    std::vector<String> documents;
    File root = LittleFS.open("/");

    if (!root) {
        return documents;
    }

    File file = root.openNextFile();

    while (file) {
        if (!file.isDirectory()) {
            String path = file.name();

            if (!path.startsWith("/")) {
                path = "/" + path;
            }

            if (path.endsWith(".md")) {
                documents.push_back(path);
            }
        }

        file = root.openNextFile();
    }

    std::sort(documents.begin(), documents.end(), [](const String& left, const String& right) {
        String leftLower = left;
        String rightLower = right;
        leftLower.toLowerCase();
        rightLower.toLowerCase();
        return leftLower < rightLower;
    });

    return documents;
}

bool Document::open(const String& path) {
    File file = LittleFS.open(path, "r");

    if (!file) {
        Serial.print("Failed to open document: ");
        Serial.println(path);
        return false;
    }

    text = file.readString();
    file.close();
    currentPath = path;

    return true;
}

bool Document::createNew() {
    String path = "/Untitled.md";
    uint8_t index = 2;

    while (LittleFS.exists(path)) {
        path = "/Untitled " + String(index) + ".md";
        index++;
    }

    currentPath = path;
    text = "# Untitled\n\n";
    return save();
}

const String& Document::getText() const {
    return text;
}

const String& Document::getPath() const {
    return currentPath;
}

String Document::getTitle() const {
    if (currentPath.length() == 0) {
        return "(no document)";
    }

    String title = currentPath;

    if (title.startsWith("/")) {
        title.remove(0, 1);
    }

    if (title.endsWith(".md")) {
        title.remove(title.length() - 3);
    }

    return title;
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
    if (currentPath.length() == 0) {
        Serial.println("No document selected.");
        return false;
    }

    File file = LittleFS.open(currentPath, "w");

    if (!file) {
        Serial.println("Failed to open document for writing.");
        return false;
    }

    file.print(text);
    file.close();

    Serial.println("Document saved.");
    return true;
}
