/***************************************************************************
 * In dieser Funktion wird die Zeit gesetzt und zurückgegeben
 *
 **************************************************************************/

/** Vermeidung Doppeldefinitionen **/
#pragma once

/** Einbinden von relevanten Bibliotheken **/
#include "Arduino.h"
#include "RTClib.h"

#define DEBUG_ZEITMASTER 0

typedef struct {
      uint8_t seconds;
      uint8_t minutes;
      uint8_t hours;
      uint8_t date;
      uint8_t dayOfWeek;
      uint8_t month;
      uint8_t year;
} timedate_t;

class Zeitmaster
{
    public:
        Zeitmaster();
        
        timedate_t getTimeDate();
        void setTimeDate(timedate_t TimeDate);
        void setTimeDate(uint8_t Hours, uint8_t Minutes, uint8_t Seconds, uint8_t Date, uint8_t Month, uint8_t Year);
        
        uint8_t getSeconds();
        uint8_t getMinutes();
        uint8_t getHours();
        uint8_t getDate();
        uint8_t getDayOfWeek();
        uint8_t getMonth();
        uint8_t getYear();

        void printZeitmasterTime();
        void printZeitmasterTimeMinuteByMinute();
        bool timeTrigger();
        
    private:
        timedate_t _TimeDate;
        RTC_DS3231 myRTCDS3231;
        DateTime myTime;
};