#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

#define RST_PIN         9           // Configurable, see typical pin layout above
#define SS_PIN          10          // Configurable, see typical pin layout above

MFRC522 rfid(SS_PIN, RST_PIN);      // Create MFRC522 instance.
Servo myservo;

byte firstUID[10];  // Variable to store the first UID (up to 10 bytes)
byte secondUID[10]; // Variable to store the second UID (up to 10 bytes)
bool isFirstTagSet = false;  // Flag to indicate if the first tag has been stored
bool isSecondTagSet = false; // Flag to indicate if the second tag has been stored
int pos = 0;

void setup() {
  Serial.begin(9600);
  SPI.begin(); // init SPI bus
  rfid.PCD_Init(); // init MFRC522
  myservo.attach(5); 
  Serial.println("Tap RFID/NFC Tag on reader for initial setup.");
}

void loop() {
  if (rfid.PICC_IsNewCardPresent()) { // new tag is available
    if (rfid.PICC_ReadCardSerial()) { // NUID has been read
      MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
      Serial.print("RFID/NFC Tag Type: ");
      Serial.println(rfid.PICC_GetTypeName(piccType));

      if (!isFirstTagSet) {
        // Store the first tag UID
        for (int i = 0; i < rfid.uid.size; i++) {
          firstUID[i] = rfid.uid.uidByte[i];
        }
        isFirstTagSet = true; // Mark the first tag as stored

        // Print the stored first UID
        Serial.print("First UID stored:");
        for (int i = 0; i < rfid.uid.size; i++) {
          Serial.print(firstUID[i] < 0x10 ? " 0" : " ");
          Serial.print(firstUID[i], HEX);
        }
        Serial.println();
        Serial.println("First tag has been stored. Tap another tag to store the second tag.");

      } else if (!isSecondTagSet) {
        // Store the second tag UID
        for (int i = 0; i < rfid.uid.size; i++) {
          secondUID[i] = rfid.uid.uidByte[i];
        }
        isSecondTagSet = true; // Mark the second tag as stored

        // Print the stored second UID
        Serial.print("Second UID stored:");
        for (int i = 0; i < rfid.uid.size; i++) {
          Serial.print(secondUID[i] < 0x10 ? " 0" : " ");
          Serial.print(secondUID[i], HEX);
        }
        Serial.println();
        Serial.println("Second tag has been stored. All subsequent tags will be compared to the first two tags.");

      } else {
        // Compare the current tag with the first and second stored UIDs
        if (compareUIDs(rfid.uid.uidByte, firstUID, rfid.uid.size)) {
          Serial.println("The current tag matches the first tag.");
          moveServo(); // Move the motor if the first tag is matched
        } else if (compareUIDs(rfid.uid.uidByte, secondUID, rfid.uid.size)) {
          Serial.println("The current tag matches the second tag.");
          moveServo(); // Move the motor if the second tag is matched
        } else {
          Serial.println("The current tag does not match either stored tag.");
        }

        // Print the current UID
        Serial.print("Current UID:");
        for (int i = 0; i < rfid.uid.size; i++) {
          Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
          Serial.print(rfid.uid.uidByte[i], HEX);
        }
        Serial.println();
      }

      rfid.PICC_HaltA(); // halt PICC
      rfid.PCD_StopCrypto1(); // stop encryption on PCD
    }
  }
}

// Function to compare two UIDs
bool compareUIDs(byte *uid1, byte *uid2, byte size) {
  for (byte i = 0; i < size; i++) {
    if (uid1[i] != uid2[i]) {
      return false;
    }
  }
  return true;
}

// Function to move the servo motor
void moveServo() {
  for (pos = 0; pos <= 90; pos += 1) { // goes from 0 degrees to 90 degrees
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
  delay(5000);                       // Delay for 5 seconds (you can adjust)
  for (pos = 90; pos >= 0; pos -= 1) { // goes from 90 degrees back to 0 degrees
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
}
