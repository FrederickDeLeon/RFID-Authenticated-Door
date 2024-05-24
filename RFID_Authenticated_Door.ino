/*** header block ***
 * code file name: RFID_Authenticated_Door
 * code description: This program will allow a door (represented by a servo motor) to be opened via an RFID Sensor scanning a valid RFID Card or Tag, 
 whether the card is valid or not will be indicated through the use of an active and a passive buzzer, and the user will be prompted to present their 
 valid RFID card through an LCD, a photoresistor is also implemented to act as a motion sensor of sorts so that the system can detect when a person 
 hovers their hand over it and it can prompt them to present their RFID card/tag. 
 * hardware required: Arduino MEGA 2560 Microcontroller Development Board
 * IDE version used to test code: Arduino IDE v2.3.2
 * programmer(s) name: Frederick De Leon
 * date when code is created/modified: Created 5/20/2024, Last Modified 5/22/2024
 * code version: v1.6 Based on Dumpinfo.ino, Knob2.ino by F. Zia, and toneMelody.ino
 ***/

// Include necessary libraries 
#include <LiquidCrystal.h>
#include <Servo.h>
#include <SPI.h>
#include <MFRC522.h>

// Define notes used for acceptedTone() function
#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523

// Define the melody and note durations
int melody[] = {
  NOTE_C4, NOTE_D4, NOTE_E4, NOTE_F4, NOTE_G4, NOTE_A4, NOTE_B4, NOTE_C5
};

int noteDurations[] = {
  4, 4, 4, 4, 4, 4, 4, 4  
};

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 30, en = 31, d4 = 33, d5 = 32, d6 = 34, d7 = 35;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

Servo myservo; // Create a servo object 
int pos = 700; // Set initial position of servo

int photo_pin = 0;       // analog pin used to connect the photoresistor, A0
int photo_val;          // variable to store the value from the analog pin 

int active_buzzer = 4; // active buzzer connected to digital pin 4 
int passive_buzzer = 2; // passive buzzer connected to digital pin 4 

// RFID setup
#define SS_PIN 53
#define RST_PIN 13
#define SCK_PIN 52
#define MOSI_PIN 51
#define MISO_PIN 50
MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance

byte validUID[] = {0x8A, 0x0A, 0x76, 0x9F}; // define the UID code for the valid RFID card

// Create FSM States
enum State {
  IDLE,
  PROMPT,
  SCAN,
  VALID,
  INVALID,
  OPEN_DOOR,
  DOOR_OPEN,
  CLOSE_DOOR,
  DOOR_CLOSED
};

State currentState = IDLE; // Set current state of the FSM 

void setup() { // Code runs once on startup 
  Serial.begin(9600);

  myservo.attach(8); // attaches servo motor to pin 8 
  pinMode(active_buzzer, OUTPUT); // initializes active buzzer as an output device

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);

  // Initialize SPI bus and MFRC522
  SPI.begin();
  mfrc522.PCD_Init();
}

void loop() { 
  switch (currentState) {
    case IDLE:
      photo_val = analogRead(photo_pin); // read value of photoresistor
      if (photo_val >= 15 && photo_val <= 75) { 
      // Triggers if the value from the photoresistor falls within this range, 
      // this range is roughly the value from the photoresistor when a hand is obscuring it 
      // from approx. 2-3 inches above in a fully lit room
        currentState = PROMPT;
      }
      break;
      
    case PROMPT:
      lcd.clear();
      lcd.print("Please provide a");
      lcd.setCursor(0, 1);
      lcd.print("valid RFID card.");
      currentState = SCAN;
      break;
      
    case SCAN:
      if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
        detectedTone();
        delay(500); // delay 500ms to distinguish detectedTone() from acceptedTone()
        if (checkUID(mfrc522.uid.uidByte, mfrc522.uid.size)) {
          currentState = VALID;
        } else {
          currentState = INVALID;
        }
        mfrc522.PICC_HaltA(); // Halt PICC
        mfrc522.PCD_StopCrypto1(); // Stop encryption on PCD
      }
      break;
      
    case VALID: // UID is valid, actuate the servo, open the door
      lcd.clear();
      lcd.print("Access Granted");
      acceptedTone(); // play melody to indicate RFID card was valid 
      delay(5000);
      lcd.clear();
      lcd.print("Door Opening");
      openDoor();
      lcd.clear();
      lcd.print("Door Open");
      delay(60000); // 15 seconds (15000) for testing purposes, 1 minute (60000) for final implementation 
      currentState = CLOSE_DOOR;
      break;
      
    case INVALID:
      lcd.clear();
      invalidTone();
      lcd.print("Invalid UID,");
      lcd.setCursor(0, 1);
      lcd.print("please try again.");
      delay(2000); // Wait 2 seconds before clearing the screen
      lcd.clear();
      currentState = PROMPT;
      break;
      
    case OPEN_DOOR:
      openDoor();
      currentState = DOOR_OPEN;
      break;
      
    case DOOR_OPEN:
      delay(60000); // 15 seconds (15000) for testing purposes, 1 minute (60000) for final implementation 
      currentState = CLOSE_DOOR;
      break;
      
    case CLOSE_DOOR:
      lcd.clear();
      lcd.print("Door Closing");
      closeDoor();
      lcd.clear();
      lcd.print("Door Closed");
      currentState = DOOR_CLOSED;
      delay(15000); // display "Door Close" for 15 seconds before wiping the LCD
      lcd.clear();
      break;
      
    case DOOR_CLOSED:
      currentState = IDLE;
      break;
  }
}

bool checkUID(byte *uid, byte uidSize) {
  if (uidSize != 4) return false; // UID must be 4 bytes long
  for (byte i = 0; i < 4; i++) {
    if (uid[i] != validUID[i]) return false;
  }
  return true;
}

// Custom Functions

void openDoor() { // function to open door 
  for (pos = 700; pos <= 2500; pos += 10) { // goes from fully counter-clockwise to fully clockwise, in increments of 10
    myservo.writeMicroseconds(pos);
    delay(15);
  }
}

void closeDoor() { // function to close door 
  for (pos = 2500; pos >= 700; pos -= 10) { // goes from fully clockwise to fully counter-clockwise, in increments of 10 
    myservo.writeMicroseconds(pos);
    delay(15);
  }
}

void detectedTone() { // function that turns on active buzzer when the RFID detects an RFID card
  digitalWrite(active_buzzer, HIGH); // turn active buzzer on 
  delay(30); // delay for 30ms 
  digitalWrite(active_buzzer, LOW); // turn active buzzer off 
}

void acceptedTone() { // function to create the melody to play when UID detected is valid 
  for (int thisNote = 0; thisNote < 8; thisNote++) {
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(passive_buzzer, melody[thisNote], noteDuration);

    // Pause for the note's duration
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);

    // Stop the tone
    noTone(passive_buzzer);
  }
}

void invalidTone() { // function to create the melody to play when UID detected is invalid
  int frequencies[] = {200, 300, 250, 350, 300, 400, 350}; // Lower-pitched, harsh frequencies
  int duration = 100; // Duration for each beep in milliseconds

  for (int i = 0; i < 7; i++) {
    tone(passive_buzzer, frequencies[i], duration);
    delay(duration * 1.5); // Short delay to make it more jarring
    noTone(passive_buzzer);
  }
}
