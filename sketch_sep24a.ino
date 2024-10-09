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

unsigned long lastTagDetectedTime = 0; // To track the time of the last tag detection
const unsigned long tagTimeout = 5000; // 30 seconds timeout (in milliseconds)
bool isServoActive = false;  // Flag to track whether the servo is in an active position

void setup() {
  Serial.begin(9600);
  SPI.begin(); // init SPI bus
  rfid.PCD_Init(); // init MFRC522
  myservo.attach(5); 
  Serial.println("Tap RFID/NFC Tag on reader for initial setup.");
}

void loop() {
  // Check if a new card is present and if we have read its serial number
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
    Serial.print("RFID/NFC Tag Type: ");
    Serial.println(rfid.PICC_GetTypeName(piccType));

    // If first tag is not set, store it
    if (!isFirstTagSet) {
      storeUID(firstUID);
      isFirstTagSet = true;
      Serial.println("First tag has been stored. Tap another tag to store the second tag.");
    }
    // If second tag is not set, store it
    else if (!isSecondTagSet) {
      storeUID(secondUID);
      isSecondTagSet = true;
      Serial.println("Second tag has been stored. All subsequent tags will be compared to the first two tags.");
    }
    // Compare scanned tag with stored UIDs
    else {
      if (compareUIDs(rfid.uid.uidByte, firstUID, rfid.uid.size)) {
        Serial.println("The current tag matches the first tag.");
        activateServo();
      } else if (compareUIDs(rfid.uid.uidByte, secondUID, rfid.uid.size)) {
        Serial.println("The current tag matches the second tag.");
        activateServo();
      } else {
        Serial.println("The current tag does not match either stored tag.");
      }
    }

    // Record the last time a tag was detected
    lastTagDetectedTime = millis();

    // Print the current UID for debugging purposes
    printUID(rfid.uid.uidByte, rfid.uid.size);

    // Halt and stop encryption on PCD
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  }

  // Check if 30 seconds have passed since the last tag detection
  if (isServoActive && (millis() - lastTagDetectedTime >= tagTimeout)) {
    deactivateServo();  // Move the motor back to position 0
  }
}

// Function to store a UID in an array
void storeUID(byte *uidArray) {
  for (int i = 0; i < rfid.uid.size; i++) {
    uidArray[i] = rfid.uid.uidByte[i];
  }

  // Print the stored UID
  Serial.print("UID stored:");
  for (int i = 0; i < rfid.uid.size; i++) {
    Serial.print(uidArray[i] < 0x10 ? " 0" : " ");
    Serial.print(uidArray[i], HEX);
  }
  Serial.println();
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

// Function to move the servo motor to the active position (90 degrees)
void activateServo() {
  if (!isServoActive) {
    for (int pos = 0; pos <= 90; pos += 1) { // goes from 0 degrees to 90 degrees
      myservo.write(pos);              // tell servo to go to position in variable 'pos'
      delay(15);                       // waits 15ms for the servo to reach the position
    }
    isServoActive = true;  // Mark servo as active
  }
  lastTagDetectedTime = millis(); // Reset the timer since a tag was detected
}

// Function to move the servo motor back to 0 degrees (deactivation)
void deactivateServo() {
  for (int pos = 90; pos >= 0; pos -= 1) { // goes from 90 degrees back to 0 degrees
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
  isServoActive = false;  // Mark servo as inactive
  Serial.println("No tag detected for 30 seconds, returning servo to position 0.");
}

// Function to print the UID
void printUID(byte *uid, byte size) {
  Serial.print("Current UID:");
  for (int i = 0; i < size; i++) {
    Serial.print(uid[i] < 0x10 ? " 0" : " ");
    Serial.print(uid[i], HEX);
  }
  Serial.println();
}
