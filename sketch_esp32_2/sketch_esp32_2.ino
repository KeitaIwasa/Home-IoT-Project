#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>

// DHTセンサーの設定
#define DHTPIN 14     // ESP32の接続ピン
#define DHTTYPE DHT11 // DHT 11を使用
DHT dht(DHTPIN, DHTTYPE);


const char* ssid = "aterm-4d09d9-g";
const char* password = "1fde8dda40d72";
const char* serverName = "https://home-iot-project-413811.an.r.appspot.com/data";


void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  dht.begin();
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    // センサーの読み取りが成功しているか確認
    if (isnan(h) || isnan(t)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    HTTPClient http;

    http.begin(serverName);
    http.addHeader("Content-Type", "application/json");

    // JSONデータの作成
    String httpRequestData = "{\"temperature\":" + String(t) + ", \"humidity\":" + String(h) + "}";
    
    int httpResponseCode = http.POST(httpRequestData);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.print(httpResponseCode);
      Serial.print(response);
      Serial.println(httpRequestData);
    } else {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("Error in WiFi connection");
  }

  delay(60000); // １分ごとにデータを送信
}

