#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <Wire.h>

// I2Cディスプレイの設定
#define LCD_ADRS 0x3E

// DHTセンサーの設定
#define DHTPIN 14     // ESP32の接続ピン
#define DHTTYPE DHT11 // DHT 11を使用
DHT dht(DHTPIN, DHTTYPE);

String LCD_STRING; char s[16]; double SET; //変数の宣言

const char* ssid = "aterm-4d09d9-g";
const char* password = "1fde8dda40d72";
const char* serverName = "https://home-iot-project-413811.an.r.appspot.com/data";


// LCDコマンド書き込み
void writeCommand(byte command) {
  Wire.beginTransmission(0x3E);
  Wire.write(0x00);
  Wire.write(command);
  Wire.endTransmission(); delay(10);
}
// LCDデータ書き込み
void writeData(byte data) {
  Wire.beginTransmission(0x3E);
  Wire.write(0x40);
  Wire.write(data);
  Wire.endTransmission(); delay(1);
}
//LCDの1行目にデータ書き込み
void LCD_DISP_16_1(void) {
  writeCommand(0x80);//DDRAMアドレスを2行目先頭にセット
  for (int i = 0; i < 16; i++) {
    writeData(LCD_STRING[i]);
  }
}
//LCDの2行目にデータ書き込み
void LCD_DISP_16_2(void) {
  writeCommand(0x40 + 0x80); //DDRAMアドレスを2行目先頭にセット
  for (int i = 0; i < 16; i++) {
    writeData(LCD_STRING[i]);
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  dht.begin();
  
  //LCD液晶初期化
  Wire.begin(); delay(100);
  writeCommand(0x38); delay(20);
  writeCommand(0x39); delay(20);
  writeCommand(0x14); delay(20);
  writeCommand(0x7A); delay(20); //3.3V=0x73, 5V=0x7A
  writeCommand(0x54); delay(20); //3.3V=0x56, 5V=0x54
  writeCommand(0x6C); delay(20);
  writeCommand(0x38); delay(20);
  writeCommand(0x01); delay(20);
  writeCommand(0x0C); delay(20);
}

void loop() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float rounded_h = round(h*10.0)/10.0;
  float rounded_t = round(t*10.0)/10.0;
  LCD_STRING = String("T: " + String(rounded_t, 1) + " C"); //乱数代入
  LCD_DISP_16_1();                //LCD一行目に変数を出力する
  LCD_STRING = String("H: " + String(rounded_h, 1) + " %");//「NobArduinoDiary!」代入
  LCD_DISP_16_2(); //LCD二行目に変数を出力する
  
  if (WiFi.status() == WL_CONNECTED) {

    // センサーの読み取りが成功しているか確認
    if (isnan(h) || isnan(t)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    HTTPClient http;

    http.begin(serverName);
    http.addHeader("Content-Type", "application/json");

    // JSONデータの作成
    String httpRequestData = "{\"temperature\":" + String() + ", \"humidity\":" + String(h) + "}";
    
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

