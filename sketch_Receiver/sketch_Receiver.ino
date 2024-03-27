#include <esp_now.h>
#include <WiFi.h>
#include <Stepper.h>

Stepper sm(2048, 32, 25, 33, 26);

const int TACT_SWITCH_PIN = 27;

// Structure to receive data
typedef struct {
  bool cardDetected;
} DataPacket;

DataPacket packet;

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);

  sm.setSpeed(10);
  pinMode(TACT_SWITCH_PIN, INPUT_PULLUP);

}

void loop() {
  if (digitalRead(TACT_SWITCH_PIN) == LOW) { // タクトスイッチが押されたかどうかを確認
    unLock(); // モーターを動作させる関数を呼び出す
    delay(3000);
  }
}

void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  memcpy(&packet, data, sizeof(packet));
  if (packet.cardDetected) {
    unLock(); 
    delay(3000);
  }
}

void unLock() {
  Serial.println("Door unlocked");
  sm.step((float)2048 * +90 / 360);
  delay(1000);
  sm.step((float)2048 * -90 / 360);
  delay(100);
  // モーターの動作が完了した直後にピンをLOWにする
  digitalWrite(32, LOW);
  digitalWrite(25, LOW);
  digitalWrite(33, LOW);
  digitalWrite(26, LOW);
}
