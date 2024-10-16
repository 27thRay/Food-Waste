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

    Serial.println(F("NFC Reader Ready. Tap a card to read."));
}

void loop() {
    String nfcData = readNFC();
    if (nfcData != "") {
        nfcData = cleanString(nfcData);  // Clean the data before processing
        Serial.println("NFC Data: " + nfcData);  // Display the NFC data in the Serial Monitor
        digitalWrite(BUZZER_PIN, HIGH);
        delay(100);
        digitalWrite(BUZZER_PIN, LOW);
    }
}

// Function to clean NFC data and remove non-printable characters
String cleanString(String input) {
    String result = "";
    for (int i = 0; i < input.length(); i++) {
        char c = input[i];
        if (isPrintable(c)) {  // Only add printable characters
            result += c;
        }
    }
    return result;
}

String readNFC() {
    // Look for new cards
    if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
        return "";
    }

    Serial.println(F("Card detected. Attempting to read..."));

    // Authenticate using key A
    MFRC522::StatusCode status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
        Serial.println(F("Authentication failed"));
        return "";
    }

    // Read data
    byte buffer[18];
    byte size = sizeof(buffer);
    status = mfrc522.MIFARE_Read(blockAddr, buffer, &size);
    if (status != MFRC522::STATUS_OK) {
        Serial.println(F("Reading failed"));
        return "";
    }

    String result = "";
    for (byte i = 0; i < 16; i++) {
        result += (char)buffer[i];
    }

    Serial.println("Data read: " + result);

    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();

    return result;
}
