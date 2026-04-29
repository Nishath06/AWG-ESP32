#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <WiFiManager.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define MQTT_MAX_PACKET_SIZE 512

// ================= TEST OLED =================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

unsigned long lastDisplayUpdate = 0;
int displayState = 0;

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
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy
rqXRfboQnoZsG4q5WTP468SQvvG5
-----END CERTIFICATE-----
)EOF";

static const char client_cert[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----
MIIDWTCCAkGgAwIBAgIUQ9rIde4d7QF4t7YVYQYrfHmuTxEwDQYJKoZIhvcNAQEL
BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g
SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTI2MDMyNjA2MjUy
M1oXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0
ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAKd0c5a1R5BPlrkCajLn
wCAa94j5JV3ZEZ7b1DAZ325r+Nn5DBI+KvGCIgWPdAIGe8bSR5jRI6O+Troyru2N
o+Gb+Ucmk6ucX+t65lAxB985ESRTQcblQgnkeLnPJJuj/4alh24RoL9tL0sgV/IR
YVj1SgUv3fSslPkPAjqv2KhEojCBOQfXfuu1bcl2UMc43fQLoJ36M+3CLcmRCu/C
qnTyVQ0AElsNoeM7fyLJ9jLkByalZM3E6ui1E3I7ci8/JG98yqOZZB8uksvxu2Yp
dkHl8A7XMcn4epEWl0D4CvAmswPSe0ejqr9DkTfU5tU+YdT18fCcforZrUCQM4aC
h8cCAwEAAaNgMF4wHwYDVR0jBBgwFoAU4G3mXf6sGcwDGUQ5XxvB+5cnhaEwHQYD
VR0OBBYEFNHiVaZNfhR2j2gSfZNZ7oys1jIQMAwGA1UdEwEB/wQCMAAwDgYDVR0P
AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQCMwVGihKs81jlo/7zC/8DgzZVE
jC4Lon3edwAUcrLLtOlRiafTCi7B8dpXObgaBKkLn5JN15DTdwAaoyaS5mTpasiB
J+5htBBeAW40Y3RbeNT3hvFYjUpy8PuaxHI7UHXwxtm+W5H0XvHMp9L/kewRTuaJ
2QVoJwwYyyxcepKIYbNU58F3TshwrkdBha5qwB3rfOaCheRMrTk8EEuvFV5TpVd5
L+1qk+qvI4jVj/l23lyVAmz1U1WUuUNiMcSCXnxlD97OoPrtvT9CKSkq5Eo48X5x
JP5pOkHaBoGFwQfFy7OZ29udTfcpahROfwM6hOQdByz3j7u0A0qXq6uJ2rko
-----END CERTIFICATE-----
)KEY";

static const char private_key[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
MIIEowIBAAKCAQEAp3RzlrVHkE+WuQJqMufAIBr3iPklXdkRntvUMBnfbmv42fkM
Ej4q8YIiBY90AgZ7xtJHmNEjo75OujKu7Y2j4Zv5RyaTq5xf63rmUDEH3zkRJFNB
xuVCCeR4uc8km6P/hqWHbhGgv20vSyBX8hFhWPVKBS/d9KyU+Q8COq/YqESiMIE5
B9d+67VtyXZQxzjd9Augnfoz7cItyZEK78KqdPJVDQASWw2h4zt/Isn2MuQHJqVk
zcTq6LUTcjtyLz8kb3zKo5lkHy6Sy/G7Zil2QeXwDtcxyfh6kRaXQPgK8CazA9J7
R6Oqv0ORN9Tm1T5h1PXx8Jx+itmtQJAzhoKHxwIDAQABAoIBABv5NCYE+oN9CCy3
3o3KcSnr11YmaLuYu3zSNEwkxlJn7xZaqE2T3RCjMJPFNxfvhVXdYuCAb3e3Huez
98zvUGUr7SSZqSmN4egwi7Dxv+LmYZqz3dUgDzwf0psXo4Hsj48qr8kPYM8ZqkDe
klJ53M1DjK35JRbujaRdgX3USlkxeL10/c9KroBjo6hyN9vAbkh2ofYMbBejBpCl
Hp+/g8IZO3Ja2JRSJkiOsbWl/KjGmKiq2Jx10JTvSdITKBVOfgIGoi3UK0C5Ws9B
d/l1Y9mevBDlMur3VHE4LEZcpl89Q55a9435SmSUVQHWMXjeVNMA0frp5qcXrk5K
PlHL+SECgYEA1uHl8QmGcNuKb05JT1+rmpEwFhfUhM/apbJEhHd7fbA6CzbAfIsM
uAcaDTnUv401ybghzvB/Lh1deJXbaWrN8IMkx9+DbFMa20O6SnjwlP5B74XiqhhT
4HgcCvHI1oNOVXabrTvSfodLHByhnAA0elBP6GoHJXVxrVRBRA0DmysCgYEAx39L
+3oMo+zDK9e6y2PNwOL5+eftU7vPEWe+pxezr02JHJOZZuDehqbLP3zXOprBMZH5
gC5PEQDYnbp+SdsEWKtqtm6incwoBkXNNJ923JxX06XTzD/yn1KUp9egpKrduTkR
QOH88Ah8TU3SCLnrL5nXljSmtWClUx8Vfeysx9UCgYEAoUVVL/v4HIeIdFULDDk/
L3g74Ljt1Oeyr50Fc23BDunOBlCHUnGBkDQ0RyzaERFIBybcqNUyfkenuTKvQ4TX
HuUuG5wSpKASjfFcxTvTQ983WS7gmMw9V0CNu2EPXH+bsOSdnfxB1JhxtJ2AZPZ6
bDoOXt4EkwZ71Jxhh1NFG5UCgYAo/0QCHgbS+SjyrZa5jX0PEpYEgWEIQCRBk2xe
skQ+1lRDByITPkr8qPJDzcf9RyhEo5b+fIXLHL1nTJf/XX77Kd74Nqw8eWQQRe/x
zIOEuqxh1kY8Rzf4/eOGyo08ydBe7BdbmhV8zoGc8U0kdfCbC6Z+O7u2snHHLzAf
fK/VMQKBgBSfaVxFBgkKW4qEmgvuj47WxPkQ8uVxYBCMGMjXAOx/w4WDLmZtKsQj
UTQSGIAZB+W0rpaIErosGXd47B3B11B0tgwBTUwf3vUvgE4txrcsh/Gj7Yn3pHpA
nCofEC46Y7c9s9IiqVp2PnL1q17UvmCFxQbYwAW1bDFgWSd6we2F
-----END RSA PRIVATE KEY-----
)KEY";

// ================= EXISTING PINS =================
#define DHTPIN 15
#define DHTTYPE DHT11
#define WATER_PIN 35
#define TDS_PIN 34

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
  pinMode(WATER_PIN, INPUT);
  pinMode(TDS_PIN, INPUT);

  digitalWrite(RELAY_PIN, LOW);

  heaterSensor.begin();
  dht.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found");
  } else {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(10, 20);
    display.println("Public AWG");
    display.display();
    delay(2000);
  }

  setupWiFi();

  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(sensorOut, INPUT);

  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);
}

// ================= DISPLAY FUNCTION =================
void displaySequence() {
  if (millis() - lastDisplayUpdate < 2000) return;
  lastDisplayUpdate = millis();

  float temperature = heaterSensor.getTempCByIndex(0);
  if (temperature == DEVICE_DISCONNECTED_C) temperature = 0.0;
  
  float humidity = dht.readHumidity();
  if (isnan(humidity)) humidity = 0.0;

  int rawLevel = analogRead(WATER_PIN);
  Serial.print("Raw Water Level: ");
  Serial.println(rawLevel);

  float waterLevel = map(rawLevel, 1200, 3000, 0, 100);
  if (waterLevel < 0.0) waterLevel = 0.0;
  if (waterLevel > 100.0) waterLevel = 100.0;

  int analogValue = analogRead(TDS_PIN);
  float voltage = analogValue * (3.3 / 4095.0);
  float tdsValue = (133.42 * voltage * voltage * voltage 
            - 255.86 * voltage * voltage 
            + 857.39 * voltage) * 0.5;

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  if (displayState == 0) {
    display.setCursor(0, 20);
    display.print("Temp: ");
    display.print(temperature);
    display.println(" C");
  } else if (displayState == 1) {
    display.setCursor(0, 20);
    display.print("Humidity: ");
    display.print(humidity);
    display.println(" %");
  } else if (displayState == 2) {
    display.setCursor(0, 20);
    display.print("Level: ");
    display.print(waterLevel);
    display.println(" %");
  } else if (displayState == 3) {
    display.setCursor(0, 20);
    display.print("TDS: ");
    display.print(tdsValue);
    display.println(" ppm");
  } else if (displayState == 4) {
    display.setCursor(0, 20);
    if (tdsValue <= 300) {
      display.println("Water SAFE");
    } else {
      display.println("NOT SAFE!");
    }
  }
  
  display.display();

  displayState++;
  if (displayState > 4) displayState = 0;
}

// ================= LOOP =================
void loop() {

  for (int i = 0; i < 10; i++) {
    checkResetButton();
    delay(5);
  }

  controlHeater();
  displaySequence();

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