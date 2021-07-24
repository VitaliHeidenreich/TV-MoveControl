#pragma once
#include "config.h"
#include "Arduino.h"

#define GET_TVSTATE (digitalRead(TVSTATE))
#define OUT_SENSSTATE (digitalRead(ET2))
#define IN_SENSSTATE (digitalRead(ET1))

class mypins {
    private:
        uint8_t myVal;
    public:
        mypins( );
        uint8_t setMotorDir( uint8_t dir );
        uint8_t getTestPinState( );
        void setOnboardLed( uint8_t ledState );
        void setMotorSpeed( uint8_t s );
        uint8_t getTVstate( void );
        uint8_t getFiltMotCurrent();
        static uint8_t collisionDetected;
        static uint8_t colorchanged;
        static uint32_t iMit;
};