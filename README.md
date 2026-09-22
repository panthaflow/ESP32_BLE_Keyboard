# ⌨️ ESP32-S3 BLE Hardware Keyboard Injector

Ein vielseitiger USB-Tastatur-Simulator auf Basis des **ESP32-S3** (z. B. Waveshare ESP32-GEEK). 
Das Gerät verbindet sich per **Web Bluetooth** direkt über den Webbrowser (ohne App-Installation) mit einem Smartphone oder PC und gibt empfangenen Text als native USB-Hardware-Tastatur an den Ziel-Computer weiter.

Unterstützt einen extrem schnellen **Windows-Modus via Alt-Codes** sowie einen **BIOS-Boot-Protocol-Modus** für direkte Eingaben in Server-BIOS-, UEFI- und Pre-Boot-Umgebungen.

---

## ✨ Features

* **⚡ Nativer USB-HID-Stack:** Verwendet `Adafruit_TinyUSB` für maximale Kompatibilität mit alten Systemen und Server-BIOS.
* **🌐 Browser-Steuerung via Web Bluetooth:** Keine App-Installation nötig. Einfach die mitgelieferte `index.html` aufrufen (z. B. via GitHub Pages auf dem Smartphone) und sofort verbinden.
* **🔄 Zwei Betriebsmodi:**
  * **BIOS-Modus:** Erzwingt das USB-Boot-Protokoll, verlangsamt die Tastenanschläge künstlich und mappt Eingaben präzise auf das englische US-Layout.
  * **Windows-Modus:** Sendet Zeichen blitzschnell via Numpad-Alt-Codes (ca. 35 ms pro Zeichen). Garantiert korrekte Zeichenübertragung unabhängig vom Tastaturlayout des Ziel-PCs.
* **📦 Asynchroner Text-Puffer:** Verhindert Watchdog-Resets und Datenverlust bei langen Texteingaben.
* **🔘 Spezialbefehle:** Unterstützt vordefinierte Steuerbefehle für Macro-Funktionen (z. B. `Strg + Alt + Entf`).

---

## 🛠️ Hardware-Voraussetzungen

* **ESP32-S3 Board mit nativem USB** (Empfohlen: *Waveshare ESP32-GEEK* oder *ESP32-S3 DevKitC*).
* USB-Kabel / direkter USB-A-Port zum Ziel-System.

---

## 💻 Software & Arduino IDE Einrichtung

### 1. Erforderliche Bibliotheken
Installiere die folgenden Bibliotheken über den **Bibliotheksverwalter** der Arduino IDE (`Strg` + `Umschalt` + `I`):
* **Adafruit TinyUSB Library** (von Adafruit)
* Standard **ESP32 BLE** Bibliotheken (bereits im ESP32 Board-Paket enthalten)

---

## ⚡ Flashen & Board-Einstellungen

Damit der ESP32-S3 vom Server-BIOS als echte USB-Hardware-Tastatur erkannt wird, müssen die USB-Optionen in der Arduino IDE zwingend korrekt gesetzt sein.

### 1. IDE-Einstellungen konfigurieren
Wähle im Menü unter **Werkzeuge (Tools)** folgende Optionen aus:

* **Board:** `ESP32S3 Dev Module` *(oder spezifisch z. B. Waveshare ESP32-S3-GEEK)*
* **USB Mode:** `USB-OTG (TinyUSB)` ⚠️ *(Absolut notwendig für die Tastatur-Emulation)*
* **USB CDC On Boot:** `Disabled` *(Verhindert einen doppelten COM-Port-Eintrag, der manche BIOS-Systeme verwirrt)*
* **Flash Size:** `8MB` oder `16MB` *(Je nach deinem spezifischen ESP32-S3 Board)*
* **Partition Scheme:** `Huge APP (3MB No OTA/1MB SPIFFS)` *(Bietet ausreichend Speicherplatz für BLE + USB)*

---

### 2. Der Flash-Vorgang (Anleitung Schritt für Schritt)

<Steps>
  <Step title="Boot-Modus aktivieren" subtitle="Hardware-Vorbereitung">
    Halte die physikalische **BOOT-Taste** am ESP32-S3 gedrückt und stecke das Board gleichzeitig in den USB-Anschluss deines Entwicklungs-PCs. Das versetzt den ESP32-S3 in den internen ROM-Bootloader.
  </Step>
  <Step title="Port auswählen" subtitle="Arduino IDE">
    Wähle unter **Werkzeuge -> Port** den neu aufgetauchten COM-Port/Seriellen Port deines Boards aus.
  </Step>
  <Step title="Code hochladen" subtitle="Upload">
    Klicke auf den **Upload-Button** (Pfeil nach rechts) in der Arduino IDE und warte, bis der Vorgang mit *"Done uploading"* abgeschlossen ist.
  </Step>
  <Step title="Neustart" subtitle="Hardware">
    Ziehe den ESP32-S3 kurz ab und stecke ihn neu ein (oder drücke kurz die **RESET-Taste**), um die neue Firmware zu starten.
  </Step>
</Steps>

---

## 🌐 Web-Interface (Web Bluetooth)

Für die Steuerung über den Browser ist eine einfache `index.html` enthalten.

> **Hinweis:** Die Web Bluetooth API erfordert einen unterstützten Browser (Chrome, Edge, Opera) und muss entweder über `https://` (z. B. via GitHub Pages) oder lokal aufgerufen werden.

### Funktionsweise der Weboberfläche:
1. Auf **"Verbinden"** klicken und das Gerät **ESP32-S3 Keyboard** auswählen.
2. Den Modus wählen (**BIOS** oder **Windows**).
3. Text eingeben und absenden oder Buttons für Schnellbefehle (`Enter`, `Strg+Alt+Entf`) nutzen.

---

## 🚀 Steuerbefehle (Protokoll)

Die Webapp sendet folgende Strings über die BLE-Charakteristik an den ESP32-S3:

| Befehl | Funktion |
| :--- | :--- |
| `CMD:BIOS` | Schaltet in den **BIOS-Modus** (US-Layout, verlangsamtes Tippen für Legacy-Systeme). |
| `CMD:WIN` | Schaltet in den **Windows-Modus** (schnelles Tippen via Alt-Codes). |
| `CMD:CAD` | Sendet das Tastenkürzel **`Strg + Alt + Entf`** (Ctrl+Alt+Del). |
| `CMD:ENTER` | Sendet die **`Enter`**-Taste. |
| *Jeder andere Text* | Wird direkt Zeichen für Zeichen über die USB-Schnittstelle getippt. |

---

## 📝 Lizenz

Dieses Projekt steht unter der [MIT License](LICENSE).
