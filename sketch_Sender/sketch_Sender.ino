#include <esp_now.h>
#include <WiFi.h>
#include <SoftwareSerial.h> //EspSoftwareSerialライブラリ必須
#include <PN532_SWHSU.h>
#include <PN532.h>
SoftwareSerial SWSerial( 32, 33 ); // PN532のTX, RX
PN532_SWHSU pn532swhsu( SWSerial );
PN532 nfc( pn532swhsu );

// Replace with the MAC address of the receiver ESP32
uint8_t receiverMacAddress[] = {0x24, 0xDC, 0xC3, 0x46, 0xB7, 0x64};

// Define the IDm of the specific FeliCa card you want to detect
const uint8_t SPECIFIC_CARD_IDM[] = {0x01, 0x40, 0x8A, 0x8F, 0x2E, 0x37, 0xB6, 0x60};

// Structure to send data
typedef struct {
  bool cardDetected;
} DataPacket;

DataPacket packet;

// コールバック関数
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Last Packet Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

esp_now_peer_info_t peerInfo; //setup()の直前に置くことでグローバル変数にすることが重要！

void setup() {
  Serial.begin(115200);
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
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata >> 24) & 0xFF, HEX);
  Serial.print("Firmware ver. "); Serial.print((versiondata >> 16) & 0xFF, DEC);
  Serial.print('.'); Serial.println((versiondata >> 8) & 0xFF, DEC);
  // PN532を設定
  nfc.setPassiveActivationRetries(0xFF);
  nfc.SAMConfig();
  Serial.println("Waiting for an Felica card");
}

void loop() {
  uint8_t idm[] = {0, 0, 0, 0, 0, 0, 0, 0};
  uint8_t pmm[] = {0, 0, 0, 0, 0, 0, 0, 0};
  uint16_t systemCode = 0xFFFF;
  uint16_t systemCodeResponse;

  int8_t result = nfc.felica_Polling(systemCode, 0x00, idm, pmm, &systemCodeResponse);

  if (result == 1) {
    if (memcmp(idm, SPECIFIC_CARD_IDM, 8) == 0) {
      Serial.print("\nSpecific FeliCa card detected!");
      Serial.print("IDm: ");
      for (uint8_t i = 0; i < 8; i++) {
        Serial.print(idm[i] < 0x10 ? " 0" : " ");
        Serial.print(idm[i], HEX);
      }
      packet.cardDetected = true;
      esp_now_send(receiverMacAddress, (uint8_t*)&packet, sizeof(packet));
      delay(3000);
    }
  }
  
}
