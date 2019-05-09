# Projekt für Ölstand-Messung

## Verkabelung
| Ultraschall Sensor | D1 mini | SD Modul |
|:------------------:|:-------:|:--------:|
|         Vcc        |    5V   |    VCC   |
|         Gnd        |    G    |    GND   |
|                    |    D8   |    CS    |
|                    |    D7   |   MOSI   |
|                    |    D6   |   MISO   |
|                    |    D5   |    SCK   |
|        Trig        |    D4   |          |
|         D3         |    D3   |          |

## Setup
### SD-Karte
* Formatieren auf FAT32
* data.csv erstellen (leere Datei)
* unixtime.txt erstellen
* Unix-Zeit in SEKUNDEN in unixtime.txt eintragen (UND NICHTS ANDERES!)
* SD-Karte in SD-Modul einstecken

### App-Server
Wenn Daten an einen eigenen Server gesendet werden sollen, muss dieser aufgesetzt und gestartet werden.  
Ein fertiger Node.js-Server mitsamt UI5-App befindet sich im Repository `https://github.com/Dooobi/oil-measurement-server`.
Die URL, wohin die Daten gesendet werden wird über ein `#define` im Programmcode eingetragen.  
Die Variablen `{timestamp}`, `{distance}` und `{volume}` können an den Stellen verwendet werden, wo die Daten eingefügt werden sollen.  
Beispiel (für den fertigen Node.js-Server):  
`#define ADD_DATA_URL "http://192.168.0.11:3000/addData?timestamp={timestamp}&distance={distance}&volume={volume}"`

### IFTTT Event (z.B. für Email-Versand)
Es kann ein IFTTT-Event ausgelöst werden.  
Hierfür müssen IFTTT ID und Eventname über `#define` im Programmcode eingetragen werden.  
Das Event wird mit einem `GET` ausgelöst und die Werte werden wie folgt befüllt:  

| IFTTT Variable |                       Wert                      |
|:--------------:|:-----------------------------------------------:|
|     value1     |  Zeitstempel im Format `DD.MM.YYYY%20HH:mm:ss`  |
|     value2     | Gemessene Distanz des Ultraschall-Sensors in cm |
|     value3     |           Berechnetes Volumen in Liter          |

### Nötige Einstellungen im Programmcode
#### Einstellungen über `#define`
Nötige Einstellungen:
* `WIFI_SSID`: für WLAN-Verbindung (Zeitsynchronisierung, Daten an Server und IFTTT senden)
* `WIFI_PW`: für WLAN-Verbindung (Zeitsynchronisierung, Daten an Server und IFTTT senden)
* `GATEWAY`: für WLAN-Verbindung (Zeitsynchronisierung, Daten an Server und IFTTT senden)
* `SUBNET`: für WLAN-Verbindung (Zeitsynchronisierung, Daten an Server und IFTTT senden)
* `IFTTT_ID`: ID vom IFTTT Service für Events
* `IFTTT_EVENT`: Eventname vom IFTTT Service
* `ADD_DATA_URL`: URL um Daten an Server zu senden (siehe [App-Server](App-Server))
* `MIN_DISTANCE`: Kleinste Distanz von Öloberfläche zu Ultraschallsensor (wenn Tank voll ist)
* `MAX_DISTANCE`: Größte Distanz von Öloberfläche zu Ultraschallsensor (wenn Tank leer ist)
* `VOLUME_AT_MIN_DISTANCE`: Volumen des Öls im Tank (in Liter) wenn Tank voll ist (also bei `MIN_DISTANCE`)
* `VOLUME_AT_MAX_DISTANCE`: Volumen des Öls im Tank (in Liter) wenn Tank leer ist (also bei `MAX_DISTANCE`)

#### Einstellungen direkt im Programmcode
Der Auslösezeitpunkt für die Messungen und Aktionen muss manuell programmiert werden.  
Die relevante Stelle ist in der loop()-Funktion in Zeile ~83.  
Ausschnitt:
```
  int currTime = now();
  if (prevTime != 0) {
    int prevMin = day(prevTime);
    int currMin = day(currTime);

    if (prevMin != currMin) {
      // Stuff happens here...
    }
```
