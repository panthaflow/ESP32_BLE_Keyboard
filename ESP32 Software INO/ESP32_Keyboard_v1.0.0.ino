#include <Arduino.h>
#include "Adafruit_TinyUSB.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define FIRMWARE_VERSION "1.0.0"

uint8_t const desc_hid_report[] = {
  TUD_HID_REPORT_DESC_KEYBOARD()
};

Adafruit_USBD_HID usb_hid;

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

bool deviceConnected = false;
bool biosMode = false;

// --- Puffer für Befehle und Text ---
String textBuffer = "";
bool executeCad = false;
bool executeEnter = false;

BLECharacteristic *pCharacteristic = NULL;

const uint8_t NUMPAD_KEYS[10] = {
  HID_KEY_KEYPAD_0, HID_KEY_KEYPAD_1, HID_KEY_KEYPAD_2, HID_KEY_KEYPAD_3, 
  HID_KEY_KEYPAD_4, HID_KEY_KEYPAD_5, HID_KEY_KEYPAD_6, HID_KEY_KEYPAD_7, 
  HID_KEY_KEYPAD_8, HID_KEY_KEYPAD_9
};

void getHidCode(char c, uint8_t &modifier, uint8_t &keycode) {
    modifier = 0;
    keycode = 0;

    // Buchstaben
    if (c >= 'a' && c <= 'z') { keycode = HID_KEY_A + (c - 'a'); } 
    else if (c >= 'A' && c <= 'Z') { modifier = KEYBOARD_MODIFIER_LEFTSHIFT; keycode = HID_KEY_A + (c - 'A'); } 
    
    // Zahlen (Hauptfeld)
    else if (c >= '1' && c <= '9') { keycode = HID_KEY_1 + (c - '1'); } 
    else if (c == '0') { keycode = HID_KEY_0; } 
    
    // Steuerung & Leerzeichen
    else if (c == ' ') { keycode = HID_KEY_SPACE; } 
    else if (c == '\n' || c == '\r') { keycode = HID_KEY_ENTER; }
    else if (c == '\t') { keycode = HID_KEY_TAB; }
    else if (c == '\b') { keycode = HID_KEY_BACKSPACE; }

    // --- SONDERZEICHEN (Basierend auf US-QWERTY Layout fürs BIOS) ---
    else if (c == '-') { keycode = HID_KEY_MINUS; }
    else if (c == '_') { modifier = KEYBOARD_MODIFIER_LEFTSHIFT; keycode = HID_KEY_MINUS; }
    else if (c == '=') { keycode = HID_KEY_EQUAL; }
    else if (c == '+') { modifier = KEYBOARD_MODIFIER_LEFTSHIFT; keycode = HID_KEY_EQUAL; }
    
    else if (c == '.') { keycode = HID_KEY_PERIOD; }
    else if (c == '>') { modifier = KEYBOARD_MODIFIER_LEFTSHIFT; keycode = HID_KEY_PERIOD; }
    else if (c == ',') { keycode = HID_KEY_COMMA; }
    else if (c == '<') { modifier = KEYBOARD_MODIFIER_LEFTSHIFT; keycode = HID_KEY_COMMA; }
    
    else if (c == '/') { keycode = HID_KEY_SLASH; }
    else if (c == '?') { modifier = KEYBOARD_MODIFIER_LEFTSHIFT; keycode = HID_KEY_SLASH; }
    else if (c == '\\') { keycode = HID_KEY_BACKSLASH; }
    else if (c == '|') { modifier = KEYBOARD_MODIFIER_LEFTSHIFT; keycode = HID_KEY_BACKSLASH; }
    
    else if (c == ';') { keycode = HID_KEY_SEMICOLON; }
    else if (c == ':') { modifier = KEYBOARD_MODIFIER_LEFTSHIFT; keycode = HID_KEY_SEMICOLON; }
    else if (c == '\'') { keycode = HID_KEY_APOSTROPHE; }
    else if (c == '"') { modifier = KEYBOARD_MODIFIER_LEFTSHIFT; keycode = HID_KEY_APOSTROPHE; }
    
    else if (c == '[') { keycode = HID_KEY_BRACKET_LEFT; }
    else if (c == '{') { modifier = KEYBOARD_MODIFIER_LEFTSHIFT; keycode = HID_KEY_BRACKET_LEFT; }
    else if (c == ']') { keycode = HID_KEY_BRACKET_RIGHT; }
    else if (c == '}') { modifier = KEYBOARD_MODIFIER_LEFTSHIFT; keycode = HID_KEY_BRACKET_RIGHT; }
    else if (c == '`') { keycode = HID_KEY_GRAVE; }
    else if (c == '~') { modifier = KEYBOARD_MODIFIER_LEFTSHIFT; keycode = HID_KEY_GRAVE; }

    // Zahlen-Sonderzeichen (US-Shift Kombinationen)
    else if (c == '!') { modifier = KEYBOARD_MODIFIER_LEFTSHIFT; keycode = HID_KEY_1; }
    else if (c == '@') { modifier = KEYBOARD_MODIFIER_LEFTSHIFT; keycode = HID_KEY_2; }
    else if (c == '#') { modifier = KEYBOARD_MODIFIER_LEFTSHIFT; keycode = HID_KEY_3; }
    else if (c == '$') { modifier = KEYBOARD_MODIFIER_LEFTSHIFT; keycode = HID_KEY_4; }
    else if (c == '%') { modifier = KEYBOARD_MODIFIER_LEFTSHIFT; keycode = HID_KEY_5; }
    else if (c == '^') { modifier = KEYBOARD_MODIFIER_LEFTSHIFT; keycode = HID_KEY_6; }
    else if (c == '&') { modifier = KEYBOARD_MODIFIER_LEFTSHIFT; keycode = HID_KEY_7; }
    else if (c == '*') { modifier = KEYBOARD_MODIFIER_LEFTSHIFT; keycode = HID_KEY_8; }
    else if (c == '(') { modifier = KEYBOARD_MODIFIER_LEFTSHIFT; keycode = HID_KEY_9; }
    else if (c == ')') { modifier = KEYBOARD_MODIFIER_LEFTSHIFT; keycode = HID_KEY_0; }
}

void typeViaAltCode(char c) {
    // Wandle das Zeichen in einen exakt 4-stelligen ISO-String um (z. B. 'a' -> "0097")
    uint8_t asciiVal = (uint8_t)c;
    char code[5];
    snprintf(code, sizeof(code), "%04d", asciiVal);

    // 1. ALT drücken (ohne extra Numpad-Taste)
    usb_hid.keyboardReport(0, KEYBOARD_MODIFIER_LEFTALT, NULL);
    delay(5); 

    // 2. Die 4 Ziffern auf dem Numpad blitzschnell tippen
    for (int i = 0; i < 4; i++) {
        int digit = code[i] - '0';
        uint8_t keys[6] = { NUMPAD_KEYS[digit], 0, 0, 0, 0, 0 };
        
        // Ziffer drücken
        usb_hid.keyboardReport(0, KEYBOARD_MODIFIER_LEFTALT, keys);
        delay(4); 
        
        // Ziffer loslassen (ALT bleibt gedrückt)
        usb_hid.keyboardReport(0, KEYBOARD_MODIFIER_LEFTALT, NULL);
        delay(3); 
    }

    // 3. ALT loslassen -> Erst JETZT löst Windows den Alt-Code aus
    usb_hid.keyboardRelease(0);
    delay(5); 
}

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pChar) {
        String rxValue = pChar->getValue().c_str();
        
        if (rxValue.length() > 0) {
            Serial.print("Empfangen: ");
            Serial.println(rxValue);

            // Commandos sofort auswerten
            if (rxValue == "CMD:BIOS") {
                biosMode = true;
                Serial.println("-> BIOS Modus");
            } 
            else if (rxValue == "CMD:WIN") {
                biosMode = false;
                Serial.println("-> WIN Modus");
            }
            else if (rxValue == "CMD:CAD") {
                executeCad = true;
            } 
            else if (rxValue == "CMD:ENTER") {
                executeEnter = true;
            }
            // NEU: Versionsabfrage der Webapp beantworten
            else if (rxValue == "CMD:GET_VER") {
                String verResp = "VER:" + String(FIRMWARE_VERSION);
                pChar->setValue(verResp.c_str());
                pChar->notify();
                Serial.println("-> Version gesendet: " + String(FIRMWARE_VERSION));
            }
            // Normaler Text wird nur in den Puffer geschoben
            else {
                textBuffer += String(rxValue);
            }
        }
    }
};

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) { deviceConnected = true; };
    void onDisconnect(BLEServer* pServer) { 
        deviceConnected = false; 
        pServer->startAdvertising(); 
    }
};

void setup() {
    Serial.begin(115200);
    
    usb_hid.setBootProtocol(HID_ITF_PROTOCOL_KEYBOARD);
    usb_hid.setPollInterval(2);
    usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));
    usb_hid.setStringDescriptor("BLE BIOS Keyboard");
    usb_hid.begin();

    BLEDevice::init("ESP32-S3 Keyboard");
    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    BLEService *pService = pServer->createService(SERVICE_UUID);
    pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_WRITE |
                                         BLECharacteristic::PROPERTY_NOTIFY
                                       );

    pCharacteristic->addDescriptor(new BLE2902());
    pCharacteristic->setCallbacks(new MyCallbacks());
    pService->start();

    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    BLEDevice::startAdvertising();
}

void loop() {
    // Nur etwas tun, wenn USB bereit ist
    if (!usb_hid.ready()) {
        delay(10);
        return;
    }

    // Strg+Alt+Entf abarbeiten
    if (executeCad) {
        executeCad = false;
        uint8_t keys[6] = { HID_KEY_DELETE, 0, 0, 0, 0, 0 };
        usb_hid.keyboardReport(0, KEYBOARD_MODIFIER_LEFTCTRL | KEYBOARD_MODIFIER_LEFTALT, keys);
        delay(100); 
        usb_hid.keyboardRelease(0); 
    }

    // Enter abarbeiten
    if (executeEnter) {
        executeEnter = false;
        uint8_t keys[6] = { HID_KEY_ENTER, 0, 0, 0, 0, 0 };
        usb_hid.keyboardReport(0, 0, keys);
        delay(50);
        usb_hid.keyboardRelease(0);
    }

    // Text Zeichen für Zeichen aus dem Puffer abarbeiten
    if (textBuffer.length() > 0) {
        char c = textBuffer[0];
        textBuffer.remove(0, 1); // Das gelesene Zeichen aus dem Puffer löschen

        if (biosMode) {
            uint8_t modifier, key;
            getHidCode(c, modifier, key);
            
            if (key != 0) {
                uint8_t keys[6] = { key, 0, 0, 0, 0, 0 };
                usb_hid.keyboardReport(0, modifier, keys);
                delay(60); 
                usb_hid.keyboardRelease(0);
                delay(60); 
            }
        } else {
            if (c == ' ') {
                uint8_t keys[6] = { HID_KEY_SPACE, 0, 0, 0, 0, 0 };
                usb_hid.keyboardReport(0, 0, keys);
                delay(25);
                usb_hid.keyboardRelease(0); 
            } else {
                typeViaAltCode(c);
            }
        }
    }

    // Kurze Pause, damit der ESP32-Watchdog nicht anschlägt
    delay(10);
}