# Rovion Climate Control (demo)

A small ESP32-S3 sketch for a greenhouse pitch. It is **not** the product firmware.

An ESP32 reads a DHT11 every two seconds and sends temperature, humidity, and Wi-Fi RSSI as a JSON line. The same payload goes out over serial (USB) and, if Wi-Fi is up, over a WebSocket on port 81. `src/rovion_demo_dashboard.html` is a local page that plots those numbers and fakes an alert chain — useful for a live demo, not a control system.

The register-level STM32 node lives in a different repo: [sentinel-node](https://github.com/javokhirt/sentinel-node). That one is the real hardware work. This one is Arduino libraries on an ESP32 so a dashboard can move.

## Hardware

| | |
|---|---|
| Board | Freenove ESP32-S3 WROOM |
| Sensor | DHT11 on GPIO 4 |
| Framework | Arduino, via PlatformIO |

## What the firmware does

1. Connects to Wi-Fi (15 s timeout). If that fails, it still prints JSON on USB.
2. Reads DHT11 every 2 s. A failed read reuses the last good sample.
3. Broadcasts `{"t":…,"h":…,"rssi":…,"seq":…}` on serial and WebSocket.

## Build and run

Needs [PlatformIO](https://platformio.org/).

```sh
pio run -t upload
pio device monitor -b 115200
```

Open `src/rovion_demo_dashboard.html` in a browser. If the board joined Wi-Fi, put its IP in the page settings (serial prints `IP MANZIL`). USB-only still works for watching the JSON.

Put SSID and password in `src/main.cpp` on your machine. Do not commit real credentials.

## What this is not

- No register-level drivers, no CRC, no host tests, no CI
- DHT11 accuracy is what it is
- The dashboard alert / call chain is UI, not a phone system
- `include/`, `lib/`, and `test/` are stock PlatformIO stubs

Built as a quick demo for [Rovion Controls](https://www.rovioncontrols.com/en).
