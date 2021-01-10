#include "config.h"
#include "Arduino.h"

#define GET_TVSTATE (digitalRead(TVSTATE))
#define GET_ET1 (digitalRead(ET1))
#define GET_ET2 (digitalRead(ET2))

class mypins {
    private:
        uint8_t myVal;
    public:
        mypins( );
        uint8_t setMotorDir( uint8_t dir );
        uint8_t getTestPinState( );
        void showEndStoppState( uint8_t stoppIn, uint8_t stoppOut);
        void setMotorSpeed( uint8_t s );
        uint8_t getTVstate( void );
};