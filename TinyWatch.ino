#include <avr/sleep.h>
#include "src/Display/DisplayManager.h"

#define LEFT_BTN 1
#define LEFT_BTN_STATE digitalRead(LEFT_BTN)

#define RIGHT_BTN 4
#define RIGHT_BTN_STATE digitalRead(RIGHT_BTN)

#define NOTHING_PRESSED (LEFT_BTN_STATE == LOW && RIGHT_BTN_STATE == LOW)

#define PUSH_BTN_INTERVAL 1000

#define TIMEOUT 5000 // 5 seconds


static uint32_t display_timeout;

unsigned long previousMillisBtn = 0,
              previousMillisBothBtns = 0,
              previousMillis = 0,
              currentMillis = 0;

bool sleeping = false,
     modeEntry = true,
     bothBtnsPressed = false,
     oneBtnPressed = false,
     timeUnitChanged = false,
     showActualChangedTimeUnit = false;

Mode mode = IDDLE;
TimeUnit actualChanged = HOUR;
DisplayManager displayManager = DisplayManager();

void setup() {
  init_wdt();

  displayManager.init();

  pinMode(LEFT_BTN, INPUT);
  pinMode(RIGHT_BTN, INPUT);

  setDisplayTimeout();
}

void handleButtonInSetMode(ChangeType changeType) {
  if (timeUnitChanged) {
    return;
  }
  if (!oneBtnPressed) {
    displayManager.time.change(actualChanged, changeType);
    displayManager.printTime();
    oneBtnPressed = true;
  }

  if (currentMillis - previousMillisBtn >= SEC_INTERVAL) {
    displayManager.time.change(actualChanged, changeType);
    displayManager.printTime();
  }
}

void setDisplayTimeout() {
  display_timeout = millis() + TIMEOUT;
}

void changeMode(Mode newMode, unsigned long *millisToSet) {
  mode = newMode;
  modeEntry = true;
  *millisToSet = currentMillis;
  displayManager.printButtonDesc(newMode, true);
}

bool checkIfAnyBtnPressedInFirstEntry() {
  if (modeEntry && (LEFT_BTN_STATE == HIGH || RIGHT_BTN_STATE == HIGH)) {
    return true;
  } else {
    modeEntry = false;
  }
  return modeEntry;
}

void drawOled() {
  if (mode == IDDLE) {
    currentMillis = millis();
    displayManager.printTime();

    if (checkIfAnyBtnPressedInFirstEntry()) return;
    mode = IDDLE;

    bool canFireAction = currentMillis - previousMillisBtn >= PUSH_BTN_INTERVAL;
    if (LEFT_BTN_STATE == HIGH && canFireAction) {
      changeMode(SET, &previousMillisBtn);
    } else if (RIGHT_BTN_STATE == HIGH && canFireAction) {
      displayManager.clearScreen();
      changeMode(DEBUG, &previousMillisBtn);
    } else if (NOTHING_PRESSED) {
      previousMillisBtn = currentMillis;
    }

  } else if (mode == SET) {

    if (checkIfAnyBtnPressedInFirstEntry()) return;

    currentMillis = millis();
    if (LEFT_BTN_STATE == HIGH && RIGHT_BTN_STATE == HIGH) {
      if (currentMillis - previousMillisBothBtns >= PUSH_BTN_INTERVAL) {
        changeMode(IDDLE, &previousMillisBothBtns);
        wdt_auto_tune();
        setDisplayTimeout();
        displayManager.showActualChangedTimeUnit = false;
      } else if (!bothBtnsPressed) {
        actualChanged = displayManager.time.nextUnitToChange(actualChanged);
        timeUnitChanged = true;
      }
      bothBtnsPressed = true;
      return;

    } else if (LEFT_BTN_STATE == HIGH || RIGHT_BTN_STATE == HIGH) {
      ChangeType change = LEFT_BTN_STATE == HIGH ? DECRASE : INCRASE;
      handleButtonInSetMode(change);
      previousMillis = currentMillis;
      displayManager.showActualChangedTimeUnit = false;
    } else {
      displayManager.flashActualChangedTimeUnit(&previousMillis, actualChanged);
      previousMillisBtn = currentMillis;
      oneBtnPressed = false;
    }
    previousMillisBothBtns = currentMillis;
    bothBtnsPressed = false;
    timeUnitChanged = false;

  } else if (mode == DEBUG) {

    displayManager.printDebugInfo();

    if (checkIfAnyBtnPressedInFirstEntry()) return;

    if (LEFT_BTN_STATE == HIGH) {
      displayManager.clearScreen();
      changeMode(IDDLE, &previousMillisBtn);
      setDisplayTimeout();
    }
  }
}

void loop() {

  if (sleeping) {
    if (RIGHT_BTN_STATE == HIGH) {
      sleeping = false;
      displayManager.init();
      setDisplayTimeout();
    } else {
      system_sleep();
    }
  } else {

    bool canDisplayOff = millis() > display_timeout && mode == IDDLE
                         && LEFT_BTN_STATE == LOW && RIGHT_BTN_STATE == LOW;
    if (canDisplayOff) {

      displayManager.sleep();
      sleeping = true;
      modeEntry = true;
    } else {
      drawOled();
    }
  }
}

ISR(PCINT0_vect) {
  setDisplayTimeout();
}
