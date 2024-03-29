#include <PN532_SWHSU.h>
#include <PN532.h>
#include <SoftwareSerial.h>
#include <Stepper.h>

SoftwareSerial softSerial(2, 3); // RX, TX
PN532_SWHSU pn532swhsu(softSerial);
PN532 nfc(pn532swhsu);
Stepper sm(2048, 8, 10, 9, 11);
const uint8_t SPECIFIC_CARD_IDM[] = { 0x01, 0x40, 0x8A, 0x8F, 0x2E, 0x37, 0xB6, 0x60 };

void setup() {
  Serial.begin(115200);
  Serial.println("Hello!");

  nfc.begin();
  sm.setSpeed(15);

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
  
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata >> 24) & 0xFF, HEX);
  Serial.print("Firmware ver. "); Serial.print((versiondata >> 16) & 0xFF, DEC);
  Serial.print('.'); Serial.println((versiondata >> 8) & 0xFF, DEC);
  
  // Set the max number of retry attempts to read from a card
  // This prevents us from waiting forever for a card, which is
  // the default behaviour of the PN532.
  nfc.setPassiveActivationRetries(0xFF);
  
  // Configure board to read RFID tags
  nfc.SAMConfig();
  
  Serial.println("Waiting for an Felica card");
}

void loop() {
  uint8_t idm[] = { 0, 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the IDm (8 bytes)
  uint8_t pmm[] = { 0, 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the PMm (8 bytes)
  uint16_t systemCode = 0xFFFF;                // Wildcard system code (0xFFFF)
  uint16_t systemCodeResponse;
  
  // Wait for a FeliCa card (including Mobile Suica)
  int8_t result = nfc.felica_Polling(systemCode, 0x00, idm, pmm, &systemCodeResponse);
  
  if (result == 1) {
    Serial.println("Mobile Suica detected!");
    
    // Print the IDm of the Mobile Suica
    Serial.print("IDm: ");
    for (uint8_t i = 0; i < 8; i++) {
      Serial.print(idm[i] < 0x10 ? " 0" : " ");
      Serial.print(idm[i], HEX);
    }
    Serial.println();
    
    if (memcmp(idm, SPECIFIC_CARD_IDM, 8) == 0) {
      Serial.println("Specific FeliCa card detected!");
      sm.step((float)2048 * +90 / 360);
      delay(1000);
      sm.step((float)2048 * -90 / 360);
    }
    // Wait 1 second before continuing
    delay(1000);
  }
  else
  {
    // PN532 probably timed out waiting for a card
    delay(800);
  }
}