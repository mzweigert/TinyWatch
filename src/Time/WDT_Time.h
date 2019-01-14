/*
 * Revised time function count time by WDT timer instead of millis() function
 * Add WDT and power related function
 * Ref.:
 * time function v1.4: https://github.com/PaulStoffregen/Time
 * WDT and power related: http://www.re-innovation.co.uk/web12/index.php/en/blog-75/306-sleep-modes-on-attiny85
 * readVcc: http://forum.arduino.cc/index.php?topic=222847.0
 * Internal temperature sensor: http://21stdigitalhome.blogspot.hk/2014/10/trinket-attiny85-internal-temperature.html
*/

#define TIME_ADDR 0 // EEPROM address for storing the time you set, it can help restore the time easier after change the battery
#define WDT_INTERVAL 6 // ~1 second
#define DEFAULT_WDT_MICROSECOND 1000000UL // put your calibrated value here, should be within +/- 10000 of 1000000 microseconds

/* calibrate voltage reference
 *  step 1: comment the follow 2 #define lines
 *  step 2: program the watch
 *  step 3: record the debug screen V reading and multimeter measured voltage
 *  step 4: uncomment the follow 2 #define lines and fill the reading value
 *  step 5: re-program the watch
 */
#define DEBUG_SCREEN_V 2860 // put your screen reading here
#define MULTI_METER_VOLTAGE 2900 // put your multimeter reading here (in millivolt)
#ifdef DEBUG_SCREEN_VOLTAGE // use calibrated value
  #define 1125300UL / DEBUG_SCREEN_V * MULTI_METER_VOLTAGE
#else // use default value
  #define DEFAULT_VOLTAGE_REF 1125300UL // 1.1 * 1023 * 1000
#endif

/* calibrate temperature constant
 *  step 1: record the debug screen T reading 2 times in different temperature condition
 *  step 2: uncomment the follow 4 #define lines and fill the values
 */
#define DEBUG_SCREEN_T_1  19919L
#define TEMPERATURE_1     21000L
#define DEBUG_SCREEN_T_2  19968L
#define TEMPERATURE_2     20500L
#ifdef DEBUG_SCREEN_T_1 // use calibrated value
  #define CHIP_TEMP_COEFF ((DEBUG_SCREEN_T_1 - DEBUG_SCREEN_T_2) * 100000L / (TEMPERATURE_1 - TEMPERATURE_2))
  #define CHIP_TEMP_OFFSET ((DEBUG_SCREEN_T_1 * 100000L) - (TEMPERATURE_1 * CHIP_TEMP_COEFF))
#else // use default value
  #define CHIP_TEMP_COEFF 6880L // 64 raw samples, 1.075 * 64 * 10000
  #define CHIP_TEMP_OFFSET 1746560000L // 64 raw samples, 272.9 *64 * 10000
#endif
// Calibration of the temperature sensor has to be changed for your own ATtiny85
// per tech note: http://www.atmel.com/Images/doc8108.pdf

#ifndef _Time_h
#ifdef __cplusplus
#define _Time_h

#include <inttypes.h>
#ifndef __AVR__
#include <sys/types.h> // for __time_t_defined, but avr libc lacks sys/types.h
#endif


#if !defined(__time_t_defined) // avoid conflict with newlib or other posix libc
typedef unsigned long time_t;
#endif

// This ugly hack allows us to define C++ overloaded functions, when included
// from within an extern "C", as newlib's sys/stat.h does.  Actually it is
// intended to include "time.h" from the C library (on ARM, but AVR does not
// have that file at all).  On Mac and Windows, the compiler will find this
// "Time.h" instead of the C library "time.h", so we may cause other weird
// and unpredictable effects by conflicting with the C library header "time.h",
// but at least this hack lets us define C++ functions as intended.  Hopefully
// nothing too terrible will result from overriding the C library header?!
extern "C++" {
  typedef enum {
    timeNotSet, timeSet
  }  timeStatus_t ;

  typedef enum {
    tmSecond, tmMinute, tmHour, tmNbrFields
  } tmByteFields;

  typedef struct  {
    uint8_t Second;
    uint8_t Minute;
    uint8_t Hour;
  }   tmElements_t, TimeElements, *tmElementsPtr_t;

  typedef time_t(*getExternalTime)();
  //typedef void  (*setExternalTime)(const time_t); // not used in this version


  /*==============================================================================*/
  /* Useful Constants */
#define SECS_PER_MIN  (60UL)
#define SECS_PER_HOUR (3600UL)
#define SECS_PER_DAY (3600UL * 24)
  /* Useful Macros for getting elapsed time */
#define numberOfSeconds(_time_) (_time_ % SECS_PER_MIN)
#define numberOfMinutes(_time_) ((_time_ / SECS_PER_MIN) % SECS_PER_MIN)
#define numberOfHours(_time_) (( _time_% SECS_PER_DAY) / SECS_PER_HOUR)
  // The following macros are used in calculating alarms and assume the clock is set to a date later than Jan 1 1971
  // Always set the correct time before settting alarms


  /* Useful Macros for converting elapsed time to a time_t */
#define minutesToTime_t ((M)) ( (M) * SECS_PER_MIN)

  /*============================================================================*/
  /*  time and date functions   */
  uint8_t  hour();            // the hour now
  uint8_t  hour(time_t t);    // the hour for the given time
  uint8_t  hourFormat12();    // the hour now in 12 hour format
  uint8_t  hourFormat12(time_t t); // the hour for the given time in 12 hour format
  bool     isAM();            // returns true if time now is AM
  bool     isAM(time_t t);    // returns true the given time is AM
  bool     isPM();            // returns true if time now is PM
  bool     isPM(time_t t);    // returns true the given time is PM
  uint8_t  minute();          // the minute now
  uint8_t  minute(time_t t);  // the minute for the given time
  uint8_t  second();          // the second now
  uint8_t  second(time_t t);  // the second for the given time

  time_t  now();              // return the current time as seconds since Jan 1 1970
  void    setTime(time_t t);
  void    setTime(uint8_t hr, uint8_t min, uint8_t sec);
  void    adjustTime(long adjustment);

  /* date strings */
#define dt_MAX_STRING_LEN 9 // length of longest date string (excluding terminating null)

  /* low level functions to convert to and from system time                     */
  void breakTime(time_t time, tmElements_t &tm);  // break time_t into elements
  time_t makeTime(tmElements_t &tm);  // convert time elements into time_t

} // extern "C++"
#endif // __cplusplus
#endif /* _Time_h */

void init_wdt();

/* WDT and power related */
void wdt_setup();
void wdt_auto_tune();
void system_sleep();
uint32_t get_wdt_microsecond_per_interrupt(); // debug use only
uint32_t get_wdt_interrupt_count(); // debug use only

// Voltage and Temperature related
static uint16_t accumulatedRawVcc = 0;
static uint16_t accumulatedRawTemp = 0;
void readRawVcc(); // debug use only
uint32_t getVcc();
void readRawTemp(); // debug use only
uint32_t getRawTemp();
int32_t getTemp();
