#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <HX711.h>
#include <ESP32Servo.h>

const char* WIFI_SSID = "YOUR_WIFI";
const char* WIFI_PASSWORD = "YOUR_PASSWORD";

const char* MQTT_SERVER = "broker.hivemq.com";
const int MQTT_PORT = 1883;

WiFiClient espClient;
PubSubClient mqtt(espClient);

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define TRIG_PIN 5
#define ECHO_PIN 18

#define HX711_DT 4
#define HX711_SCK 19

#define MOTOR_IN1 26
#define MOTOR_IN2 27
#define MOTOR_ENA 25

#define PLASTIC_SERVO 13
#define PAPER_SERVO 14
#define METAL_SERVO 23
#define GLASS_SERVO 16
#define WET_SERVO 17
#define SANITARY_SERVO 33

HX711 scale;

Servo plasticServo;
Servo paperServo;
Servo metalServo;
Servo glassServo;
Servo wetServo;
Servo sanitaryServo;

float calibration_factor = 2280.0;
float distance = 0;
float weight = 0;

int fillLevel = 0;

String wasteType = "Plastic";

bool binFull = false;

void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int count = 0;

  while (WiFi.status() != WL_CONNECTED && count < 20) {
    delay(500);
    count++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi Connected");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("WiFi Connection Failed");
  }
}

void connectMQTT() {
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  if (mqtt.connected()) {
    return;
  }

  String clientID = "SmartWasteESP32-";
  clientID += String((uint32_t)ESP.getEfuseMac(), HEX);

  if (mqtt.connect(clientID.c_str())) {
    Serial.println("MQTT Connected");
  } else {
    Serial.println("MQTT Connection Failed");
  }
}

float readFillDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duration == 0) {
    return 30.0;
  }

  return duration * 0.0343 / 2.0;
}

int calculateFillLevel(float d) {
  int level = map((int)d, 30, 5, 0, 100);
  return constrain(level, 0, 100);
}

float readWeight() {
  if (!scale.is_ready()) {
    return 0;
  }

  float value = scale.get_units(5);

  if (value < 0) {
    value = 0;
  }

  return value;
}

void conveyorON() {
  if (binFull) {
    return;
  }

  digitalWrite(MOTOR_IN1, HIGH);
  digitalWrite(MOTOR_IN2, LOW);
  digitalWrite(MOTOR_ENA, HIGH);
}

void conveyorOFF() {
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, LOW);
  digitalWrite(MOTOR_ENA, LOW);
}

void moveServo(Servo &servo) {
  servo.write(90);
  delay(1000);
  servo.write(0);
  delay(500);
}

void segregateWaste(String waste) {
  if (binFull) {
    return;
  }

  conveyorOFF();

  if (waste == "Plastic") {
    moveServo(plasticServo);
  }
  else if (waste == "Paper") {
    moveServo(paperServo);
  }
  else if (waste == "Metal") {
    moveServo(metalServo);
  }
  else if (waste == "Glass") {
    moveServo(glassServo);
  }
  else if (waste == "Wet Waste") {
    moveServo(wetServo);
  }
  else if (waste == "Sanitary Waste") {
    moveServo(sanitaryServo);
  }

  conveyorON();
}

void displayData() {
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print(wasteType);

  lcd.setCursor(0, 1);
  lcd.print("W:");
  lcd.print(weight, 1);
  lcd.print(" F:");
  lcd.print(fillLevel);
  lcd.print("%");
}

void sendCloudData() {
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  if (!mqtt.connected()) {
    connectMQTT();
  }

  if (!mqtt.connected()) {
    return;
  }

  mqtt.loop();

  char message[200];

  snprintf(
    message,
    sizeof(message),
    "{\"waste\":\"%s\",\"weight\":%.2f,\"fill\":%d}",
    wasteType.c_str(),
    weight,
    fillLevel
  );

  mqtt.publish("smartwaste/data", message);
}

void readWasteFromSerial() {
  if (!Serial.available()) {
    return;
  }

  String input = Serial.readStringUntil('\n');
  input.trim();
  input.toLowerCase();

  if (input == "plastic") {
    wasteType = "Plastic";
  }
  else if (input == "paper") {
    wasteType = "Paper";
  }
  else if (input == "metal") {
    wasteType = "Metal";
  }
  else if (input == "glass") {
    wasteType = "Glass";
  }
  else if (input == "wet") {
    wasteType = "Wet Waste";
  }
  else if (input == "sanitary") {
    wasteType = "Sanitary Waste";
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);
  pinMode(MOTOR_ENA, OUTPUT);

  conveyorOFF();

  Wire.begin(21, 22);

  lcd.init();
  lcd.backlight();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SMART WASTE");
  lcd.setCursor(0, 1);
  lcd.print("SYSTEM START");

  delay(2000);

  scale.begin(HX711_DT, HX711_SCK);
  scale.set_scale(calibration_factor);
  delay(500);
  scale.tare();

  plasticServo.attach(PLASTIC_SERVO);
  paperServo.attach(PAPER_SERVO);
  metalServo.attach(METAL_SERVO);
  glassServo.attach(GLASS_SERVO);
  wetServo.attach(WET_SERVO);
  sanitaryServo.attach(SANITARY_SERVO);

  plasticServo.write(0);
  paperServo.write(0);
  metalServo.write(0);
  glassServo.write(0);
  wetServo.write(0);
  sanitaryServo.write(0);

  connectWiFi();

  mqtt.setServer(MQTT_SERVER, MQTT_PORT);

  connectMQTT();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SYSTEM READY");
  lcd.setCursor(0, 1);
  lcd.print("WAITING WASTE");

  delay(2000);

  conveyorON();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  if (!mqtt.connected()) {
    connectMQTT();
  }

  mqtt.loop();

  readWasteFromSerial();

  distance = readFillDistance();
  fillLevel = calculateFillLevel(distance);
  weight = readWeight();

  displayData();

  if (fillLevel >= 90) {
    binFull = true;

    conveyorOFF();

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("BIN FULL");
    lcd.setCursor(0, 1);
    lcd.print("PLEASE EMPTY");

    delay(3000);

    return;
  }

  binFull = false;

  segregateWaste(wasteType);

  sendCloudData();

  Serial.print("Waste: ");
  Serial.println(wasteType);

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  Serial.print("Fill: ");
  Serial.print(fillLevel);
  Serial.println("%");

  Serial.print("Weight: ");
  Serial.println(weight);

  delay(5000);
}
