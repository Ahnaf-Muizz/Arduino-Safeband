/*
  Combined Multi-Tap Alarm and RFID Attendance System

  This sketch integrates two functionalities into one:
  1. Multi-Tap Alarm System:
     - 2 Taps (Concern): Activates a buzzer and red LED for 2 seconds.
     - 3 Taps (SOS): Activates a Morse code SOS signal with buzzer and red LED.
     - Safety Check: A red light prompts a check-in press every 10 seconds of inactivity.

  2. RFID Attendance Tracker:
     - Scans for RFID cards/tags using an MFRC522 reader.
     - Prints "Attendance Recorded!" to the Serial Monitor when a card is scanned.
     - Plays a short confirmation beep and flashes a green light on successful scan.
     - Scanning a card also resets the 10-second safety timer.

  *** IMPORTANT PIN CHANGE ***
  - The Buzzer has been moved from Pin 9 to Pin 6 to resolve a conflict with the RFID reader.

  Required Library:
  - MFRC522 by GithubCommunity (Install via Arduino Library Manager)

  Hardware Setup (for Arduino Uno/Nano):
  - Pushbutton:      Pin 2 to GND
  - Red LED:          Pin 8 to GND (via 220-ohm resistor)
  - Green LED:        Pin 7 to GND (via 220-ohm resistor)
  - Buzzer:           Pin 6 to GND
  
  - MFRC522 Reader --- Arduino
  - SDA (SS)----------- Pin 10
  - SCK ---------------- Pin 13
  - MOSI --------------- Pin 11
  - MISO --------------- Pin 12
  - RST ---------------- Pin 9
  - GND ---------------- GND
  - 3.3V --------------- 3.3V
*/

// --- Include Libraries ---
#include <SPI.h>
#include <MFRC522.h>

// --- Pin Definitions ---
const int PUSHBUTTON_PIN = 2;
const int LED_PIN = 8;
const int GREEN_LED_PIN = 7;
const int BUZZER_PIN = 6;       // MOVED to pin 6 to avoid conflict with RFID RST pin
#define RST_PIN 9
#define SS_PIN 10

// --- Timing Constants (in milliseconds) ---
const unsigned long TAP_TIMEOUT = 400;
const unsigned long DEBOUNCE_DELAY = 50;
const unsigned long SAFETY_INTERVAL = 3000;
const unsigned long RFID_COOLDOWN = 2000; // 2-second delay between RFID reads

// --- Global Variables ---
// RFID
MFRC522 rfid(SS_PIN, RST_PIN);
unsigned long lastRfidScanTime = 0;

// Button Handling
int tapCount = 0;
unsigned long lastPressTime = 0;
unsigned long buttonDownTime = 0;
boolean buttonWasPressed = false;

// State Machine
unsigned long lastActivityTime = 0;
unsigned long eventStartTime = 0;
enum SystemState { IDLE, SAFETY_PENDING, ALARM_SOS, ALARM_CONCERN, SAFETY_CONFIRMED, RFID_CONFIRMED };
SystemState currentState = IDLE;
SystemState previousState = IDLE;

// SOS Signal
int sosPhase = 0;
unsigned long sosPhaseStartTime = 0;


void setup() {
  Serial.begin(9600);
  while (!Serial);

  // Initialize Pins
  pinMode(PUSHBUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Initialize RFID
  SPI.begin();
  rfid.PCD_Init();

  // Initial State
  digitalWrite(LED_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, LOW);
  noTone(BUZZER_PIN);
  lastActivityTime = millis();

  Serial.println("--- Combined Alarm & RFID System Initialized ---");
  Serial.println("System is active. Scan card or use pushbutton.");
}

void handleButton() {
  if (digitalRead(PUSHBUTTON_PIN) == LOW && !buttonWasPressed) {
    buttonDownTime = millis();
    buttonWasPressed = true;
  }
  
  if (digitalRead(PUSHBUTTON_PIN) == HIGH && buttonWasPressed && (millis() - buttonDownTime > DEBOUNCE_DELAY)) {
    buttonWasPressed = false;
    lastActivityTime = millis();

    if (currentState == SAFETY_PENDING) {
      currentState = SAFETY_CONFIRMED;
      tapCount = 0;
      return;
    }

    if (millis() - lastPressTime < TAP_TIMEOUT) {
      tapCount++;
    } else {
      tapCount = 1;
    }
    lastPressTime = millis();
    Serial.print("Tap Count: ");
    Serial.println(tapCount);
  }
}

void handleRfid() {
  // Only check for a card if the cooldown period has passed
  if (millis() - lastRfidScanTime < RFID_COOLDOWN) {
    return;
  }

  // Look for new cards
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }
  
  // Card detected
  Serial.println("-----------------------");
  Serial.println("Attendance Recorded!");
  Serial.println("-----------------------");
  
  // Play a confirmation beep if no alarm is active
  if (currentState != ALARM_SOS && currentState != ALARM_CONCERN) {
    tone(BUZZER_PIN, 1500, 150); // High-pitched, short beep
  }
  
  lastActivityTime = millis(); // RFID scan counts as activity
  lastRfidScanTime = millis(); // Start cooldown timer
  
  // If in a safety check, an RFID scan also confirms safety.
  // If idle, show the RFID confirmation light.
  if (currentState == SAFETY_PENDING) {
      currentState = SAFETY_CONFIRMED;
  } else if (currentState == IDLE) {
      currentState = RFID_CONFIRMED;
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

void runSOS() {
    const int dot = 150, dash = dot * 3, gap = dot, letter_gap = dot * 3;
    unsigned long currentTime = millis();
    
    switch(sosPhase) {
        case 0: tone(BUZZER_PIN, 1000); digitalWrite(LED_PIN, HIGH); sosPhase++; sosPhaseStartTime = currentTime; break;
        case 1: if(currentTime - sosPhaseStartTime >= dot) { noTone(BUZZER_PIN); digitalWrite(LED_PIN, LOW); sosPhase++; sosPhaseStartTime = currentTime; } break;
        case 2: if(currentTime - sosPhaseStartTime >= gap) { tone(BUZZER_PIN, 1000); digitalWrite(LED_PIN, HIGH); sosPhase++; sosPhaseStartTime = currentTime; } break;
        case 3: if(currentTime - sosPhaseStartTime >= dot) { noTone(BUZZER_PIN); digitalWrite(LED_PIN, LOW); sosPhase++; sosPhaseStartTime = currentTime; } break;
        case 4: if(currentTime - sosPhaseStartTime >= gap) { tone(BUZZER_PIN, 1000); digitalWrite(LED_PIN, HIGH); sosPhase++; sosPhaseStartTime = currentTime; } break;
        case 5: if(currentTime - sosPhaseStartTime >= dot) { noTone(BUZZER_PIN); digitalWrite(LED_PIN, LOW); sosPhase++; sosPhaseStartTime = currentTime; } break;
        case 6: if(currentTime - sosPhaseStartTime >= letter_gap) { sosPhase++; } break;
        case 7: tone(BUZZER_PIN, 1000); digitalWrite(LED_PIN, HIGH); sosPhase++; sosPhaseStartTime = currentTime; break;
        case 8: if(currentTime - sosPhaseStartTime >= dash) { noTone(BUZZER_PIN); digitalWrite(LED_PIN, LOW); sosPhase++; sosPhaseStartTime = currentTime; } break;
        case 9: if(currentTime - sosPhaseStartTime >= gap) { tone(BUZZER_PIN, 1000); digitalWrite(LED_PIN, HIGH); sosPhase++; sosPhaseStartTime = currentTime; } break;
        case 10: if(currentTime - sosPhaseStartTime >= dash) { noTone(BUZZER_PIN); digitalWrite(LED_PIN, LOW); sosPhase++; sosPhaseStartTime = currentTime; } break;
        case 11: if(currentTime - sosPhaseStartTime >= gap) { tone(BUZZER_PIN, 1000); digitalWrite(LED_PIN, HIGH); sosPhase++; sosPhaseStartTime = currentTime; } break;
        case 12: if(currentTime - sosPhaseStartTime >= dash) { noTone(BUZZER_PIN); digitalWrite(LED_PIN, LOW); sosPhase++; sosPhaseStartTime = currentTime; } break;
        case 13: if(currentTime - sosPhaseStartTime >= letter_gap) { sosPhase++; } break;
        case 14: tone(BUZZER_PIN, 1000); digitalWrite(LED_PIN, HIGH); sosPhase++; sosPhaseStartTime = currentTime; break;
        case 15: if(currentTime - sosPhaseStartTime >= dot) { noTone(BUZZER_PIN); digitalWrite(LED_PIN, LOW); sosPhase++; sosPhaseStartTime = currentTime; } break;
        case 16: if(currentTime - sosPhaseStartTime >= gap) { tone(BUZZER_PIN, 1000); digitalWrite(LED_PIN, HIGH); sosPhase++; sosPhaseStartTime = currentTime; } break;
        case 17: if(currentTime - sosPhaseStartTime >= dot) { noTone(BUZZER_PIN); digitalWrite(LED_PIN, LOW); sosPhase++; sosPhaseStartTime = currentTime; } break;
        case 18: if(currentTime - sosPhaseStartTime >= gap) { tone(BUZZER_PIN, 1000); digitalWrite(LED_PIN, HIGH); sosPhase++; sosPhaseStartTime = currentTime; } break;
        case 19: if(currentTime - sosPhaseStartTime >= dot) { noTone(BUZZER_PIN); digitalWrite(LED_PIN, LOW); sosPhase++; sosPhaseStartTime = currentTime; } break;
        case 20: currentState = IDLE; lastActivityTime = currentTime; break;
    }
}


void loop() {
  unsigned long currentTime = millis();

  // Handle inputs from both sources
  handleButton();
  handleRfid();

  // --- Evaluate Multi-Tap Commands ---
  if (currentState == IDLE || currentState == SAFETY_PENDING) {
    if (tapCount > 0 && currentTime - lastPressTime > TAP_TIMEOUT) {
      if (tapCount == 3) { // 3 Taps for SOS
        Serial.println("SOS Signal Triggered!");
        currentState = ALARM_SOS;
      } else if (tapCount == 2) { // 2 Taps for Concern
        Serial.println("Concern Signal Triggered!");
        currentState = ALARM_CONCERN;
      }
      tapCount = 0;
    }
  }

  // --- Main State Machine ---
  if (currentState != previousState) { // If the state has changed, run setup code for the new state
      eventStartTime = currentTime;
      switch(currentState) {
          case ALARM_SOS: sosPhase = 0; break;
          case SAFETY_CONFIRMED: digitalWrite(LED_PIN, LOW); digitalWrite(GREEN_LED_PIN, HIGH); break;
          case ALARM_CONCERN: digitalWrite(LED_PIN, HIGH); tone(BUZZER_PIN, 500); break;
          case RFID_CONFIRMED: digitalWrite(GREEN_LED_PIN, HIGH); break;
      }
  }
  
  switch(currentState) {
    case IDLE:
      if (currentTime - lastActivityTime > SAFETY_INTERVAL) {
        currentState = SAFETY_PENDING;
        Serial.println("Safety Check Activated. Press button or scan card to confirm.");
        digitalWrite(LED_PIN, HIGH); // Turn on red light to prompt user
      }
      break;
    
    case SAFETY_CONFIRMED:
      if (currentTime - eventStartTime >= 1000) {
        digitalWrite(GREEN_LED_PIN, LOW);
        currentState = IDLE;
      }
      break;

    case RFID_CONFIRMED:
      if (currentTime - eventStartTime >= 1000) {
        digitalWrite(GREEN_LED_PIN, LOW);
        currentState = IDLE;
      }
      break;

    case ALARM_CONCERN:
      if (currentTime - eventStartTime >= 2000) {
        digitalWrite(LED_PIN, LOW);
        noTone(BUZZER_PIN);
        currentState = IDLE;
        lastActivityTime = currentTime;
      }
      break;
    
    case ALARM_SOS:
      runSOS();
      break;
  }
  
  previousState = currentState;
}

