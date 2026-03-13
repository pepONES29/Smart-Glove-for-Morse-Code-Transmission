#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// --- CONFIGURARE OLED ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// --- CONFIGURARE BLE (UART Service) ---
BLECharacteristic *pCharacteristicTX;
bool deviceConnected = false;

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" 
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println(">>> Dispozitiv conectat!");
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println(">>> Dispozitiv deconectat! Repornesc advertising...");
      pServer->getAdvertising()->start(); 
    }
};

// --- PARAMETRI SENZOR ---
const int flexPin = 33;
const int PRAG_START = 950;  
const int PRAG_LINIE = 650;  
const int MIN_DURATA = 80;    
const int PAUZA_LITERA = 1500;
const int PAUZA_CUVANT = 3500;

// --- VARIABILE LOGICA ---
String propozitie = "";
String bufferMorse = "";
String cuvantCurent = ""; 
unsigned long momentStartFlexie = 0;
unsigned long momentUltimulInput = 0;
bool miscareActiva = false;
bool cuvantIncheiat = true;
int valMinima = 4095;

char decodeMorse(String s);
void updateUI();

void setup() {
  // --- REZOLVARE BROWNOUT ---
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 

  Serial.begin(115200);
  Wire.begin(21, 22);
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED esuat!");
  } else {
    display.clearDisplay();
    display.display();
  }

  // Initializare BLE
  BLEDevice::init("ManusaMorse_iOS");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristicTX = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY);
  pCharacteristicTX->addDescriptor(new BLE2902());
  pService->createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE);

  pService->start();
  pServer->getAdvertising()->start();
}

void loop() {
  int raw = analogRead(flexPin);
  unsigned long acum = millis();

  if (raw < PRAG_START) {
    if (!miscareActiva) {
      miscareActiva = true;
      momentStartFlexie = acum;
      valMinima = 4095;
    }
    if (raw < valMinima) valMinima = raw;
  } 
  else if (miscareActiva && raw > PRAG_START) {
    unsigned long durata = acum - momentStartFlexie;
    miscareActiva = false;
    if (durata > MIN_DURATA) {
      bufferMorse += (valMinima < PRAG_LINIE) ? "-" : ".";
      cuvantIncheiat = false; 
      updateUI();
    }
    momentUltimulInput = acum;
  }

  if (!miscareActiva && bufferMorse.length() > 0 && (acum - momentUltimulInput > PAUZA_LITERA)) {
    char c = decodeMorse(bufferMorse);
    if (c != '?') {
      propozitie += c;
      cuvantCurent += c;
      updateUI();
    }
    bufferMorse = "";
    momentUltimulInput = acum;
  }

  if (!miscareActiva && !cuvantIncheiat && (acum - momentUltimulInput > PAUZA_CUVANT)) {
    if (cuvantCurent.length() > 0) {
      if (deviceConnected) {
        String deTrimis = cuvantCurent + " ";
        pCharacteristicTX->setValue(deTrimis.c_str());
        pCharacteristicTX->notify();
      }
      propozitie += " ";
      cuvantCurent = "";
    }
    cuvantIncheiat = true;
    updateUI();
  }
  delay(15);
}

void updateUI() {
  display.clearDisplay();
  display.setTextColor(WHITE);
  
  // Indicator Status Bluetooth (Dreapta-Sus)
  display.setTextSize(1);
  display.setCursor(110, 0); 
  display.print(deviceConnected ? "BT" : ".."); 

  display.setCursor(0, 0);
  display.print("Cod: "); 
  display.println(bufferMorse);
  display.drawLine(0, 12, 128, 12, WHITE);
  
  display.setTextSize(2);
  display.setCursor(0, 25);
  String deAfisat = propozitie;
  if (deAfisat.length() > 10) deAfisat = deAfisat.substring(deAfisat.length() - 10);
  display.print(deAfisat);
  display.display();
}

char decodeMorse(String s) {
  if (s == ".-") return 'A';   if (s == "-...") return 'B';
  if (s == "-.-.") return 'C'; if (s == "-..") return 'D';
  if (s == ".") return 'E';    if (s == "..-.") return 'F';
  if (s == "--.") return 'G';  if (s == "....") return 'H';
  if (s == "..") return 'I';   if (s == ".---") return 'J';
  if (s == "-.-") return 'K';  if (s == ".-..") return 'L';
  if (s == "--") return 'M';   if (s == "-.") return 'N';
  if (s == "---") return 'O';  if (s == ".--.") return 'P';
  if (s == "--.-") return 'Q'; if (s == ".-.") return 'R';
  if (s == "...") return 'S';  if (s == "-") return 'T';
  if (s == "..-") return 'U';  if (s == "...-") return 'V';
  if (s == ".--") return 'W';  if (s == "-..-") return 'X';
  if (s == "-.--") return 'Y'; if (s == "--..") return 'Z';
  if (s == ".----") return '1'; if (s == "..---") return '2';
  if (s == "...--") return '3'; if (s == "....-") return '4';
  if (s == ".....") return '5'; if (s == "-....") return '6';
  if (s == "--...") return '7'; if (s == "---..") return '8';
  if (s == "----.") return '9'; if (s == "-----") return '0';
  return '?';
}