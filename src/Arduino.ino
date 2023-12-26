#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

// PIN Yang digunakan
const int SERVOPIN = 6;
const int TRIGPIN1 = 12;
const int ECHOPIN1 = 13;
const int TRIGPIN2 = 8;
const int ECHOPIN2 = 9;
const int BUZZPIN = 7;
const int GASSENSOR = A0;
const int RX_PIN = 3;
const int TX_PIN = 2;

// Jarak untuk membuka tempat sampah dalam cm
const int DISTANCE_TRIG = 8;

// Object initialization
SoftwareSerial ArduinoUno(RX_PIN, TX_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo myServo;

// Var initialization
const int CLOSED_POS = 180;  // 180 derajat saat posisi tempat sampah tertutup
const int OPEN_POS = 0;      // 0 derajat saat posisi tempat sampah terbuka
float distance;              // Untuk menyimpan hasil jarak dari sensor ultrasonic (cm)
int gasValue;                // Untuk menyimpan pembacaan sensor mq-2
int binVolume;               // Untuk menyimpan volume sampah
String msg;                  // Menyimpan data yang dikirim ke NodeMCU
int distanceTmp = 0;         // Menyimpan data jarak sebelumnya
bool displayPause = false;

void setup() {
  // Pengaturan PIN, lcd, servo, dan serial com
  pinMode(TRIGPIN1, OUTPUT);
  pinMode(ECHOPIN1, INPUT);
  pinMode(TRIGPIN2, OUTPUT);
  pinMode(ECHOPIN2, INPUT);
  pinMode(BUZZPIN, OUTPUT);
  pinMode(GASSENSOR, INPUT);

  servoFunc(CLOSED_POS);

  Wire.setClock(10000);

  lcd.init();
  lcd.backlight();

  // Serial communication initialization
  Serial.begin(9600);
  ArduinoUno.begin(9600);

  displayBinVolume();
}

void loop() {
  displayBinVolume();
  distance = distanceUltrasonic(TRIGPIN2, ECHOPIN2);
  gasValue = analogRead(GASSENSOR);
  msg = String(gasValue) + "#" + String(binVolume);
  ArduinoUno.println(msg);  // Kirim Serial ke NodeMCU

  // Tempat sampah terbuka saat jarak terbaca kurang dari DISTANCE_TRIG
  if (distance != 0) {
    if (distance <= DISTANCE_TRIG) {
      tone(BUZZPIN, 1046, 100);
      servoFunc(OPEN_POS);
      displayThankYou();
      displayPause = true;
    } else {
      servoFunc(CLOSED_POS);
      displayPause = false;
    }
  }

  // Buzzer bunyi saat ppm bernilai lebih dari 5000
  while (gasValue >= 512) {  // ppm mq2 = ppm level/max ppm * max bit (10)
    tone(BUZZPIN, 2092, 300);
    delay(500);
    gasValue = analogRead(GASSENSOR);
  }
}

void servoFunc(int degree) {
  myServo.attach(SERVOPIN);
  myServo.write(degree);
  delay(500);
  myServo.detach();
}

int distanceUltrasonic(int trig, int echo) {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  float travelTime = pulseIn(echo, HIGH);  // data waktu microseconds
  int result = 0.034 * travelTime / 2;     // 34cm/ms * travelTime / 2
  if (result >= 2) {
    distanceTmp = result;
    return result;
  } else {
    return distanceTmp;
  }
}

void displayMessage(const char *line1, const char *line2, int delayTime) {
  lcd.clear();

  // Kalkulasi agar text berada pada tengah layar
  int pos1 = (16 - strlen(line1)) / 2;
  int pos2 = (16 - strlen(line2)) / 2;

  lcd.setCursor(pos1, 0);
  lcd.print(line1);
  lcd.setCursor(pos2, 1);
  lcd.print(line2);
  delay(delayTime);
}

void displayThankYou() {
  displayMessage("Terimakasih", "Sudah", 1000);
  displayMessage("Buang Sampah", "Pada Tempatnya", 1000);
  lcd.clear();
}

void displayBinVolume() {
  if (displayPause) return;
  lcd.clear();
  binVolume = (25 - distanceUltrasonic(TRIGPIN1, ECHOPIN1)) * 100 / 25;
  Serial.println(distanceUltrasonic(TRIGPIN1, ECHOPIN1));
  lcd.setCursor(0, 0);
  lcd.print("Volume");
  lcd.setCursor(0, 1);
  lcd.print("Sampah : ");
  if (binVolume < 0) {
    lcd.print("");
  } else {
    lcd.print(binVolume);
  }
  lcd.print("%");
}
