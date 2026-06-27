#include "display.h"
#include "document.h"
#include "keyboard.h"
#include "config.h"

#include <vector>

enum AppMode {
    WAITING_FOR_KEYBOARD,
    DOCUMENT_PICKER,
    DOCUMENT_EDITOR
};

const uint8_t KEY_ENTER = 0x28;
const uint8_t KEY_ESCAPE = 0x29;
const uint8_t KEY_BACKSPACE = 0x2A;
const uint8_t KEY_DOWN = 0x51;
const uint8_t KEY_UP = 0x52;
const uint8_t KEY_N = 0x11;

static AppMode appMode = WAITING_FOR_KEYBOARD;
static bool documentReady = false;
static bool documentChanged = false;
static unsigned long lastDocumentChangeAt = 0;
static std::vector<String> documentPaths;
static size_t selectedDocumentIndex = 0;

static String titleFromPath(String path) {
    if (path.startsWith("/")) {
        path.remove(0, 1);
    }

    if (path.endsWith(".md")) {
        path.remove(path.length() - 3);
    }

    return path;
}

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

    appMode = DOCUMENT_EDITOR;
}

static void showDocumentPicker(bool partialRefresh = false) {
    if (!documentReady) {
        display.printText(
            TEXT_LEFT,
            TEXT_TOP,
            "Could not mount LittleFS.\n\nCheck the filesystem partition.",
            TEXT_SIZE
        );
        return;
    }

    documentPaths = document.listDocuments();

    if (selectedDocumentIndex >= documentPaths.size()) {
        selectedDocumentIndex = 0;
    }

    String pickerText = "Documents\n\n";

    if (documentPaths.empty()) {
        pickerText += "No documents yet.\n";
    }
    else {
        for (size_t i = 0; i < documentPaths.size(); i++) {
            pickerText += i == selectedDocumentIndex ? "> " : "  ";
            pickerText += titleFromPath(documentPaths[i]);
            pickerText += "\n";
        }
    }

    pickerText += "\nEnter opens. Ctrl+N creates.";

    if (partialRefresh) {
        display.printTextPartial(
            TEXT_LEFT,
            TEXT_TOP,
            pickerText.c_str(),
            TEXT_SIZE
        );
    }
    else {
        display.printText(
            TEXT_LEFT,
            TEXT_TOP,
            pickerText.c_str(),
            TEXT_SIZE
        );
    }

    appMode = DOCUMENT_PICKER;
}

static void createNewDocument() {
    if (!documentReady || !document.createNew()) {
        display.printText(
            TEXT_LEFT,
            TEXT_TOP,
            "Could not create a new document.",
            TEXT_SIZE
        );
        return;
    }

    documentChanged = false;
    showDocument();
}

static void openSelectedDocument() {
    if (documentPaths.empty()) {
        return;
    }

    if (!document.open(documentPaths[selectedDocumentIndex])) {
        display.printText(
            TEXT_LEFT,
            TEXT_TOP,
            "Could not open document.",
            TEXT_SIZE
        );
        return;
    }

    documentChanged = false;
    showDocument();
}

static void returnToDocumentPicker() {
    if (documentChanged) {
        document.save();
        documentChanged = false;
    }

    showDocumentPicker();
}

static void handlePickerEvent(const KeyboardEvent& event) {
    if (event.ctrl && event.keycode == KEY_N) {
        createNewDocument();
        return;
    }

    if (event.keycode == KEY_ENTER) {
        openSelectedDocument();
        return;
    }

    if (documentPaths.empty()) {
        return;
    }

    if (event.keycode == KEY_UP) {
        selectedDocumentIndex = selectedDocumentIndex == 0
            ? documentPaths.size() - 1
            : selectedDocumentIndex - 1;
        showDocumentPicker(true);
    }
    else if (event.keycode == KEY_DOWN) {
        selectedDocumentIndex = (selectedDocumentIndex + 1) % documentPaths.size();
        showDocumentPicker(true);
    }
}

static bool handleEditorEvent(const KeyboardEvent& event) {
    if (event.keycode == KEY_ESCAPE) {
        returnToDocumentPicker();
        return false;
    }

    if (event.ctrl && event.keycode == KEY_N) {
        createNewDocument();
        return false;
    }

    if (event.ctrl) {
        return false;
    }

    if (event.keycode == KEY_BACKSPACE) {
        document.backspace();
    }
    else if (event.character != '\0') {
        document.insertChar(event.character);
    }
    else {
        return false;
    }

    documentChanged = true;
    lastDocumentChangeAt = millis();
    return true;
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

    if (keyboard.isConnected() && appMode == WAITING_FOR_KEYBOARD) {
        showDocumentPicker();
    }

    KeyboardEvent event;
    bool shouldRefreshDocument = false;

    while (keyboard.readEvent(event)) {
        if (appMode == DOCUMENT_PICKER) {
            handlePickerEvent(event);
        }
        else if (appMode == DOCUMENT_EDITOR) {
            shouldRefreshDocument = handleEditorEvent(event) || shouldRefreshDocument;
        }
    }

    if (shouldRefreshDocument && appMode == DOCUMENT_EDITOR) {
        display.printTextPartial(TEXT_LEFT, TEXT_TOP, document.getText().c_str(), TEXT_SIZE);
    }

    if (documentChanged && millis() - lastDocumentChangeAt >= DOCUMENT_SAVE_IDLE_MS) {
        document.save();
        documentChanged = false;
    }
}
