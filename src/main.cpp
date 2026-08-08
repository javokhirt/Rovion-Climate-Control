#include <Arduino.h>

#include <WiFi.h>
#include <WebSocketsServer.h>
#include <DHT.h>


const char* WIFI_SSID = "Meniki PC";     
const char* WIFI_PASS = "12344321";        

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


  if (millis() - lastRead >= 2000) {
    lastRead = millis();

    float t = dht.readTemperature();
    float h = dht.readHumidity();


    if (isnan(t)) t = lastT;
    if (isnan(h)) h = lastH;
    if (isnan(t)) return;          
    lastT = t; lastH = h;

    long rssi = (WiFi.status() == WL_CONNECTED) ? WiFi.RSSI() : 0;

    char buf[120];
    snprintf(buf, sizeof(buf),
             "{\"t\":%.1f,\"h\":%.0f,\"rssi\":%ld,\"seq\":%lu}",
             t, h, rssi, (unsigned long)seq++);

    Serial.println(buf);      
    ws.broadcastTXT(buf);
  }
}
