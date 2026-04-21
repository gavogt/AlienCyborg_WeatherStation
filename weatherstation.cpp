#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>

#include <Adafruit_BMP280.h>

#include <SensirionI2cScd4x.h>
#include <SensirionCore.h>

#include <DHT20.h>   // install "DHT20" by Rob Tillaart

static const int I2C_SDA = 8;
static const int I2C_SCL = 9;
static const int RAIN_DO = 16;
static const bool RAIN_ACTIVE_LOW = true;

const char* WIFI_SSID = "xxx";
const char* WIFI_PASS = "xxx";
const char* MQTT_HOST = "192.168.x.xxx";
const uint16_t MQTT_PORT = 1883;
const char* MQTT_TOPIC = "weatherstation/telemetry";

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

Adafruit_BMP280 bmp;
SensirionI2cScd4x scd4x;
DHT20 dht;

bool bmpOk = false, scdOk = false, dhtOk = false;

struct Reading {
    unsigned long tsMs = 0;
    bool rainWet = false;

    float thTempC = NAN;
    float thRH = NAN;

    float bmpTempC = NAN;
    float pressure_hPa = NAN;

    uint16_t co2ppm = 0;
    float scdTempC = NAN;
    float scdRH = NAN;
} g;

void wifiConnect() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) delay(250);
}

void mqttConnect() {
    while (!mqtt.connected()) {
        String id = "esp32s3-weather-";
        id += String((uint32_t)ESP.getEfuseMac(), HEX);
        mqtt.connect(id.c_str());
        delay(300);
    }
}

void readRain() {
    int v = digitalRead(RAIN_DO);
    g.rainWet = RAIN_ACTIVE_LOW ? (v == LOW) : (v == HIGH);
}

void readDHT20() {
    if (!dhtOk) return;
    int s = dht.read();
    if (s != 0) return;
    float t = dht.getTemperature();
    float rh = dht.getHumidity();
    if (!isnan(t) && !isnan(rh) && rh >= 0 && rh <= 100) {
        g.thTempC = t;
        g.thRH = rh;
    }
}

void readBMP() {
    if (!bmpOk) return;
    float t = bmp.readTemperature();
    float p = bmp.readPressure() / 100.0f;
    if (!isnan(t) && t > -40 && t < 85) g.bmpTempC = t;
    if (!isnan(p) && p > 200 && p < 1200) g.pressure_hPa = p;
}

void readSCD41() {
    if (!scdOk) return;
    bool ready = false;
    int16_t err = scd4x.getDataReadyStatus(ready);
    if (err != 0 || !ready) return;

    uint16_t co2 = 0; float t = NAN, rh = NAN;
    err = scd4x.readMeasurement(co2, t, rh);
    if (err != 0 || co2 == 0) return;

    g.co2ppm = co2;
    g.scdTempC = t;
    g.scdRH = rh;
}

String makeJson() {
    String j = "{";
    j += "\"tsMs\":" + String(g.tsMs) + ",";
    j += "\"rainWet\":" + (String)(g.rainWet ? "true" : "false") + ",";
    j += "\"bmp\":{\"addr\":\"0x77\",\"pressure_hPa\":" + String(g.pressure_hPa, 2) + "},";
    j += "\"scd41\":{\"co2ppm\":" + String(g.co2ppm) + ",\"tempC\":" + String(g.scdTempC, 2) + ",\"rh\":" + String(g.scdRH, 2) + "}";
    j += "}";
    return j;
}

void setup() {
    Serial.begin(115200);
    pinMode(RAIN_DO, INPUT_PULLUP);

    Wire.begin(I2C_SDA, I2C_SCL);
    Wire.setClock(25000);

    // DHT20 (0x38)
    dhtOk = dht.begin();
    Serial.println(dhtOk ? "DHT20 OK" : "DHT20 FAIL");

    // BMP280 (0x77)
    bmpOk = bmp.begin(0x77);
    Serial.println(bmpOk ? "BMP OK" : "BMP FAIL");
    if (bmpOk) {
        bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
            Adafruit_BMP280::SAMPLING_X2,
            Adafruit_BMP280::SAMPLING_X16,
            Adafruit_BMP280::FILTER_X16,
            Adafruit_BMP280::STANDBY_MS_500);
    }

    // SCD41 (0x62)
    scd4x.begin(Wire, 0x62);
    scd4x.wakeUp();
    delay(20);
    scd4x.stopPeriodicMeasurement();
    delay(100);
    scdOk = (scd4x.startPeriodicMeasurement() == 0);
    Serial.println(scdOk ? "SCD41 OK" : "SCD41 FAIL");
    delay(5000);

    wifiConnect();
    mqtt.setServer(MQTT_HOST, MQTT_PORT);
    mqttConnect();
}

unsigned long lastSend = 0;

void loop() {
    if (!mqtt.connected()) mqttConnect();
    mqtt.loop();

    unsigned long now = millis();
    if (now - lastSend >= 5000) {
        lastSend = now;
        g.tsMs = now;

        readRain();
        readDHT20();
        readBMP();
        readSCD41();

        String payload = makeJson();
        Serial.println(payload);
        mqtt.publish(MQTT_TOPIC, payload.c_str());
    }
}