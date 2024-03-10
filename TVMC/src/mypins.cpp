#include <Arduino.h>
#include "config.h"
#include "mypins.h"

uint8_t mypins::collisionDetected = 0;
uint8_t mypins::sendCurrentADCValues = 0;
uint8_t mypins::colorchanged = 1;
uint32_t mypins::iMit = 2180;
uint32_t mypins::ActualStepTVBoard = 0;

uint8_t mypins::direction = 0;



mypins::mypins()
{
    // Test pin
    pinMode(TESTBUTTON, INPUT_PULLUP);

    // Eingang Entstopps
    pinMode(ET1, INPUT);
    pinMode(ET2, INPUT);

    // Richtungspins auf Ausgang schalten
    pinMode(DRK, OUTPUT);
    pinMode(BRK, OUTPUT);

    // Test Taste (Nur zum Testen)
    pinMode(TESTPIN,INPUT_PULLUP);

    // Ueberwachungs-LEDs als Ausgang
    pinMode(LED_MOT_ON, OUTPUT);

    // Motor current meassurement pin
    pinMode(CURRENTMEASPIN, INPUT);
    pinMode(TVPIN, ANALOG);
}

// Nur zum Testen
uint8_t mypins::rangeCheck( uint8_t dirOut )
{
    uint8_t iRet = 1;
    if ( ((dirOut == 1) && (mypins::ActualStepTVBoard >= MAX_STEP_TV)) || ((dirOut == 0) && (mypins::ActualStepTVBoard == 0)) )
        iRet = 0;
    return iRet;
}

/**
 * @brief dir == 1 --> OUT
 * 
 * @param dir 
 * @return uint8_t 
 */
uint8_t mypins::setMotorDir( uint8_t dir )
{
    if ( getTestPinState() )
        (dir == 1) ? dir = 0 : dir = 1;

    if( dir )
    {
        // Fernseher ausfahren
        digitalWrite(DRK, 0); 
        direction = 1;
    }
    else
    {
        // Fernseher einfahren
        digitalWrite(DRK, 1);
        direction = 0;
    }
    return direction;
}

uint8_t mypins::getTestPinState( void )
{
    return (digitalRead(TESTBUTTON));
}

// Zum Einstellen der Motorgeschwindigkeit (beide Motoren, falls vorhanden)
void mypins::setMotorSpeed( uint16_t speed )
{
    if(speed > 0)
    {
        digitalWrite(LED_MOT_ON, 1);
        digitalWrite(BRK, 1);
    } 
    else
    {
        digitalWrite(LED_MOT_ON, 0);
        digitalWrite(BRK, 0);
    }
}

uint32_t mypins::getTVstate( void )
{
    static uint32_t myVal[TV_MEASNUMB] = {2180};
    static uint32_t z = 0;
    static uint32_t iRet = 0;

    static uint32_t counter = 100;

    // Read analog value
    // Befüllung des Ringbuffers (kopieren von vorne nach hinten, beginnend am Ende)
    for (uint32_t i = (TV_MEASNUMB - 1); i > 0; i--)
        myVal[i] = myVal[i - 1];

    // Neues Zeichen in den Buffer[0] schieben
    myVal[0] = analogRead(TVPIN);

    // Bilden des Mittelwertes
    for(uint32_t i = 0; i < TV_MEASNUMB; i++)
        iMit = iMit +  myVal[i];
    iMit = iMit / TV_MEASNUMB;

    if(z >= TV_MEASNUMB)
    {
        z=1;
    }
    z++;

    if( iMit > TVONVALUE )
    {
        iRet = 1;
    }
    else if(iMit < TVOFFVALUE )
    {
        iRet = 0;
    }
    else
    {
        iRet = iRet;
    }

    //Debug Nachricht
    if( mypins::sendCurrentADCValues == 1 )
    {
        if( counter >= 100 )
        {
            //Serial.println( iMit );   
            counter = 0;
        }
        counter++;
    }

    return iRet;
}

uint8_t mypins::getFiltMotCurrent()
{
    static uint16_t currentValues[CURRENTNUMVAL] = {0};
    uint32_t medianCValue = 0;

    #if DEBUG == 7
        static uint8_t test = 0;
    #endif

    //Befüllung des Ringbuffers (kopieren von vorne nach hinten, beginnend am Ende)
    for (uint8_t i = (CURRENTNUMVAL - 1); i > 0; i--)
    {
        currentValues[i] = currentValues[i - 1];
    }

    // Neues Zeichen in den Buffer[0] schieben
    currentValues[0] = analogRead(CURRENTMEASPIN);

    // Bilden des Mittelwertes
    for (uint8_t i = 0; i < CURRENTNUMVAL; i++)
    {
        medianCValue += currentValues[i];
    }

    medianCValue /= CURRENTNUMVAL;

    #if DEBUG == 7
        if( test == 9)
        {
            Serial.println(medianCValue);
        } 
        test ++;
        if( test >= 10 )
            test = 0;
    #endif
    
    // testing the measured median val
    if( OVERCURDETVAL <= medianCValue )
        return 1;
    else
        return 0;
}