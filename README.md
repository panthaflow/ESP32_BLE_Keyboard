# ⌨️ ESP32-S3 BLE Hardware Keyboard Injector

Ein vielseitiger USB-Tastatur-Simulator auf Basis des **ESP32-S3** (z. B. Waveshare ESP32-GEEK). 
Das Gerät verbindet sich per **Bluetooth Low Energy (BLE)** mit einem Smartphone/PC und gibt empfangenen Text als native USB-Hardware-Tastatur an den Ziel-Computer weiter.

Unterstützt einen extrem schnellen **Windows-Modus via Alt-Codes** sowie einen **BIOS-Boot-Protocol-Modus** für direkte Eingaben in Server-BIOS-, UEFI- und Pre-Boot-Umgebungen.

---

## ✨ Features

* **⚡ Nativer USB-HID-Stack:** Verwendet `Adafruit_TinyUSB` für maximale Kompatibilität mit alten Systemen und Server-BIOS[cite: 1].
* **🔄 Zwei Betriebsmodi:**
  * **BIOS-Modus:** Erzwingt das USB-Boot-Protokoll, verlangsamt die Tastenanschläge künstlich und mappt Eingaben präzise auf das englische US-Layout.
  * **Windows-Modus:** Sendet Zeichen blitzschnell via Numpad-Alt-Codes (ca. 35 ms pro Zeichen). Garantiert korrekte Zeichenübertragung unabhängig vom Tastaturlayout des Ziel-PCs.
* **📦 Asynchroner Text-Puffer:** Verhindert Watchdog-Resets und Datenverlust bei langen Texteingaben.
* **🔘 Spezialbefehle:** Unterstützt vordefinierte Steuerbefehle für Macro-Funktionen (z. B. `Strg + Alt + Entf`).
* **📱 BLE-Steuerung:** Steuerung über jede beliebige BLE-Serial- oder Terminal-App (z. B. Serial Bluetooth Terminal).

---

## 🛠️ Hardware-Voraussetzungen

* **ESP32-S3 Board mit nativem USB** (Empfohlen: *Waveshare ESP32-GEEK* oder *ESP32-S3 DevKitC*).
* USB-Kabel / direkter USB-A-Port zum Ziel-System.

---

## 💻 Software & Einstellungen (Arduino IDE)

### 1. Erforderliche Bibliotheken
Installiere die folgenden Bibliotheken über den **Bibliotheksverwalter** der Arduino IDE:
* `Adafruit TinyUSB Library`
* Standard ESP32 BLE Bibliotheken (im ESP32 Board-Paket enthalten)

### 2. Board-Einstellungen
Wähle dein ESP32-S3 Board aus und setze folgende Werte unter **Werkzeuge (Tools)**:
* **Board:** `ESP32S3 Dev Module` (oder entsprechendes spezifisches Board)
* **USB Mode:** `USB-OTG (TinyUSB)` *(Wichtig!)*
* **USB CDC On Boot:** `Disabled` *(Empfohlen für maximale BIOS-Kompatibilität)*

---

## 🚀 Steuerbefehle (über BLE)

Sende die folgenden Befehle als Text über die Bluetooth-Verbindung, um den Modus zu wechseln oder Sonderaktionen auszuführen:

| Befehl | Funktion |
| :--- | :--- |
| `CMD:BIOS` | Schaltet in den **BIOS-Modus** (US-Layout, verlangsamtes Tippen für Legacy-Systeme). |
| `CMD:WIN` | Schaltet in den **Windows-Modus** (schnelles Tippen via Alt-Codes). |
| `CMD:CAD` | Sendet das Tastenkürzel **`Strg + Alt + Entf`** (Ctrl+Alt+Del). |
| `CMD:ENTER` | Sendet die **`Enter`**-Taste. |
| *Jeder andere Text* | Wird direkt Zeichen für Zeichen über die USB-Schnittstelle getippt. |

