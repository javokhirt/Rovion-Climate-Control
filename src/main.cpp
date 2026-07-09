#include <Arduino.h>

/*
 * ROVION DEMO NODE — ESP32 + DHT11
 * Streams JSON over BOTH WiFi (WebSocket server, port 81) and USB Serial.
 * The dashboard (rovion_demo_dashboard.html) accepts either — WiFi is the
 * primary "simsiz" demo path, USB is the bulletproof fallback.
 *
 * LIBRARIES (Arduino IDE -> Library Manager):
 *   1. "DHT sensor library" by Adafruit  (+ auto-installs "Adafruit Unified Sensor")
 *   2. "WebSockets" by Markus Sattler (links2004)
 *
 * WIRING (DHT11 module with 3 pins):
 *   DHT11 VCC  -> 3V3
 *   DHT11 GND  -> GND
 *   DHT11 DATA -> GPIO 4   (bare 4-pin sensor: add 10k pullup DATA->3V3)
 *
 * SETUP:
 *   1. Turn on your PHONE HOTSPOT, put its name/password below.
 *   2. Upload, open Serial Monitor @115200 — it prints the IP address.
 *   3. Connect the laptop to the SAME hotspot.
 *   4. In the dashboard: Sozlamalar -> enter the IP -> "WiFi orqali ulanish".
 *   Fallback: just click "USB orqali ulanish" with the cable attached.
 */

#include <WiFi.h>
#include <WebSocketsServer.h>
#include <DHT.h>

// ======= EDIT THESE TWO LINES =======
const char* WIFI_SSID = "Javohir-iPhone";     // your phone hotspot name
const char* WIFI_PASS = "rovion2026";         // hotspot password
// ====================================

#define DHTPIN  4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
WebSocketsServer ws = WebSocketsServer(81);

unsigned long lastRead = 0;
uint32_t seq = 0;
float lastT = NAN, lastH = NAN;

void onWsEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t len) {
  if (type == WStype_CONNECTED) {
    Serial.printf("# dashboard connected (client %u)\n", num);
  }
}

void setup() {
  Serial.begin(115200);
  delay(300);
  dht.begin();

  Serial.println();
  Serial.println("# ROVION demo node boshlanyapti...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  // Try WiFi for 15s; if it fails, USB-serial mode still works fine.
  unsigned long t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 15000) {
    delay(400);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("# WiFi ULANDI ✓");
    Serial.print("# IP MANZIL (dashboardga kiriting): ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("# WiFi ulanmadi — USB rejimida davom etamiz (bu ham yetarli).");
  }

  ws.begin();
  ws.onEvent(onWsEvent);
}

void loop() {
  ws.loop();

  // DHT11 minimum interval ~2s
  if (millis() - lastRead >= 2000) {
    lastRead = millis();

    float t = dht.readTemperature();
    float h = dht.readHumidity();

    // DHT11 occasionally returns NaN — reuse last good value once
    if (isnan(t)) t = lastT;
    if (isnan(h)) h = lastH;
    if (isnan(t)) return;          // no good data yet
    lastT = t; lastH = h;

    long rssi = (WiFi.status() == WL_CONNECTED) ? WiFi.RSSI() : 0;

    char buf[120];
    snprintf(buf, sizeof(buf),
             "{\"t\":%.1f,\"h\":%.0f,\"rssi\":%ld,\"seq\":%lu}",
             t, h, rssi, (unsigned long)seq++);

    Serial.println(buf);      // USB path (Web Serial in Chrome)
    ws.broadcastTXT(buf);     // WiFi path (WebSocket)
  }
}
