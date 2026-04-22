# ESP32-S3 MQTT Weather Station (SCD41 + BMP280 + Rain)

A small, reliable ESP32‑S3 weather / air‑quality node that publishes telemetry to an MQTT broker as JSON.

This project intentionally **does not rely on the AHT/DHT portion** of the common purple “AHT20+BMP280” combo boards because those modules are often inconsistent. Instead, it uses:

- **Sensirion SCD41** for **CO₂ + temperature + humidity**
- **Bosch BMP280** for **barometric pressure** (temperature is read internally for pressure compensation)
- **LM393 rain module** for **wet/dry** (digital output)

---

## Hardware

### Required parts

- ESP32‑S3 dev board (Arduino‑compatible)
- **SCD41** breakout (I²C, address `0x62`)
- **BMP280** breakout (I²C, commonly `0x76` or `0x77`)
- **Rain sensor**: LM393 module + rain plate (digital output)

### Recommended for stability

- **0.1µF (104) ceramic capacitor** across **VCC ↔ GND** at **each** sensor module (place physically close to sensor pins)
- Optional: **10µF** near the SCD41 module
- Keep power wiring short; SCD41 is sensitive to voltage drop on VCC.

> **Field note:** If SCD41 “ACKs” on I²C scan but fails on `startPeriodicMeasurement()`, the most common root cause is a high‑resistance power path (long duponts / breadboard rail hops). A short direct 3.3V feed often fixes it.

---

## Pin mapping (ESP32‑S3)

| Function | ESP32‑S3 GPIO | Notes |
|---|---:|---|
| I²C SDA | **GPIO 8** | Shared by SCD41 + BMP280 |
| I²C SCL | **GPIO 9** | Shared by SCD41 + BMP280 |
| Rain DO | **GPIO 16** | LM393 DO output |
| 3V3 | 3V3 pin | Power for sensors (3.3V) |
| GND | GND pin | Common ground |

---

## Wiring

### SCD41 (I²C)

- VCC → 3V3  
- GND → GND  
- SDA → GPIO 8  
- SCL → GPIO 9  

### BMP280 (I²C)

- VCC → 3V3  
- GND → GND  
- SDA → GPIO 8  
- SCL → GPIO 9  

### Rain (LM393 module)

- VCC → 3V3  
- GND → GND  
- DO → GPIO 16  
- Rain plate → LM393 module sensor connector (+/− polarity doesn’t matter for the plate)

---

## MQTT

- **Broker:** `192.168.x.xxx`
- **Port:** `1883`
- **Topic:** `weatherstation/telemetry`

Telemetry is published about every **5 seconds** as JSON.

### JSON payload (current)

```json
{
  "tsMs": 12345,
  "rainWet": false,
  "bmp": {
    "addr": "0x77",
    "pressure_hPa": 795.88
  },
  "scd41": {
    "co2ppm": 701,
    "tempC": 29.56,
    "rh": 35.16
  }
}
```

**Notes**
- `pressure_hPa` comes from **BMP280**
- `co2ppm`, `tempC`, `rh` come from **SCD41**
- BMP temperature is read internally (for proper pressure compensation) but is **not** emitted in JSON

---

## Software

### Tooling

- Arduino IDE (1.8.x or 2.x) or PlatformIO
- Board: ESP32‑S3 (Arduino core for ESP32)

### Required libraries (Arduino Library Manager)

Install these from **Sketch → Include Library → Manage Libraries…**

- **PubSubClient** (Nick O’Leary) — MQTT client
- **Adafruit BMP280 Library**
- **Adafruit Unified Sensor** (dependency)
- **Adafruit BusIO** (dependency)
- **Sensirion I2C SCD4x** (SCD41 driver; provides `SensirionI2cScd4x.h` and `SensirionCore.h`)

---

## Firmware configuration

Edit these constants in the sketch:

- `WIFI_SSID`, `WIFI_PASS`
- `MQTT_HOST` (default `192.168.x.xxx`)
- `MQTT_PORT` (default `1883`)
- `MQTT_TOPIC` (default `weatherstation/telemetry`)

I²C configuration (this build):

- `Wire.begin(8, 9);`
- `Wire.setClock(25000);` (25 kHz — chosen for breadboard tolerance)

---

## Troubleshooting

### “I²C scan shows devices, but reads return NaN / 0”

An I²C scan only checks address ACK; real sensor commands are longer and more sensitive.

Try:

- Add **0.1µF (104)** at each sensor VCC/GND
- Shorten VCC/GND wires to sensors (especially SCD41)
- Ensure **common ground** across all modules
- Keep I²C clock at 25 kHz while prototyping

### SCD41 fails `startPeriodicMeasurement()` / returns errors

Most often power integrity:

- Feed SCD41 **3.3V directly** (avoid multiple rail hops)
- Add 0.1µF + 10µF near SCD41
- Confirm SDA/SCL are on GPIO 8/9 (not USB pins)

### BMP280 pressure reads but temperature is NaN

Usually indicates unstable I²C reads or a flaky module:

- Power BMP280 with a short VCC/GND branch
- Add a 0.1µF cap at the BMP280 module
- Verify chip ID register `0xD0` reads `0x58` (BMP280)
- Try both addresses: `0x77` and `0x76` if needed

### Rain reads inverted (wet vs dry)

Flip `RAIN_ACTIVE_LOW`:

- `true` if DO is LOW when wet
- `false` if DO is HIGH when wet

---

## Next steps

- Build a .NET MQTT subscriber (MQTTnet) to consume `weatherstation/telemetry`
- Store telemetry and chart it in your dashboard
- Add thresholds/alerts (CO₂, rain events, pressure drops)

---

## License

Choose your preferred license (MIT is common for small embedded projects).
