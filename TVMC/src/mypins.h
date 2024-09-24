#pragma once
#include "config.h"
#include "Arduino.h"

#define GET_TVSTATE (digitalRead(TVSTATE))
#define OUT_SENSSTATE (digitalRead(ET2))
#define IN_SENSSTATE (digitalRead(ET1))

class mypins {
    private:
        uint32_t myVal;
        static uint8_t collisionDetected;
    public:
        mypins( );
        uint8_t setMotorDir( uint8_t dir );
        uint8_t getTestPinState( );
        void setOnboardLed( uint8_t ledState );
        void setMotorSpeed( uint16_t s );
        uint32_t getTVstate( );
        uint8_t checkMotorIsBlocked();

        uint8_t rangeCheck( uint8_t dirOut );
        
        static uint8_t direction;
        static uint8_t sendCurrentADCValues;
        static uint8_t sendDebugMotorCurrent;
        static uint8_t sendMotorCurrentValues;
        static uint32_t ActualStepTVBoard;

        void resetCollisionDetected();
        uint8_t getColorChangeTrigger();
        void setColorChangeTrigger();
};