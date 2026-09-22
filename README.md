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
* **USB CDC On Boot:** `Disabled` *(Verhindert ein doppelter COM-Port-Eintrag, der manche BIOS-Systeme verwirrt)*
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
