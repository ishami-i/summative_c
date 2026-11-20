#include <LiquidCrystal.h>

// Initialize LCD with interface pins (RS, E, D4, D5, D6, D7)
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// Pin definitions
const int entryBtnPin = 7;  // Brown wire
const int exitBtnPin  = 6;  // Grey wire
const int greenLedPin = 9;  // Pink wire
const int redLedPin   = 8;  // Black wire


// Room capacity settings
const int MAX_CAPACITY = 10;  // Change this to your desired room capacity
int peopleCount = 0;          // Starts at 0

// Button state tracking for edge detection
int lastEntryState = HIGH;
int lastExitState  = HIGH;

void setup() {
  // Set up buttons with internal pull-up resistors
  pinMode(entryBtnPin, INPUT_PULLUP);
  pinMode(exitBtnPin, INPUT_PULLUP);

  // Set up LEDs
  pinMode(greenLedPin, OUTPUT);
  pinMode(redLedPin, OUTPUT);

  // Start LCD
  lcd.begin(16, 2);
  
  // Initial display & LED state
  updateSystem();
}

void loop() {
  int currentEntryState = digitalRead(entryBtnPin);
  int currentExitState  = digitalRead(exitBtnPin);

  // Check if Entry button was just pressed (HIGH to LOW transition)
  if (lastEntryState == HIGH && currentEntryState == LOW) {
    if (peopleCount < MAX_CAPACITY) {
      peopleCount++;
      updateSystem();
    }
    delay(50); // Simple debounce
  }

  // Check if Exit button was just pressed (HIGH to LOW transition)
  if (lastExitState == HIGH && currentExitState == LOW) {
    if (peopleCount > 0) {
      peopleCount--;
      updateSystem();
    }
    delay(50); // Simple debounce
  }

  lastEntryState = currentEntryState;
  lastExitState  = currentExitState;
}

void updateSystem() {
  // Update LCD Line 1: People count out of capacity
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("People: ");
  lcd.print(peopleCount);
  lcd.print("/");
  lcd.print(MAX_CAPACITY);

  // Update LCD Line 2 & LEDs based on room space
  lcd.setCursor(0, 1);
  if (peopleCount >= MAX_CAPACITY) {
    lcd.print("Status: FULL");
    digitalWrite(greenLedPin, LOW);
    digitalWrite(redLedPin, HIGH);
  } else {
    lcd.print("Space Available");
    digitalWrite(greenLedPin, HIGH);
    digitalWrite(redLedPin, LOW);
  }
}
