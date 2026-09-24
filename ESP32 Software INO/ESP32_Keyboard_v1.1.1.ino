#include <Arduino.h>
#include "Adafruit_TinyUSB.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// ==========================================
// --- DISPLAY KONFIGURATION (Waveshare GEEK)
// ==========================================
#define ENABLE_DISPLAY 1
#define DISPLAY_ROTATION 2

#if ENABLE_DISPLAY
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

#define TFT_CS    10
#define TFT_DC    8
#define TFT_RST   9
#define TFT_BL    7
#define TFT_MOSI  11
#define TFT_SCLK  12

Adafruit_ST7789 display = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
enum UI_STATE { UI_INIT, UI_BLE_WAIT, UI_CONNECTED, UI_TYPING, UI_ACTION, UI_NO_USB, UI_NUMLOCK_OFF };
#endif

#define FIRMWARE_VERSION "1.1.1"

uint8_t const desc_hid_report[] = {
  TUD_HID_REPORT_DESC_KEYBOARD()
};

Adafruit_USBD_HID usb_hid;

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

bool deviceConnected = false;
bool biosMode = false;
bool lastUsbReady = false; 

// Flags für Num-Lock Ausführung & Live-Überwachung
volatile bool numLockAutoChecked = false;
volatile bool triggerNumLock = false;
volatile bool isNumLockActive = true; 
volatile bool numLockStatusChanged = false;

String textBuffer = "";
bool executeCad = false;
bool executeEnter = false;

BLECharacteristic *pCharacteristic = NULL;

const uint8_t NUMPAD_KEYS[10] = {
  HID_KEY_KEYPAD_0, HID_KEY_KEYPAD_1, HID_KEY_KEYPAD_2, HID_KEY_KEYPAD_3, 
  HID_KEY_KEYPAD_4, HID_KEY_KEYPAD_5, HID_KEY_KEYPAD_6, HID_KEY_KEYPAD_7, 
  HID_KEY_KEYPAD_8, HID_KEY_KEYPAD_9
};

// ===================================================================
// --- NUM LOCK CHECK (Callback bei jeder LED-Änderung vom PC)
// ===================================================================
void hid_report_callback(uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {
  if (report_type == HID_REPORT_TYPE_OUTPUT && bufsize > 0) {
    uint8_t leds = buffer[0];
    bool currentNumLock = (leds & KEYBOARD_LED_NUMLOCK) != 0;

    // Statusänderung für das Display erkennen
    if (currentNumLock != isNumLockActive || !numLockAutoChecked) {
      isNumLockActive = currentNumLock;
      numLockStatusChanged = true; // Signalisiert dem Loop, das Display zu aktualisieren
    }

    // Erster Check nach dem Booten -> Automatisches Anschalten antriggern
    if (!numLockAutoChecked) {
      numLockAutoChecked = true;
      if (!currentNumLock) {
        triggerNumLock = true; 
      }
    }
  }
}

#if ENABLE_DISPLAY
void printCentered(String text, int y, int requestedTextSize, uint16_t color) {
    bool isPortrait = (DISPLAY_ROTATION == 0 || DISPLAY_ROTATION == 2);
    int screenWidth = isPortrait ? 135 : 240;
    
    int numLines = 1;
    for (int i = 0; i < text.length(); i++) {
        if (text[i] == '\n') numLines++;
    }
    
    String lines[10]; 
    int lineIdx = 0;
    int lastIdx = 0;
    for (int i = 0; i < text.length(); i++) {
        if (text[i] == '\n') {
            lines[lineIdx++] = text.substring(lastIdx, i);
            lastIdx = i + 1;
        }
    }
    lines[lineIdx] = text.substring(lastIdx);
    
    int maxLen = 0;
    for (int i = 0; i < numLines; i++) {
        if (lines[i].length() > maxLen) maxLen = lines[i].length();
    }

    int textSize = requestedTextSize;
    int textWidth = maxLen * (6 * textSize);
    
    while (textWidth > screenWidth && textSize > 1) {
        textSize--;
        textWidth = maxLen * (6 * textSize);
    }
    
    display.setTextSize(textSize);
    display.setTextColor(color);
    
    for (int i = 0; i < numLines; i++) {
        int w = lines[i].length() * (6 * textSize);
        int x = (screenWidth - w) / 2;
        if (x < 0) x = 0;
        int currentY = y + (i * ((8 * textSize) + 4)); 
        display.setCursor(x, currentY);
        display.print(lines[i]);
    }
}

void drawThickLine(int x0, int y0, int x1, int y1, uint16_t color) {
    display.drawLine(x0, y0, x1, y1, color);
    display.drawLine(x0+1, y0, x1+1, y1, color);
    display.drawLine(x0-1, y0, x1-1, y1, color);
    display.drawLine(x0, y0+1, x1, y1+1, color);
    display.drawLine(x0, y0-1, x1, y1-1, color);
}

void updateDisplay(UI_STATE state, String message = "") {
    display.fillScreen(ST77XX_BLACK);
    bool isPortrait = (DISPLAY_ROTATION == 0 || DISPLAY_ROTATION == 2);
    int cx = isPortrait ? 67 : 120;     
    int cy = isPortrait ? 90 : 50;      
    int textY = isPortrait ? 135 : 90;  

    switch(state) {
        case UI_INIT:
            display.drawCircle(cx, cy, 15, ST77XX_WHITE);
            display.drawCircle(cx, cy, 14, ST77XX_WHITE);
            printCentered(message.length() > 0 ? message : "Booting...", textY, 2, ST77XX_WHITE);
            break;
        case UI_NO_USB:
            display.fillTriangle(cx, cy-18, cx-18, cy+14, cx+18, cy+14, ST77XX_RED);
            display.fillRect(cx-2, cy-5, 4, 10, ST77XX_BLACK);
            display.fillRect(cx-2, cy+7, 4, 4, ST77XX_BLACK);
            printCentered(message.length() > 0 ? message : "NO USB\nWaiting...", textY, 2, ST77XX_WHITE);
            break;
        case UI_BLE_WAIT:
            drawThickLine(cx, cy-18, cx, cy+18, 0x03FF); 
            drawThickLine(cx, cy-18, cx+12, cy-6, 0x03FF);
            drawThickLine(cx+12, cy-6, cx-10, cy+10, 0x03FF);
            drawThickLine(cx-10, cy-10, cx+12, cy+6, 0x03FF);
            drawThickLine(cx+12, cy+6, cx, cy+18, 0x03FF);
            printCentered("Waiting\nfor BLE\nconnection", textY, 2, ST77XX_WHITE);
            break;
        case UI_CONNECTED:
            drawThickLine(cx-12, cy, cx-4, cy+12, ST77XX_GREEN);
            drawThickLine(cx-4, cy+12, cx+16, cy-12, ST77XX_GREEN);
            printCentered("Connected!", textY, 2, ST77XX_GREEN);
            break;
        case UI_TYPING:
            display.drawRoundRect(cx-26, cy-16, 52, 32, 6, ST77XX_YELLOW);
            display.drawRoundRect(cx-25, cy-15, 50, 30, 5, ST77XX_YELLOW);
            display.fillRect(cx-16, cy-8, 6, 6, ST77XX_YELLOW);
            display.fillRect(cx-3, cy-8, 6, 6, ST77XX_YELLOW);
            display.fillRect(cx+10, cy-8, 6, 6, ST77XX_YELLOW);
            display.fillRect(cx-12, cy+4, 24, 6, ST77XX_YELLOW);
            printCentered(message.length() > 0 ? message : "Typing...", textY, 2, ST77XX_WHITE);
            break;
        case UI_ACTION:
            display.fillTriangle(cx, cy-18, cx-18, cy+14, cx+18, cy+14, ST77XX_RED);
            display.fillRect(cx-2, cy-5, 4, 10, ST77XX_BLACK);
            display.fillRect(cx-2, cy+7, 4, 4, ST77XX_BLACK);
            printCentered(message, textY, 2, ST77XX_WHITE);
            break;
        case UI_NUMLOCK_OFF:
            // Rotes Warn-Dreieck mit "NUM LOCK OFF" Hinweis
            display.fillTriangle(cx, cy-18, cx-18, cy+14, cx+18, cy+14, ST77XX_RED);
            display.fillRect(cx-2, cy-5, 4, 10, ST77XX_BLACK);
            display.fillRect(cx-2, cy+7, 4, 4, ST77XX_BLACK);
            printCentered("NUM LOCK\nOFF", textY, 2, ST77XX_RED);
            break;
    }
}
#else
enum UI_STATE { UI_INIT, UI_BLE_WAIT, UI_CONNECTED, UI_TYPING, UI_ACTION, UI_NO_USB, UI_NUMLOCK_OFF };
void updateDisplay(UI_STATE state, String message = "") {}
#endif

void getHidCode(char c, uint8_t &modifier, uint8_t &keycode) {
    modifier = 0; keycode = 0;
    if (c >= 'a' && c <= 'z') { keycode = HID_KEY_A + (c - 'a'); } 
    else if (c >= 'A' && c <= 'Z') { modifier = KEYBOARD_MODIFIER_LEFTSHIFT; keycode = HID_KEY_A + (c - 'A'); } 
    else if (c >= '1' && c <= '9') { keycode = HID_KEY_1 + (c - '1'); } 
    else if (c == '0') { keycode = HID_KEY_0; } 
    else if (c == ' ') { keycode = HID_KEY_SPACE; } 
    else if (c == '\n' || c == '\r') { keycode = HID_KEY_ENTER; }
    else if (c == '\t') { keycode = HID_KEY_TAB; }
    else if (c == '\b') { keycode = HID_KEY_BACKSPACE; }
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
    uint8_t asciiVal = (uint8_t)c;
    char code[5];
    snprintf(code, sizeof(code), "%04d", asciiVal);

    usb_hid.keyboardReport(0, KEYBOARD_MODIFIER_LEFTALT, NULL);
    delay(5); 

    for (int i = 0; i < 4; i++) {
        int digit = code[i] - '0';
        uint8_t keys[6] = { NUMPAD_KEYS[digit], 0, 0, 0, 0, 0 };
        usb_hid.keyboardReport(0, KEYBOARD_MODIFIER_LEFTALT, keys);
        delay(4); 
        usb_hid.keyboardReport(0, KEYBOARD_MODIFIER_LEFTALT, NULL);
        delay(3); 
    }
    usb_hid.keyboardRelease(0);
    delay(5); 
}

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pChar) {
        String rxValue = pChar->getValue().c_str();
        if (rxValue.length() > 0) {
            if (rxValue == "CMD:BIOS") {
                biosMode = true;
            } else if (rxValue == "CMD:WIN") {
                biosMode = false;
            } else if (rxValue == "CMD:CAD") {
                executeCad = true;
                updateDisplay(UI_ACTION, "Ctrl+Alt+Del");
            } else if (rxValue == "CMD:ENTER") {
                executeEnter = true;
                updateDisplay(UI_ACTION, "Enter");
            } else if (rxValue == "CMD:GET_VER") {
                String verResp = "VER:" + String(FIRMWARE_VERSION);
                pChar->setValue(verResp.c_str());
                pChar->notify();
            } else {
                textBuffer += String(rxValue);
                String shortMsg = rxValue;
                if(shortMsg.length() > 10) shortMsg = shortMsg.substring(0, 8) + "..";
                updateDisplay(UI_TYPING, shortMsg);
            }
        }
    }
};

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) { 
        deviceConnected = true; 
        if (usb_hid.ready()) {
            if (!isNumLockActive) updateDisplay(UI_NUMLOCK_OFF);
            else updateDisplay(UI_CONNECTED);
        }
    };
    void onDisconnect(BLEServer* pServer) { 
        deviceConnected = false; 
        pServer->startAdvertising(); 
        if (usb_hid.ready()) {
            if (!isNumLockActive) updateDisplay(UI_NUMLOCK_OFF);
            else updateDisplay(UI_BLE_WAIT);
        }
    }
};

void setup() {
    Serial.begin(115200);

    // 1. USB INITIALISIEREN & LED-CALLBACK REGISTRIEREN
    usb_hid.setBootProtocol(HID_ITF_PROTOCOL_KEYBOARD);
    usb_hid.setPollInterval(2);
    usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));
    usb_hid.setStringDescriptor("BLE BIOS Keyboard");
    
    // Callback registrieren (MUSS vor usb_hid.begin() stehen)
    usb_hid.setReportCallback(NULL, hid_report_callback);
    
    usb_hid.begin();

    // 2. DISPLAY STARTEN
#if ENABLE_DISPLAY
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
    SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
    display.init(135, 240); 
    display.setRotation(DISPLAY_ROTATION); 
    updateDisplay(UI_INIT, "Booting...");
#endif

    // 3. BLUETOOTH STARTEN
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

    // Initiale Statusabfrage nach dem Booten
    lastUsbReady = usb_hid.ready();
    if (lastUsbReady) {
        updateDisplay(UI_BLE_WAIT);
    } else {
        updateDisplay(UI_NO_USB, "NO USB\nWaiting...");
    }
}

void loop() {
    bool currentUsbReady = usb_hid.ready();

    if (currentUsbReady != lastUsbReady) {
        lastUsbReady = currentUsbReady;
        if (currentUsbReady) {
            if (textBuffer.length() == 0 && !executeCad && !executeEnter) {
                if (!isNumLockActive) updateDisplay(UI_NUMLOCK_OFF);
                else if (deviceConnected) updateDisplay(UI_CONNECTED);
                else updateDisplay(UI_BLE_WAIT);
            }
        } else {
            // Bei Disconnect Reset der Num-Lock Flags
            numLockAutoChecked = false; 
            triggerNumLock = false;
            updateDisplay(UI_NO_USB, "NO USB\nWaiting...");
        }
    }

    if (!currentUsbReady) {
        delay(10);
        return; 
    }

    // ===================================================================
    // --- NUM LOCK DISPLAY UPDATE CHECK (Reagiert auf Änderungen)
    // ===================================================================
    if (numLockStatusChanged) {
        numLockStatusChanged = false;
        if (!isNumLockActive) {
            updateDisplay(UI_NUMLOCK_OFF);
        } else {
            if (deviceConnected) updateDisplay(UI_CONNECTED);
            else updateDisplay(UI_BLE_WAIT);
        }
    }

    // ===================================================================
    // --- NUM LOCK TRIGGER (Sicher im Main-Loop ausführen)
    // ===================================================================
    if (triggerNumLock) {
        triggerNumLock = false; 

        Serial.println("[USB] Num Lock ist AUS -> Aktiviere jetzt im Main-Loop...");
        
        delay(300); 

        uint8_t keyPress[6] = { HID_KEY_NUM_LOCK, 0, 0, 0, 0, 0 };
        uint8_t keyRelease[6] = { 0, 0, 0, 0, 0, 0 };

        usb_hid.keyboardReport(0, 0, keyPress);
        delay(15);
        usb_hid.keyboardReport(0, 0, keyRelease);

        Serial.println("[USB] Num Lock ohne Popup getriggert!");
    }

    if (executeCad) {
        executeCad = false;
        uint8_t keys[6] = { HID_KEY_DELETE, 0, 0, 0, 0, 0 };
        usb_hid.keyboardReport(0, KEYBOARD_MODIFIER_LEFTCTRL | KEYBOARD_MODIFIER_LEFTALT, keys);
        delay(100); 
        usb_hid.keyboardRelease(0); 
        
        if (!isNumLockActive) updateDisplay(UI_NUMLOCK_OFF);
        else if (deviceConnected) updateDisplay(UI_CONNECTED);
    }

    if (executeEnter) {
        executeEnter = false;
        uint8_t keys[6] = { HID_KEY_ENTER, 0, 0, 0, 0, 0 };
        usb_hid.keyboardReport(0, 0, keys);
        delay(50);
        usb_hid.keyboardRelease(0);
        
        if (!isNumLockActive) updateDisplay(UI_NUMLOCK_OFF);
        else if (deviceConnected) updateDisplay(UI_CONNECTED);
    }

    if (textBuffer.length() > 0) {
        char c = textBuffer[0];
        textBuffer.remove(0, 1);

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
        
        if(textBuffer.length() == 0) {
            if (!isNumLockActive) updateDisplay(UI_NUMLOCK_OFF);
            else if (deviceConnected) updateDisplay(UI_CONNECTED);
        }
    }
    delay(10);
}