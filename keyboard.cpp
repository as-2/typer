#include "keyboard.h"
#include "config.h"
#include "display.h"
#include "document.h"

#include <BLEAdvertisedDevice.h>
#include <BLEClient.h>
#include <BLEDevice.h>
#include <BLESecurity.h>
#include <BLEUtils.h>

Keyboard keyboard;

static BLEUUID hidServiceUUID((uint16_t)0x1812);
static BLEUUID reportCharUUID((uint16_t)0x2A4D);

static BLEAdvertisedDevice* keyboardDevice = nullptr;
static BLEClient* bleClient = nullptr;
static bool shouldConnect = false;
static bool shouldScan = false;
static bool connected = false;
static bool scanStarted = false;
static uint8_t previousReport[7] = {};
static String pendingKeys;
static bool documentChanged = false;
static unsigned long lastDocumentChangeAt = 0;

static bool isShiftPressed(uint8_t modifier) {
    return (modifier & 0x22) != 0;
}

static bool keyWasPressed(uint8_t keycode) {
    for (uint8_t i = 1; i < sizeof(previousReport); i++) {
        if (previousReport[i] == keycode) {
            return true;
        }
    }

    return false;
}

static char keycodeToAscii(uint8_t keycode, bool shifted) {
    if (keycode >= 0x04 && keycode <= 0x1D) {
        const char base = shifted ? 'A' : 'a';
        return base + (keycode - 0x04);
    }

    if (keycode >= 0x1E && keycode <= 0x27) {
        const char normal[] = "1234567890";
        const char shiftedChars[] = "!@#$%^&*()";
        const uint8_t index = keycode == 0x27 ? 9 : keycode - 0x1E;
        return shifted ? shiftedChars[index] : normal[index];
    }

    switch (keycode) {
        case 0x28: return '\n';
        case 0x2B: return '\t';
        case 0x2C: return ' ';
        case 0x2D: return shifted ? '_' : '-';
        case 0x2E: return shifted ? '+' : '=';
        case 0x2F: return shifted ? '{' : '[';
        case 0x30: return shifted ? '}' : ']';
        case 0x31: return shifted ? '|' : '\\';
        case 0x33: return shifted ? ':' : ';';
        case 0x34: return shifted ? '"' : '\'';
        case 0x35: return shifted ? '~' : '`';
        case 0x36: return shifted ? '<' : ',';
        case 0x37: return shifted ? '>' : '.';
        case 0x38: return shifted ? '?' : '/';
        default: return '\0';
    }
}

static void queueKeyboardReport(const uint8_t* data, size_t length) {
    if (length < sizeof(previousReport)) {
        return;
    }

    const bool shifted = isShiftPressed(data[0]);

    for (uint8_t i = 1; i < sizeof(previousReport); i++) {
        const uint8_t keycode = data[i];

        if (keycode == 0 || keyWasPressed(keycode)) {
            continue;
        }

        if (keycode == 0x2A) {
            pendingKeys += '\b';
            continue;
        }

        const char ascii = keycodeToAscii(keycode, shifted);

        if (ascii != '\0') {
            pendingKeys += ascii;
        }
        else {
            Serial.print("Unhandled HID keycode: ");
            Serial.println(keycode);
        }
    }

    memcpy(previousReport, data, sizeof(previousReport));
}

static void notifyCallback(BLERemoteCharacteristic* characteristic, uint8_t* data, size_t length, bool isNotify) {
    (void)characteristic;
    (void)isNotify;

    queueKeyboardReport(data, length);
}

class KeyboardClientCallbacks : public BLEClientCallbacks {
    void onConnect(BLEClient* client) override {
        (void)client;
        Serial.println("Connected to BLE keyboard.");
    }

    void onDisconnect(BLEClient* client) override {
        (void)client;
        connected = false;
        Serial.println("Disconnected from BLE keyboard.");
    }
};

static bool subscribeToKeyboardReports(BLERemoteService* hidService) {
    std::map<std::string, BLERemoteCharacteristic*>* characteristics = hidService->getCharacteristics();
    uint8_t subscribedCount = 0;

    for (auto const& entry : *characteristics) {
        BLERemoteCharacteristic* characteristic = entry.second;

        if (!characteristic->getUUID().equals(reportCharUUID) || !characteristic->canNotify()) {
            continue;
        }

        Serial.print("Subscribing to report characteristic handle 0x");
        Serial.println(characteristic->getHandle(), HEX);
        characteristic->registerForNotify(notifyCallback);
        subscribedCount++;
    }

    if (subscribedCount == 0) {
        Serial.println("No notify-capable HID report characteristics found.");
        return false;
    }

    connected = true;
    Serial.print("Subscribed to ");
    Serial.print(subscribedCount);
    Serial.println(" HID input report characteristic(s). Press keys now.");
    return true;
}

static bool connectToKeyboard() {
    if (keyboardDevice == nullptr) {
        return false;
    }

    Serial.print("Connecting to ");
    Serial.print(keyboardDevice->getName().c_str());
    Serial.print(" at ");
    Serial.println(keyboardDevice->getAddress().toString().c_str());

    bleClient = BLEDevice::createClient();
    bleClient->setClientCallbacks(new KeyboardClientCallbacks());

    if (!bleClient->connect(keyboardDevice)) {
        Serial.println("BLE keyboard connect failed.");
        return false;
    }

    bleClient->setMTU(185);

    BLERemoteService* hidService = bleClient->getService(hidServiceUUID);

    if (hidService == nullptr) {
        Serial.println("Connected, but HID service was not found.");
        bleClient->disconnect();
        return false;
    }

    Serial.println("Found HID service.");

    if (!subscribeToKeyboardReports(hidService)) {
        bleClient->disconnect();
        return false;
    }

    return true;
}

class BleKeyboardScanCallbacks : public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice device) override {
        const bool advertisesHid = device.haveServiceUUID() && device.isAdvertisingService(hidServiceUUID);

        if (!advertisesHid) {
            return;
        }

        Serial.print("BLE HID keyboard candidate: ");

        if (device.haveName()) {
            Serial.print(device.getName().c_str());
        }
        else {
            Serial.print("(no name)");
        }

        Serial.print("  address=");
        Serial.println(device.getAddress().toString().c_str());

        BLEDevice::getScan()->stop();
        keyboardDevice = new BLEAdvertisedDevice(device);
        shouldConnect = true;
    }
};

static void startKeyboardScan() {
    Serial.println("Scanning for BLE keyboard. Put keyboard into pairing mode.");

    BLEScan* scan = BLEDevice::getScan();
    scan->setAdvertisedDeviceCallbacks(new BleKeyboardScanCallbacks(), true);
    scan->setActiveScan(true);
    scan->setInterval(100);
    scan->setWindow(99);
    scan->start(KEYBOARD_SCAN_SECONDS, false);
    scanStarted = true;
}

bool Keyboard::begin() {
    Serial.println("Starting BLE keyboard input...");

    BLEDevice::init("ESP32-Typewriter");

    BLESecurity* security = new BLESecurity();
    security->setCapability(ESP_IO_CAP_NONE);
    security->setAuthenticationMode(true, false, true);
    BLEDevice::setSecurityCallbacks(new BLESecurityCallbacks());

    shouldScan = true;

    return true;
}

void Keyboard::update() {
    if (shouldScan) {
        shouldScan = false;
        startKeyboardScan();
    }

    if (shouldConnect) {
        shouldConnect = false;

        if (!connectToKeyboard()) {
            Serial.println("Connection failed. Restart the board to scan again.");
        }
    }

    if (!connected && scanStarted && !shouldConnect) {
        scanStarted = false;
        Serial.println("BLE scan finished. Restart the board to scan again.");
        display.printText(
            TEXT_LEFT,
            TEXT_TOP,
            "Keyboard not found.\n\nRestart the board to scan again.",
            TEXT_SIZE
        );
    }

    if (documentChanged && millis() - lastDocumentChangeAt >= DOCUMENT_SAVE_IDLE_MS) {
        document.save();
        documentChanged = false;
    }

    if (pendingKeys.length() == 0) {
        return;
    }

    String keys = pendingKeys;
    pendingKeys = "";

    for (size_t i = 0; i < keys.length(); i++) {
        const char key = keys[i];

        if (key == '\b') {
            document.backspace();
        }
        else {
            document.insertChar(key);
        }
    }

    documentChanged = true;
    lastDocumentChangeAt = millis();
    display.printTextPartial(TEXT_LEFT, TEXT_TOP, document.getText().c_str(), TEXT_SIZE);
}

bool Keyboard::isConnected() const {
    return connected;
}
