/*
 * Revised time function count time by WDT timer instead of millis() function
 * Add WDT and power related function
 * Ref.:
 * time function v1.4: https://github.com/PaulStoffregen/Time
 * WDT and power related: http://www.re-innovation.co.uk/web12/index.php/en/blog-75/306-sleep-modes-on-attiny85
 * readVcc: http://forum.arduino.cc/index.php?topic=222847.0
 * Internal temperature sensor: http://21stdigitalhome.blogspot.hk/2014/10/trinket-attiny85-internal-temperature.html
*/

#if ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include <avr/wdt.h>        // Supplied Watch Dog Timer Macros 
#include <avr/sleep.h>      // Supplied AVR Sleep Macros
#include "WDT_Time.h"

// Routines to clear and set bits (used in the sleep code)
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

static tmElements_t tm;          // a cache of time elements
static time_t cacheTime;   // the time the cache was updated

static uint32_t sysTime = 0;
static uint32_t prev_microsecond = 0;
static timeStatus_t Status = timeNotSet;

static uint32_t wdt_microsecond = 0;
static uint32_t prev_sysTime = 0;

void refreshCache(time_t t) {
  if (t != cacheTime) {
    breakTime(t, tm);
    cacheTime = t;
  }
}

uint8_t hour() { // the hour now
  return hour(now());
}

uint8_t hour(time_t t) { // the hour for the given time
  refreshCache(t);
  return tm.Hour;
}

uint8_t hourFormat12() { // the hour now in 12 hour format
  return hourFormat12(now());
}

uint8_t hourFormat12(time_t t) { // the hour for the given time in 12 hour format
  refreshCache(t);
  if ( tm.Hour == 0 )
    return 12; // 12 midnight
  else if ( tm.Hour  > 12)
    return tm.Hour - 12 ;
  else
    return tm.Hour ;
}

bool isAM() { // returns true if time now is AM
  return !isPM(now());
}

bool isAM(time_t t) { // returns true if given time is AM
  return !isPM(t);
}

bool isPM() { // returns true if PM
  return isPM(now());
}

bool isPM(time_t t) { // returns true if PM
  return (hour(t) >= 12);
}

uint8_t minute() {
  return minute(now());
}

uint8_t minute(time_t t) { // the minute for the given time
  refreshCache(t);
  return tm.Minute;
}

uint8_t second() {
  return second(now());
}

uint8_t second(time_t t) {  // the second for the given time
  refreshCache(t);
  return tm.Second;
}


/*============================================================================*/
/* functions to convert to and from system time */
/* These are for interfacing with time serivces and are not normally needed in a sketch */

void breakTime(time_t timeInput, tmElements_t &tm) {
  // break the given time_t into time components
  // this is a more compact version of the C library localtime function
  // note that year is offset from 1970 !!!

  uint32_t tmp_time = (uint32_t)timeInput;

  tm.Second = tmp_time % 60;
  tmp_time /= 60; // now it is minutes
  tm.Minute = tmp_time % 60;
  tmp_time /= 60; // now it is hours
  tm.Hour = tmp_time % 24;
  tmp_time /= 24; // now it is days
}

time_t makeTime(tmElements_t &tm) {
  // assemble time elements into time_t
  // note year argument is offset from 1970 (see macros in time.h to convert to other formats)
  // previous version used full four digit year (or digits since 2000),i.e. 2009 was 2009 or 9

  uint8_t i;
  uint32_t seconds = 0;

  seconds += tm.Hour * SECS_PER_HOUR;
  seconds += tm.Minute * SECS_PER_MIN;
  seconds += tm.Second;
  return (time_t)seconds;
}

/*=====================================================*/
/* Low level system time functions  */

time_t now() {
  while (wdt_microsecond - prev_microsecond >= 1000000UL) {
    sysTime++;
    prev_microsecond += 1000000UL;
  }

  return (time_t)sysTime;
}

void setTime(time_t t) {
  sysTime = (uint32_t)t;
  Status = timeSet;
  prev_microsecond = wdt_microsecond; // restart counting from now (thanks to Korman for this fix)
}

void setTime(uint8_t hr, uint8_t mnt, uint8_t scnd) {
  // year can be given as full four digit year or two digts (2010 or 10 for 2010);
  //it is converted to years since 1970

  tm.Hour = hr;
  tm.Minute = mnt;
  tm.Second = scnd;
  setTime(makeTime(tm));
}

void adjustTime(long adjustment) {
  sysTime += adjustment;
}

/* WDT and power related */
// TODO: dynamic calibrate wdt_microsecond_per_interrupt by current voltage (readVcc) and temperature
uint32_t wdt_microsecond_per_interrupt = DEFAULT_WDT_MICROSECOND; // calibrate value
uint32_t wdt_interrupt_count = 0;

// 0=16ms, 1=32ms,2=64ms,3=128ms,4=250ms,5=500ms
// 6=1 sec,7=2 sec, 8=4 sec, 9= 8sec
void setup_watchdog(uint8_t ii) {
  byte bb;
  int ww;
  if (ii > 9 ) ii = 9;
  bb = ii & 7;
  if (ii > 7) bb |= (1 << 5);
  bb |= (1 << WDCE);
  ww = bb;

  MCUSR &= ~(1 << WDRF);
  // start timed sequence
  WDTCR |= (1 << WDCE) | (1 << WDE);
  // set new watchdog timeout value
  WDTCR = bb;
  WDTCR |= _BV(WDIE);
  sbi(GIMSK, PCIE); // Turn on Pin Change interrupts (Tell Attiny85 we want to use pin change interrupts (can be any pin))
  sbi(PCMSK, PCINT3);
  //sbi(PCMSK, PCINT4);
  sei();    // Enable the Interrupts
}

void init_wdt() {
  setup_watchdog(WDT_INTERVAL);
}

// WDT interrupt event function
ISR(WDT_vect) {
  sleep_disable();

  wdt_interrupt_count++;
  wdt_microsecond += wdt_microsecond_per_interrupt;
  // flush microsecond every half an hour to avoid overflow
  if (wdt_microsecond > 1800000000UL) {
    now();
    wdt_microsecond -= prev_microsecond;
    prev_microsecond = 0;
  }

  sleep_enable();
}

void wdt_auto_tune() {
  // skip tuning for the first input after power on
  if (prev_sysTime == 0) {
        prev_sysTime = sysTime - (millis() / 1000); // init prev_sysTime
  } else {
    // check only tune the time if it have pass enough time range (> 1 hour)
    if (wdt_interrupt_count > 3600) {
      // calculation equation: wdt_microsecond_per_interrupt = (sysTime - prev_sysTime) / wdt_interrupt_count * 1,000,000 micro second
      // rephase equation to use a maximum factor (3579) to retain significant value and avoid overflow
      // factor allow 20% adjustment: 2^32 / 1.2 / 1000000 = 3579
      wdt_microsecond_per_interrupt = 3579 * 1000000 / wdt_interrupt_count * (sysTime - prev_sysTime) / 3579;

      // Reset time and stat data after tune
      prev_microsecond = 0;
      wdt_microsecond = 0;
      wdt_interrupt_count = 0;
      prev_sysTime = sysTime;
    }
  }
}

// set system into the sleep state
// system wakes up when watchdog is timed out
void system_sleep() {
  cbi(ADCSRA, ADEN);                   // switch Analog to Digital converter OFF
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); // sleep mode is set here
  sleep_mode();                        // System actually sleeps here
  sbi(ADCSRA, ADEN);                   // switch Analog to Digital converter ON
}

uint32_t get_wdt_microsecond_per_interrupt() {
  return wdt_microsecond_per_interrupt;
}
uint32_t get_wdt_interrupt_count() {
  return wdt_interrupt_count;
}


// Voltage and Temperature related
// Common code for both sources of an ADC conversion
uint16_t readADC() {
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA, ADSC)); // measuring
  return ADC;
}

uint16_t getNewAccumulatedValue(uint16_t accumulatedValue, uint16_t value) {
  if (accumulatedValue == 0) {
    return value << 6; // initial value, multiply by 64
  } else {
    accumulatedValue -= accumulatedValue >> 6; // remove one old value, divide by 64
    accumulatedValue += value; // add new value
  }
  return accumulatedValue;
}

void readRawVcc() {
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  ADMUX = _BV(MUX3) | _BV(MUX2);
  delay(2); // Wait for Vref to settle

  accumulatedRawVcc = getNewAccumulatedValue(accumulatedRawVcc, readADC());
}

uint32_t getVcc() {
  readRawVcc();
  return DEFAULT_VOLTAGE_REF / (accumulatedRawVcc >> 6); // calibrated value, average Vcc in millivolts
}

void readRawTemp() {
  // Measure temperature
  ADMUX = 0xF | _BV( REFS1 ); // ADC4 (Temp Sensor) and Ref voltage = 1.1V;
  delay(2); // Wait for Vref to settle

  accumulatedRawTemp = getNewAccumulatedValue(accumulatedRawTemp, readADC());
}

uint32_t getRawTemp() {
  readRawTemp();
  return accumulatedRawTemp;
}

int32_t getTemp() {
  readRawTemp();

  // Temperature compensation using the chip voltage
  // with 3.0 V VCC is 1 lower than measured with 1.7 V VCC
  uint32_t vcc = getVcc();
  uint16_t compensation = (vcc < 1700) ? 0 : ( (vcc > 3000) ? 1000 : (vcc - 1700) * 10 / 13);

  return (((accumulatedRawTemp * 100000L) - CHIP_TEMP_OFFSET) / CHIP_TEMP_COEFF) + compensation;
}
