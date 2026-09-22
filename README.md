# ⌨️ ESP32-S3 BLE Hardware Keyboard Injector

![License](https://img.shields.io/github/license/panthaflow/ESP32_BLE_Keyboard?color=blue) &nbsp;&nbsp;&nbsp; [![Stars](https://img.shields.io/github/stars/panthaflow/ESP32_BLE_Keyboard?style=social)](https://github.com/panthaflow/ESP32_BLE_Keyboard/stargazers) &nbsp;&nbsp;&nbsp; ![Release](https://img.shields.io/github/v/release/panthaflow/ESP32_BLE_Keyboard?color=green) &nbsp;&nbsp;&nbsp; ![ESP32-S3](https://img.shields.io/badge/Hardware-ESP32--S3-orange)

[English](#-english) | [Deutsch](#-deutsch) | [🌐 Live Demo](https://panthaflow.github.io/ESP32_BLE_Keyboard/)

---

## 🇬🇧 English

A versatile USB keyboard simulator based on the **ESP32-S3** (e.g., *Waveshare ESP32-GEEK*). 
The device connects via **Web Bluetooth** directly through any modern web browser (no app installation required) from a smartphone or PC, injecting received text as a native USB hardware keyboard into the target computer.

Supports a lightning-fast **Windows mode via Alt codes** and a **BIOS Boot Protocol mode** for direct input in server BIOS, UEFI, iDRAC/iLO, and pre-boot environments.

> 🚀 **Try it out directly:** Launch the [Web Bluetooth Control Panel (Live Demo)](https://panthaflow.github.io/ESP32_BLE_Keyboard/) in your browser.

---

### ✨ Features

* **⚡ Native USB HID Stack:** Uses `Adafruit_TinyUSB` for maximum compatibility with legacy systems and server BIOS environments.
* **🌐 Browser Control via Web Bluetooth:** No app installation required. Simply open the included web interface (or use the [Live Demo](https://panthaflow.github.io/ESP32_BLE_Keyboard/)) and connect instantly.
* **🔄 Two Operating Modes:**
  * **BIOS Mode:** Forces the USB Boot Protocol, adds artificial delays to keypresses, and maps input precisely to the US QWERTY keyboard layout.
  * **Windows Mode:** Sends characters at high speed via Numpad Alt codes (~35 ms per character). Guarantees accurate character transmission regardless of the target PC's set keyboard layout.
* **📦 Asynchronous Text Buffer:** Prevents watchdog resets and data loss during long text injections.
* **🔘 Special Commands:** Built-in support for macro operations (e.g., `Ctrl + Alt + Del`).

---

### 🛠️ Hardware Requirements

* **ESP32-S3 board with native USB** (Recommended: *Waveshare ESP32-GEEK* or *ESP32-S3 DevKitC*).
* USB-A cable or direct USB-A port on the target system.

---

### ⚡ Flashing & IDE Setup

To ensure the ESP32-S3 is recognized as a genuine USB hardware keyboard by server BIOS systems, you must configure the Arduino IDE settings correctly.

#### 1. Required Libraries
Install the following libraries via the Arduino IDE **Library Manager** (`Ctrl` + `Shift` + `I`):
* **Adafruit TinyUSB Library** (by Adafruit)
* Standard **ESP32 BLE** libraries (included in the ESP32 board package)

#### 2. Board Configuration
Select the following options under **Tools**:
* **Board:** `ESP32S3 Dev Module` (or your specific board like *Waveshare ESP32-S3-GEEK*)
* **USB Mode:** `USB-OTG (TinyUSB)` ⚠️ *(Mandatory for keyboard emulation)*
* **USB CDC On Boot:** `Disabled` *(Prevents dual COM port conflicts in BIOS)*
* **Flash Size:** `8MB` or `16MB` (depending on your board)
* **Partition Scheme:** `Huge APP (3MB No OTA/1MB SPIFFS)`

#### 3. Step-by-Step Flashing
1. **Enable Boot Mode:** Press and hold the physical **BOOT button** on your ESP32-S3 while plugging it into your development PC.
2. **Select Port:** In the Arduino IDE, choose the serial port under **Tools -> Port**.
3. **Upload Code:** Click the **Upload** button and wait for *"Done uploading"*.
4. **Reboot:** Re-plug the ESP32-S3 or press the **RESET button** to start the firmware.

---

### 🚀 Control Protocol

The web interface sends the following raw strings over the BLE characteristic:

| Command | Function |
| :--- | :--- |
| `CMD:BIOS` | Switches to **BIOS Mode** (US Layout, delayed typing for legacy systems). |
| `CMD:WIN` | Switches to **Windows Mode** (fast typing via Alt codes). |
| `CMD:CAD` | Triggers the **`Ctrl + Alt + Del`** key combination. |
| `CMD:ENTER` | Sends the **`Enter`** key. |
| *Any other text* | Typed directly character-by-character over USB. |

---
---

## 🇩🇪 Deutsch

Ein vielseitiger USB-Tastatur-Simulator auf Basis des **ESP32-S3** (z. B. *Waveshare ESP32-GEEK*). 
Das Gerät verbindet sich per **Web Bluetooth** direkt über den Webbrowser (ohne App-Installation) mit einem Smartphone oder PC und gibt empfangenen Text als native USB-Hardware-Tastatur an den Ziel-Computer weiter.

Unterstützt einen extrem schnellen **Windows-Modus via Alt-Codes** sowie einen **BIOS-Boot-Protocol-Modus** für direkte Eingaben in Server-BIOS-, UEFI- und Pre-Boot-Umgebungen.

> 🚀 **Direkt ausprobieren:** Öffne das [Web Bluetooth Control Panel (Live Demo)](https://panthaflow.github.io/ESP32_BLE_Keyboard/) direkt in deinem Browser.

---

### ✨ Features

* **⚡ Nativer USB-HID-Stack:** Verwendet `Adafruit_TinyUSB` für maximale Kompatibilität mit alten Systemen und Server-BIOS.
* **🌐 Browser-Steuerung via Web Bluetooth:** Keine App-Installation nötig. Einfach die [Live Demo](https://panthaflow.github.io/ESP32_BLE_Keyboard/) aufrufen (oder die mitgelieferte `index.html` lokal nutzen) und sofort verbinden.
* **🔄 Zwei Betriebsmodi:**
  * **BIOS-Modus:** Erzwingt das USB-Boot-Protokoll, verlangsamt die Tastenanschläge künstlich und mappt Eingaben präzise auf das englische US-Layout.
  * **Windows-Modus:** Sendet Zeichen blitzschnell via Numpad-Alt-Codes (ca. 35 ms pro Zeichen). Garantiert korrekte Zeichenübertragung unabhängig vom Tastaturlayout des Ziel-PCs.
* **📦 Asynchroner Text-Puffer:** Verhindert Watchdog-Resets und Datenverlust bei langen Texteingaben.
* **🔘 Spezialbefehle:** Unterstützt vordefinierte Steuerbefehle für Macro-Funktionen (z. B. `Strg + Alt + Entf`).

---

### 🛠️ Hardware-Voraussetzungen

* **ESP32-S3 Board mit nativem USB** (Empfohlen: *Waveshare ESP32-GEEK* oder *ESP32-S3 DevKitC*).
* USB-Kabel / direkter USB-A-Port zum Ziel-System.

---

### ⚡ Flashing & IDE-Einrichtung

Damit der ESP32-S3 vom Server-BIOS als echte USB-Hardware-Tastatur erkannt wird, müssen die USB-Optionen in der Arduino IDE zwingend korrekt gesetzt sein.

#### 1. Erforderliche Bibliotheken
Installiere die folgenden Bibliotheken über den **Bibliotheksverwalter** der Arduino IDE (`Strg` + `Umschalt` + `I`):
* **Adafruit TinyUSB Library** (von Adafruit)
* Standard **ESP32 BLE** Bibliotheken (im ESP32 Board-Paket enthalten)

#### 2. Board-Einstellungen
Wähle im Menü unter **Werkzeuge (Tools)** folgende Optionen aus:
* **Board:** `ESP32S3 Dev Module` *(oder z. B. Waveshare ESP32-S3-GEEK)*
* **USB Mode:** `USB-OTG (TinyUSB)` ⚠️ *(Absolut notwendig für die Tastatur-Emulation)*
* **USB CDC On Boot:** `Disabled` *(Verhindert einen doppelten COM-Port-Eintrag)*
* **Flash Size:** `8MB` oder `16MB`
* **Partition Scheme:** `Huge APP (3MB No OTA/1MB SPIFFS)`

#### 3. Schritt-für-Schritt Flash-Anleitung
1. **Boot-Modus aktivieren:** Halte die **BOOT-Taste** am ESP32-S3 gedrückt und stecke das Board gleichzeitig in den USB-Anschluss deines PCs.
2. **Port auswählen:** Wähle unter **Werkzeuge -> Port** den Seriellen Port aus.
3. **Code hochladen:** Klicke auf **Upload** und warte auf *"Done uploading"*.
4. **Neustart:** Ziehe das Board kurz ab oder drücke die **RESET-Taste**.

---

### 🚀 Steuerbefehle (Protokoll)

Die Webapp sendet folgende Strings über die BLE-Charakteristik an den ESP32-S3:

| Befehl | Funktion |
| :--- | :--- |
| `CMD:BIOS` | Schaltet in den **BIOS-Modus** (US-Layout, verlangsamtes Tippen). |
| `CMD:WIN` | Schaltet in den **Windows-Modus** (schnelles Tippen via Alt-Codes). |
| `CMD:CAD` | Sendet das Tastenkürzel **`Strg + Alt + Entf`** (Ctrl+Alt+Del). |
| `CMD:ENTER` | Sendet die **`Enter`**-Taste. |
| *Jeder andere Text* | Wird direkt Zeichen für Zeichen über die USB-Schnittstelle getippt. |

---

## 📝 License / Lizenz

Distributed under the **Apache License 2.0**. See `LICENSE` for more information.
