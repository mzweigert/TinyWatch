#ifndef _DISPLAY_MANAGER_H_
#define _DISPLAY_MANAGER_H_

#include "font6x8.h"
#include "ssd1306xled.h"
#include "ssd1306xled16x32.h"
#include "../Time/TimeManager.h"

#define SEC_INTERVAL 1000

#define BUTTON_DESC_Y_OFFSET 7

enum Mode {
  IDDLE,
  SET,
  DEBUG
};

class DisplayManager {

  private:
    bool buttonDescPrinted = false;
    void printDebugInfo(char key, int val, int yPos);

  public:
    TimeManager time = TimeManager();
    bool showActualChangedTimeUnit = true;
    void init();
    void clearScreen();
    void printTime();
    void printButtonDesc(Mode currentMode, bool force = false);
    void flashActualChangedTimeUnit(unsigned long *previousMillis, TimeUnit actualChanged);
    void wakeUp();
    void sleep();
    void printDebugInfo();
    TimeUnit nextUnitToChange();


};

#endif
