#include <Arduino.h>
#include "config.h"
#include "mypins.h"

uint8_t mypins::collisionDetected = 0;
uint8_t mypins::colorchanged = 1;
uint32_t mypins::iMit = 2180;

mypins::mypins()
{
    // Fernseher-USB-Stecker
    pinMode(TVSTATE, INPUT);

    // Eingang Entstopps
    pinMode(ET1, INPUT);
    pinMode(ET2, INPUT);

    // Motor 1 und 2 Enabler
    ledcSetup(0, FREQUENZ, AUFLOESUNG);
    // ledcAttachPin(EN_A, 0);
    ledcAttachPin(EN_B, 0);

    // Richtungspins auf Ausgang schalten
    // pinMode(IN_1A, OUTPUT);
    // pinMode(IN_2A, OUTPUT);
    pinMode(IN_1B, OUTPUT);
    pinMode(IN_2B, OUTPUT);

    // Test Taste (Nur zum Testen)
    pinMode(TESTPIN,INPUT_PULLUP);

    // Ueberwachungs-LEDs als Ausgang
    pinMode(LED_IN,  OUTPUT);
    pinMode(LED_OUT, OUTPUT);

    // Motor current meassurement pin
    pinMode(CURRENTMEASPIN, INPUT);
    pinMode(TVPIN, ANALOG);
}

uint8_t mypins::setMotorDir( uint8_t dir )
{
     if( !dir )
     {
        // Fernseher ausfahren
        digitalWrite(IN_1B, 0);
        digitalWrite(IN_2B, 1);
        return 1;
     }
     else
     {
        // Fernseher einfahren
        digitalWrite(IN_1B, 1);
        digitalWrite(IN_2B, 0);
        return 0;
     }
}

uint8_t mypins::getTestPinState( void )
{
    return (digitalRead(TESTPIN));
}

void mypins::setOnboardLed( uint8_t ledState )
{
    if(!ledState)
        digitalWrite(LED_OUT, 1);
    else
        digitalWrite(LED_OUT, 0);
}

// Zum Einstellen der Motorgeschwindigkeit (beide Motoren, falls vorhanden)
void mypins::setMotorSpeed( uint8_t speed )
{
    ledcWrite(0, speed);
}

uint8_t mypins::getTVstate( void )
{
    //uint32_t iMit = 0;
    static uint32_t myVal[TV_MEASNUMB] = {2180};
    static uint8_t z = 0;
    static uint8_t iRet = 0;

    // Read analog value
    // Befüllung des Ringbuffers (kopieren von vorne nach hinten, beginnend am Ende)
    for (uint8_t i = (TV_MEASNUMB - 1); i > 0; i--)
    {
        myVal[i] = myVal[i - 1];
    }

    // Neues Zeichen in den Buffer[0] schieben
    myVal[0] = analogRead(TVPIN);

    // Bilden des Mittelwertes
    for(uint8_t i = 0; i < TV_MEASNUMB; i++)
        iMit = iMit +  myVal[i];
    iMit = iMit / TV_MEASNUMB;

    if(z >= TV_MEASNUMB)
        z=1;
    if(z==(TV_MEASNUMB - 1))
        Serial.println(iMit);
    
    z++;

    if( iMit > TVONVALUE )
        iRet = 1;
    else if(iMit < TVOFFVALUE )
        iRet = 0;
    else
        iRet = iRet;

    return iRet;
}

uint8_t mypins::getFiltMotCurrent()
{
    static uint16_t currentValues[CURRENTNUMVAL] = {0};
    uint32_t medianCValue = 0;

    //static uint8_t test = 0;

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

//to del
    // if( test == 9)
    //     Serial.println(medianCValue);
    // test ++;
    // if( test >= 10 )
    //     test = 0;
// end del
    
    // testing the measured median val
    if( OVERCURDETVAL <= medianCValue )
        return 1;
    else
        return 0;
}