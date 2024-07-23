#define BLYNK_TEMPLATE_ID "TMPL3Md4q5vGD"
#define BLYNK_TEMPLATE_NAME "IOT PROJECT"
#define BLYNK_AUTH_TOKEN "jlf6l5_AEcJZiU1jO2s_uOHEfWx8Oyig"
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <MFRC522.h>
#include <ESP32Servo.h>

// Blynk authentication and WiFi credentials
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "zproject.wifi";
char pass[] = "source.pyc";

// RFID and sensor pins
#define RFID_SS_PIN 21
#define RFID_RST_PIN 22
#define IR_SENSOR_EXIT_A_PIN 33
#define IR_SENSOR_EXIT_B_PIN 34
#define SERVO_MAIN_PIN 14
#define SERVO_ZONE_A_PIN 13
#define SERVO_ZONE_B_PIN 12
#define LED_MAIN_PIN 15
#define LED_ZONE_A_PIN 2
#define LED_ZONE_B_PIN 4
#define BUZZER_PIN 27

// Initialize RFID reader and servos
MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN);
Servo servoMain, servoZoneA, servoZoneB;

// Zone variables
int zoneACount = 0;
int zoneBCount = 0;
const int maxCapacity = 3;
int prevZoneACount = 0;
int prevZoneBCount = 0;
String rfidTag = "";

void setup() {
  pinMode(IR_SENSOR_EXIT_A_PIN, INPUT_PULLUP);
  pinMode(IR_SENSOR_EXIT_B_PIN, INPUT_PULLUP);
  pinMode(LED_MAIN_PIN, OUTPUT);
  pinMode(LED_ZONE_A_PIN, OUTPUT);
  pinMode(LED_ZONE_B_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(LED_MAIN_PIN, LOW);
  digitalWrite(LED_ZONE_A_PIN, LOW);
  digitalWrite(LED_ZONE_B_PIN, LOW);

  servoMain.attach(SERVO_MAIN_PIN);
  servoZoneA.attach(SERVO_ZONE_A_PIN);
  servoZoneB.attach(SERVO_ZONE_B_PIN);

  Serial.begin(9600);
  delay(10);
  servoMain.write(0);
  servoZoneA.write(0);
  servoZoneB.write(0);

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");

  Blynk.begin(auth, ssid, pass);
  SPI.begin();
  rfid.PCD_Init();

  xTaskCreatePinnedToCore(loop1, "loop1", 4096, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(loop2, "loop2", 4096, NULL, 1, NULL, 0);
}

void loop() {
  // Empty loop since we're using tasks
}

void loop2(void* parameter) {
  for (;;) {
    readRFIDTags();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void loop1(void* parameter) {
  for (;;) {
    manageGates();
    manageSensors();
    updateServerWithZoneCounts();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void readRFIDTags() {
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    rfidTag = "";
    for (byte i = 0; i < rfid.uid.size; i++) {
      rfidTag += String(rfid.uid.uidByte[i] < 0x10 ? "0" : "") + String(rfid.uid.uidByte[i], HEX);
    }
    rfidTag.toUpperCase();
    Serial.println("Card No: " + rfidTag);
    rfid.PICC_HaltA();

    if (zoneACount < maxCapacity) {
      openMainGate();
      zoneACount++;
    } else {
      denyAccess();
    }
  }
}

void updateServerWithZoneCounts() {
  if (zoneACount != prevZoneACount || zoneBCount != prevZoneBCount) {
    Blynk.run();
    Blynk.virtualWrite(V0, zoneACount);
    Blynk.virtualWrite(V1, zoneBCount);
    Blynk.virtualWrite(V2, zoneACount >= maxCapacity);
    Blynk.virtualWrite(V3, zoneBCount >= maxCapacity);
    prevZoneACount = zoneACount;
    prevZoneBCount = zoneBCount;
  }
}

void manageGates() {
  manageMainGate();
  manageZoneGates();
}

void manageMainGate() {
   if (zoneACount >= maxCapacity && zoneBCount >= maxCapacity) {
    servoMain.write(0);
    digitalWrite(LED_MAIN_PIN, HIGH);
  } else {
    servoMain.write(90);
    digitalWrite(LED_MAIN_PIN, LOW);
  }
}

void manageZoneGates() {
  servoZoneA.write(zoneACount >= maxCapacity ? 0 : 90);
  digitalWrite(LED_ZONE_A_PIN, zoneACount >= maxCapacity); 
 
  servoZoneB.write(zoneBCount >= maxCapacity ? 0 : 90);
  digitalWrite(LED_ZONE_B_PIN, zoneBCount >= maxCapacity);
}

void manageSensors() {
  if (digitalRead(IR_SENSOR_EXIT_A_PIN) == LOW) {
    zoneACount = max(0, zoneACount - 1);
    zoneBCount = min(maxCapacity, zoneBCount + 1);
    buzz();
  }
  if (digitalRead(IR_SENSOR_EXIT_B_PIN) == LOW) {
    zoneBCount = max(0, zoneBCount - 1);
    buzz();
  }
}

void openMainGate() {
  servoMain.write(90);
  buzz();
  servoMain.write(0);
}

void denyAccess() {
  servoMain.write(0);
  Serial.println("Both zones are full, access denied.");
  digitalWrite(LED_MAIN_PIN, HIGH);
  delay(300);
  digitalWrite(LED_MAIN_PIN, LOW);
}

void buzz() {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(100);
  digitalWrite(BUZZER_PIN, LOW);
}
