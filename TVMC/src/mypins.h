#pragma once
#include "config.h"
#include "Arduino.h"
#include "WS2812B.h"

#define GET_TVSTATE (digitalRead(TVSTATE))
#define ENDSTOP_OUT (digitalRead(ET1))
#define ENDSTOP_IN  (digitalRead(ET2))

class mypins {
    private:
        uint32_t myVal;
        static uint8_t collisionDetected;
        static uint32_t iMitRes;
        static uint32_t iSpecialEffect;
        pixel_t iSpecialEffectColor;
    public:
        mypins( );
        uint8_t setMotorDir( uint8_t dir );
        void setOnboardLed( uint8_t ledState );
        void setMotorSpeed( uint16_t s );
        uint32_t getTVstate( );
        void checkMotorForCollision();
        
        static uint8_t sendCurrentADCValues;
        static uint8_t sendDebugMotorCurrent;
        static uint8_t sendMotorCurrentValues;
        static uint32_t ActualStepTVBoard;

        uint8_t readBtnIn( void );
        uint8_t readBtnOut( void );
        uint8_t readBtnMulti( void );

        uint8_t stateBtnInFnx( void );
        uint8_t stateBtnOutFnx( void );
        uint8_t stateBtnMultiFnx( void );
        uint8_t checkBtnFunctions( void );

        void resetCollisionDetected();
        uint8_t motorCollisionDetected();

        uint8_t getColorChangeTrigger();
        void setColorChangeTrigger();

        void changeToRandomColor();
};