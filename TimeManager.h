#ifndef _TIME_MANAGER_H_
#define _TIME_MANAGER_H_
#include "WDT_Time.h"

enum ChangeType {
  INCRASE,
  DECRASE
};

enum TimeUnit {
  HOUR,
  MIN,
  SEC,
};



class TimeManager {

  private:
    void incrementVal(int *val, byte maxRange) {
      if (*val < maxRange) {
        *val += 1;
      } else {
        *val = 0;
      }
    }

    void decrementVal(int *val, byte maxRange) {
      if (*val <= 0) {
        *val = maxRange;
      } else {
        *val -= 1;
      }
    }

  public:

  TimeUnit nextUnitToChange(TimeUnit current) {
  if (current == HOUR) {
    return MIN;
  } else if (current == MIN) {
    return SEC;
  } else {
    return HOUR;
  }
}

    char* toString() {
      char *time = new char[9];
      time[0] = '0' + hour() / 10; time[1] = '0' + hour() % 10; time[2] = ':';
      time[3] = '0' + minute() / 10; time[4] = '0' + minute() % 10; time[5] = ':';
      time[6] = '0' + second() / 10; time[7] = '0' + second() % 10; time[8] = '\0';
      return time;
    }

    char* toStringWithout(TimeUnit unit) {
      char *time = toString();
      if (unit == HOUR) {
        time[0] = '-', time[1] = '-';
      } else if (unit == MIN) {
        time[3] = '-', time[4] = '-';
      } else if (unit == SEC) {
        time[6] = '-', time[7] = '-';
      }
      return time;
    }

    void change(TimeUnit unit, ChangeType type) {
      if (type == INCRASE) {
        increment(unit);
      } else {
        decrement(unit);
      }
    }

    void increment(TimeUnit unit) {
      byte maxRange = unit == HOUR ? 23 : 59;
      int val;
      if (unit == HOUR) {
        val = hour();
        incrementVal(&val, maxRange);
        setTime(val, minute(), second());
      } else if (unit == MIN) {
        val = minute();
        incrementVal(&val, maxRange);
        setTime(hour(), val, second());
      } else if (unit == SEC) {
        val = second();
        incrementVal(&val, maxRange);
        setTime(hour(), minute(), val);
      }
    }

    void decrement(TimeUnit unit) {
      byte maxRange = unit == HOUR ? 23 : 59;
      int val;
      if (unit == HOUR) {
        val = hour();
        decrementVal(&val, maxRange);
        setTime(val, minute(), second());
      } else if (unit == MIN) {
        val = minute();
        decrementVal(&val, maxRange);
        setTime(hour(), val, second());
      } else if (unit == SEC) {
        val = second();
        decrementVal(&val, maxRange);
        setTime(hour(), minute(), val);
      }
    }

};


#endif
