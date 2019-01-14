#include "DisplayManager.h"

void DisplayManager::init() {
  _delay_ms(40);
  SSD1306.ssd1306_init();
  SSD1306.ssd1306_fillscreen(0x00);
  printTime();
  printButtonDesc(IDDLE);
}

void DisplayManager::wakeUp() {
  SSD1306.ssd1306_on();
  _delay_ms(40);
  printTime();
  printButtonDesc(IDDLE);
}

void DisplayManager::sleep() {
  buttonDescPrinted = false;
  SSD1306.ssd1306_fillscreen(0x00);
  SSD1306.ssd1306_off();
  _delay_ms(40);
}

void DisplayManager::clearScreen() {
  SSD1306.ssd1306_fillscreen(0x00);
}

void DisplayManager::flashActualChangedTimeUnit(unsigned long *previousMillis, TimeUnit actualChanged) {
  if (millis() - (*previousMillis) > SEC_INTERVAL) {
    *previousMillis = millis();
    showActualChangedTimeUnit = !showActualChangedTimeUnit;
  }
  char *timeAsStr = showActualChangedTimeUnit ? time.toStringWithout(actualChanged) : time.toString();
  SSD1306.ssd1306_char_f16x32(0, 1, timeAsStr);
  free(timeAsStr);
}

void DisplayManager::printTime() {
  char *timeAsStr = time.toString();
  SSD1306.ssd1306_char_f16x32(0, 1, timeAsStr);
  free(timeAsStr);
}

void DisplayManager::printButtonDesc(Mode currentMode, bool force) {
  if (buttonDescPrinted && !force) {
    return;
  }
  SSD1306.ssd1306_setpos(0, BUTTON_DESC_Y_OFFSET);
  SSD1306.ssd1306_string_font6x8("                     ");
  char *left, *center, *right;
  int xRight;
  if (currentMode == IDDLE) {
    left = "SET", center = "", right = "DEBUG"; xRight = 95;
  } else if (currentMode == SET) {
    left = "DOWN", center = "CHANGE", right = "UP"; xRight = 110;
  } else if (currentMode == DEBUG) {
    left = "BACK", center = "", right = ""; xRight = 100;
  }
  SSD1306.ssd1306_setpos(0, BUTTON_DESC_Y_OFFSET);
  SSD1306.ssd1306_string_font6x8(left);
  SSD1306.ssd1306_setpos(45, BUTTON_DESC_Y_OFFSET);
  SSD1306.ssd1306_string_font6x8(center);
  SSD1306.ssd1306_setpos(xRight, BUTTON_DESC_Y_OFFSET);
  SSD1306.ssd1306_string_font6x8(right);
  buttonDescPrinted = true;
}

void DisplayManager::printDebugInfo(char key, int val, int yPos) {
  char *str = new char[12]; str[0] = key; str[1] = ':'; str[2] = '0';
  itoa(val, str + 2, 10);
  SSD1306.ssd1306_setpos(40, yPos);
  SSD1306.ssd1306_string_font6x8(str);
  free(str);
}

void DisplayManager::printDebugInfo() {
  byte i = -1;
  //sprintDebugInfo('I', get_wdt_interrupt_count(), ++i);
  printDebugInfo('M', get_wdt_microsecond_per_interrupt(), ++i);
  printDebugInfo('V', getVcc(), ++i);
  printDebugInfo('T', getRawTemp(), ++i);
}
