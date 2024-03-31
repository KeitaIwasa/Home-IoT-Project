#include <SoftwareSerial.h> //EspSoftwareSerialライブラリ必須
#include <esp_now.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PN532_SWHSU.h>
#include <PN532.h>
SoftwareSerial SWSerial( 32, 33 ); // PN532のTX, RX
PN532_SWHSU pn532swhsu( SWSerial );
PN532 nfc( pn532swhsu );

// LINE Notify設定
const char* wifi_ssid = "aterm-4d09d9-g";
const char* wifi_password = "1fde8dda40d72";
const char* host = "notify-api.line.me";
const char* token = "xwFC4cARiGCzJ21aptrhKwuVd7WLVSR367PS8Vbhks7";
const char* low_battery_message = "⚠️スマートロックの電池残量が少なくなっています⚠️電池(単三×2)を交換してください";

// Replace with the MAC address of the receiver ESP32
uint8_t receiverMacAddress[] = {0x24, 0xDC, 0xC3, 0x46, 0xB7, 0x64};
// Define the IDm of the specific FeliCa card you want to detect
const uint8_t SPECIFIC_CARD_IDM[] = {0x01, 0x40, 0x8A, 0x8F, 0x2E, 0x37, 0xB6, 0x60};

typedef struct {
  bool cardDetected;
} DataPacket;
DataPacket packet;

RTC_DATA_ATTR int bootCount = 0; // ディープスリープ時にも保持できる変数

esp_now_peer_info_t peerInfo; //setup()の直前に置くことでグローバル変数にすることが重要！

void setup() {
  pinMode(15, INPUT_PULLUP);
  pinMode(4, OUTPUT); //電池残量測定用出力
  pinMode(2, ANALOG); //電池残量測定用入力

  esp_sleep_enable_ext0_wakeup(GPIO_NUM_15, 1);
  
  // 起動回数（ディープスリープの回数）をカウント
  bootCount++;
  Serial.begin(115200);
  Serial.print("bootCount = ");
  Serial.println(bootCount);
  
  // 電池残量確認・通知
  digitalWrite(4,HIGH);
  analogSetAttenuation(ADC_0db);
  float adc = analogRead(2);
  float volt = adc / 4096 * 0.95;
  Serial.print("\nVOLT = ");
  Serial.println(volt);
  if(volt < 0.75) { 
    Serial.print(low_battery_message);
    send_line(volt);
  }
  digitalWrite(4, LOW);

  // 初回起動時は実行しない
  if (bootCount > 1) {
    WiFi.mode(WIFI_STA);

    if (esp_now_init() != ESP_OK) {
      Serial.println("Error initializing ESP-NOW");
      return;
    }
    esp_now_register_send_cb(OnDataSent);

    memcpy(peerInfo.peer_addr, receiverMacAddress, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
      Serial.println("Failed to add peer");
      return;
    }

    nfc.begin();
    
    uint32_t versiondata = nfc.getFirmwareVersion();
    
    if (!versiondata) {
      Serial.println("Didn't find PN53x board");
      esp_deep_sleep_start();
    }
    // Got ok data, print it out!
    Serial.println("Found chip PN5");
    // PN532を設定
    nfc.setPassiveActivationRetries(0xFF);
    nfc.SAMConfig();
    Serial.print("Waiting for an Felica card");
    unsigned long startTime;
    unsigned long elapsedTime = 0;
    const unsigned long timeLimit = 5000;
    startTime = millis();
    while (elapsedTime < timeLimit) {
      elapsedTime = millis() - startTime;
      Serial.print(".");

      uint8_t idm[] = {0, 0, 0, 0, 0, 0, 0, 0};
      uint8_t pmm[] = {0, 0, 0, 0, 0, 0, 0, 0};
      uint16_t systemCode = 0xFFFF;
      uint16_t systemCodeResponse;

      int8_t result = nfc.felica_Polling(systemCode, 0x00, idm, pmm, &systemCodeResponse);

      if (result == 1) {
        if (memcmp(idm, SPECIFIC_CARD_IDM, 8) == 0) {
          Serial.println("\nSpecific FeliCa card detected!");
          Serial.print("IDm:");
          for (uint8_t i = 0; i < 8; i++) {
            Serial.print(idm[i] < 0x10 ? " 0" : " ");
            Serial.print(idm[i], HEX);
          }
          packet.cardDetected = true;
          esp_now_send(receiverMacAddress, (uint8_t*)&packet, sizeof(packet));
          delay(3000);
          elapsedTime = timeLimit;
        }
      } 
    } 
  }

  // ディープスリープ突入
  Serial.println("\nEntering Deep Sleep...");
  esp_deep_sleep_start();
}

void loop() {
}

// コールバック関数
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\nLast Packet Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// line通知
void send_line(float get_volt) {
  // WiFi接続
  WiFi.begin(wifi_ssid, wifi_password);

  // WiFiの接続状態を確認
  while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // HTTPSへアクセス（SSL通信）するためのライブラリ
  WiFiClientSecure client;

  // サーバー証明書の検証を行わずに接続する場合に必要
  client.setInsecure();
  
  //LineのAPIサーバにSSL接続（ポート443:https）
  if (!client.connect(host, 443)) {
    Serial.println("Connection failed");
    return;
  }
  Serial.println("Connected");

  // リクエスト送信
  String query = String("message=") + String(low_battery_message) + String("\nVOLT = ") + String(get_volt);
  String request = String("") +
               "POST /api/notify HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Authorization: Bearer " + token + "\r\n" +
               "Content-Length: " + String(query.length()) +  "\r\n" + 
               "Content-Type: application/x-www-form-urlencoded\r\n\r\n" +
                query + "\r\n";
  client.print(request);
 
  // 受信完了まで待機 
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      break;
    }
  }
  Serial.print("Line success! ");
}
