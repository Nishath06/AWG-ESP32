#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <WiFiManager.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define MQTT_MAX_PACKET_SIZE 512

// ================= NEW ADDITIONS =================
#define ONE_WIRE_BUS 2
#define RELAY_PIN 16
#define MANUAL_SWITCH 17

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature heaterSensor(&oneWire);

float lowerThreshold = 68.0;
float upperThreshold = 70.0;

// 🔥 Timeout
unsigned long heaterStartTime = 0;
bool heaterRunning = false;
const unsigned long heaterMaxDuration = 90UL * 60UL * 1000UL;

// ================= AWS =================
const char* mqtt_server = "a126j8eka03lhs-ats.iot.ap-south-1.amazonaws.com";

// ================= CERTIFICATES =================
// (UNCHANGED — KEEP YOUR ORIGINAL CERTS EXACTLY SAME)
static const char root_ca[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
...YOUR CERT...
-----END CERTIFICATE-----
)EOF";

static const char client_cert[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----
...YOUR CERT...
-----END CERTIFICATE-----
)KEY";

static const char private_key[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
...YOUR KEY...
-----END RSA PRIVATE KEY-----
)KEY";

// ================= EXISTING PINS =================
#define DHTPIN 15
#define DHTTYPE DHT11
#define WATER_PIN 34
#define TDS_PIN 35

#define S0 14
#define S1 27
#define S2 26
#define S3 25
#define sensorOut 33

#define RESET_BUTTON 4

DHT dht(DHTPIN, DHTTYPE);
WiFiClientSecure net;
PubSubClient client(net);

// ================= TIMING =================
unsigned long lastSend = 0;
const int interval = 5000;

unsigned long lastAWSAttempt = 0;
const int awsRetryInterval = 5000;

unsigned long pressStart = 0;
bool pressed = false;

unsigned long lastHeaterRead = 0;

// ================= RGB =================
int redFreq, greenFreq, blueFreq;

// ================= WIFI =================
void setupWiFi() {
  WiFiManager wm;
  if (!wm.autoConnect("ESP32-Setup", "12345678")) {
    ESP.restart();
  }
}

// ================= RESET =================
void checkResetButton() {
  if (digitalRead(RESET_BUTTON) == LOW) {
    if (!pressed) {
      pressed = true;
      pressStart = millis();
    }
    if (millis() - pressStart > 2000) {
      WiFi.disconnect(true);
      WiFiManager wm;
      wm.resetSettings();
      ESP.restart();
    }
  } else pressed = false;
}

// ================= AWS =================
void connectAWS() {
  if (client.connected()) return;
  if (millis() - lastAWSAttempt < awsRetryInterval) return;

  lastAWSAttempt = millis();

  net.setCACert(root_ca);
  net.setCertificate(client_cert);
  net.setPrivateKey(private_key);
  client.setServer(mqtt_server, 8883);

  unsigned long start = millis();

  while (!client.connected() && millis() - start < 3000) {
    checkResetButton();
    if (client.connect("ESP32_Client")) break;
    delay(200);
  }
}

// ================= RGB =================
int readColor(bool s2, bool s3) {
  digitalWrite(S2, s2);
  digitalWrite(S3, s3);
  delay(50);
  return pulseIn(sensorOut, LOW);
}

void readRGB() {
  redFreq = readColor(LOW, LOW);
  greenFreq = readColor(HIGH, HIGH);
  blueFreq = readColor(LOW, HIGH);
}

// ================= HEATER CONTROL =================
void controlHeater() {

  if (millis() - lastHeaterRead < 2000) return;
  lastHeaterRead = millis();

  bool switchState = digitalRead(MANUAL_SWITCH) == LOW;

  if (!switchState) {
    digitalWrite(RELAY_PIN, LOW);
    heaterRunning = false;
    return;
  }

  heaterSensor.requestTemperatures();
  float temp = heaterSensor.getTempCByIndex(0);

  if (temp == DEVICE_DISCONNECTED_C || temp < -50 || temp > 150) {
    digitalWrite(RELAY_PIN, LOW);
    heaterRunning = false;
    return;
  }

  readRGB();

  // 🎨 Orange gradient detection
  bool isOrange =
    (redFreq < 120) &&
    (greenFreq > 100 && greenFreq < 300) &&
    (blueFreq > 200);

  if (isOrange) {
    digitalWrite(RELAY_PIN, LOW);
    heaterRunning = false;
    return;
  }

  // ⏱ Start timer
  if (!heaterRunning && temp < lowerThreshold) {
    heaterStartTime = millis();
    heaterRunning = true;
  }

  // ⏱ Timeout
  if (heaterRunning && millis() - heaterStartTime > heaterMaxDuration) {
    digitalWrite(RELAY_PIN, LOW);
    heaterRunning = false;
    return;
  }

  if (temp < lowerThreshold) {
    digitalWrite(RELAY_PIN, HIGH);
  } else if (temp >= upperThreshold) {
    digitalWrite(RELAY_PIN, LOW);
    heaterRunning = false;
  }
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  pinMode(RESET_BUTTON, INPUT_PULLUP);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(MANUAL_SWITCH, INPUT_PULLUP);

  digitalWrite(RELAY_PIN, LOW);

  heaterSensor.begin();
  dht.begin();

  setupWiFi();

  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(sensorOut, INPUT);

  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);
}

// ================= LOOP =================
void loop() {

  for (int i = 0; i < 10; i++) {
    checkResetButton();
    delay(5);
  }

  controlHeater();

  connectAWS();
  client.loop();

  if (millis() - lastSend > interval) {
    lastSend = millis();

    float temp = dht.readTemperature();
    float hum = dht.readHumidity();
    int water = analogRead(WATER_PIN);
    int tds = analogRead(TDS_PIN);

    readRGB();

    if (isnan(temp) || isnan(hum)) return;

    char payload[300];

    snprintf(payload, sizeof(payload),
      "{"
      "\"temperature\":%.2f,"
      "\"humidity\":%.2f,"
      "\"water_level\":%d,"
      "\"tds\":%d,"
      "\"red\":%d,"
      "\"green\":%d,"
      "\"blue\":%d"
      "}",
      temp, hum, water, tds,
      redFreq, greenFreq, blueFreq
    );

    if (client.connected()) {
      client.publish("esp32/data", payload);
    }
  }
}