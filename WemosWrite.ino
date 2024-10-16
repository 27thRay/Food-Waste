#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         0   // D3 (GPIO 0) on WeMos D1 Mini
#define SS_PIN          15  // D8 (GPIO 15) on WeMos D1 Mini
#define BUZZER_PIN      2   // Assume the buzzer is connected to D4 (GPIO 2)

MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

constexpr byte sector         = 1;
constexpr byte blockAddr      = 4;
constexpr byte trailerBlock   = 7;
const String textToWrite = "stall_1";  // Text that you want to write to the NFC card

void setup() {
    Serial.begin(115200);
    while (!Serial);
    SPI.begin();
    mfrc522.PCD_Init();
    pinMode(BUZZER_PIN, OUTPUT);

    // Prepare the key (used both as key A and as key B)
    for (byte i = 0; i < 6; i++) {
        key.keyByte[i] = 0xFF;
    }

    Serial.println(F("NFC Writer Ready. Tap a card to write."));
}

void loop() {
    if (writeNFC(textToWrite)) {
        Serial.println("Data written to NFC card successfully!");
        digitalWrite(BUZZER_PIN, HIGH);
        delay(100);
        digitalWrite(BUZZER_PIN, LOW);
    }
    delay(1000);  // Slight delay to avoid spamming the NFC write process
}

bool writeNFC(String data) {
    // Look for new cards
    if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
        return false;
    }

    Serial.println(F("Card detected. Attempting to write..."));

    // Authenticate using key A
    MFRC522::StatusCode status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("Authentication failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        return false;
    }

    // Prepare the data to write (must be 16 bytes)
    byte buffer[16];
    data.getBytes(buffer, sizeof(buffer));  // Copy data to buffer
    if (data.length() < 16) {
        for (int i = data.length(); i < 16; i++) {
            buffer[i] = ' ';  // Fill the remaining bytes with spaces
        }
    }

    // Write data to the NFC card
    status = mfrc522.MIFARE_Write(blockAddr, buffer, 16);
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("Writing failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        return false;
    }

    Serial.println("Data successfully written to the card!");

    mfrc522.PICC_HaltA();  // Halt PICC
    mfrc522.PCD_StopCrypto1();  // Stop encryption on PCD

    return true;
}
