/** object for recording intervals and recording maximum intervals */

class Stopwatch {
    unsigned long millisAtStart = 0;
    unsigned long millisAtEnd = 0;
    unsigned long durMillis = 0;
    unsigned long maxDurMillis = 0;

  public:
    void markStart() {
      millisAtStart = millis();
    }
    void markEnd() {
      millisAtEnd = millis();
      if(millisAtStart == 0){ //this covers calling start after calling end for the reverse timers.
        millisAtStart = millisAtEnd;
      }
      durMillis = millisAtEnd - millisAtStart;
      if (durMillis > maxDurMillis) {
        maxDurMillis = durMillis;
      }
    }

    unsigned long getStart() {
      return millisAtStart;
    }

    unsigned long getEnd() {
      return millisAtEnd;
    }



    unsigned long getDur() {
      return durMillis;
    }

    unsigned long getDurSeconds() {
      return durMillis/1000UL;
    }

    unsigned long getMaxDur() {
      return maxDurMillis;
    }

    unsigned long getMaxDurSeconds() {
      return maxDurMillis/1000UL;
    }
};
