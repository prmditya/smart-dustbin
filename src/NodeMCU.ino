#define BLYNK_TEMPLATE_ID "BLYNK_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "BLYNK_TEMPLATE_NAME"
#define BLYNK_AUTH_TOKEN "BLYNK_AUTH_TOKEN"

#include <SoftwareSerial.h>
#include <BlynkSimpleEsp8266.h>

const int RX_PIN = D5;
const int TX_PIN = D6;

// Object initialization
SoftwareSerial NodeMCU(RX_PIN, TX_PIN);

// Set ssid dan password wifi
char ssid[] = "WIFI_SSID";
char pass[] = "WIFI_PASSWORD";

String data;        // Data hasil baca serial
String gasValue;    // Data hasil parsing berupa data gas
String binVolume;   // Data hasil parsing berupa volume sampah
int dataCount = 0;

int gasValueInt = 0;
int binVolumeInt = 0;
int gasInPPM = 0;

void setup() {
  Serial.begin(9600);
  NodeMCU.begin(9600);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
}

void loop() {
  while (NodeMCU.available() > 0) {
    char incomingChar = NodeMCU.read();
    data += incomingChar;

    if (incomingChar == '\n') {
      // Jika ditemukan karakter newline, proses data
      processSerialData(data);
      data = "";  // Reset string untuk menerima data berikutnya
    }
  }
  
  notificationAndDisplay();
}

void processSerialData(String data) {
  gasValue = "";
  binVolume = "";
  dataCount = 0;

  for (int i = 0; i < data.length(); i++) {
    char currentChar = data[i];

    if (currentChar != '#' && currentChar != '\n') {
      if (dataCount == 0) {
        gasValue += currentChar;
      } else if (dataCount == 1) {
        binVolume += currentChar;
      }
    } else {
      dataCount++;
    }
  }

  // Konversi String menjadi nilai numerik jika diperlukan
  gasValueInt = gasValue.toInt();
  binVolumeInt = binVolume.toInt();
  gasInPPM = map(gasValueInt, 0, 1023, 200, 10000);
}

void notificationAndDisplay(){
  Blynk.virtualWrite(V1, gasInPPM);
  Blynk.virtualWrite(V0, binVolumeInt);

  if(gasValueInt > 512){
    Blynk.logEvent("flamable_gas_alerts", String("Bahaya Kebakaran ppm = ") + gasInPPM);
  }

  if(binVolumeInt > 80){
    Blynk.logEvent("trash_volume", String("Tong sampah hampir penuh!!"));
  }else if (binVolumeInt == 100){
    Blynk.logEvent("trash_volume", String("Tong sampah sudah penuh!!"));
  }
}
