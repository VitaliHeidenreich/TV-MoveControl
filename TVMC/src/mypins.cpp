#include <Arduino.h>
#include "config.h"
#include "mypins.h"

mypins::mypins()
{
    // Fernseher-USB-Stecker
    pinMode(TVSTATE,INPUT);
    // Eingang Entstopps
    pinMode(ET1,INPUT);
    pinMode(ET2,INPUT);
    // Motor 1 Enabler
    ledcSetup(0, FREQUENZ, AUFLOESUNG);
    ledcAttachPin(EN_A, 0);
    // Motor 2 Enabler
    ledcSetup(0, FREQUENZ, AUFLOESUNG);
    ledcAttachPin(EN_B, 1);
    // Richtungspins auf Ausgang schalten
    pinMode(IN_1A,OUTPUT);
    pinMode(IN_2A,OUTPUT);
    pinMode(IN_1B,OUTPUT);
    pinMode(IN_2B,OUTPUT);

    // Test Taste (Nur zum Testen)
    pinMode(TESTPIN,INPUT_PULLUP);

    // Ueberwachungs-LEDs als Ausgang
    pinMode(LED_IN,OUTPUT);
    pinMode(LED_OUT,OUTPUT);

    // Derzeit nicht verwendete Pins
    digitalWrite(IN_2A, 0);
    digitalWrite(IN_2B, 0);
    digitalWrite(EN_B, 0);
}

uint8_t mypins::setMotorDir( uint8_t dir )
{
    uint8_t iRet = 0;

     if( dir )
     {
         iRet = 1;
        // Fernseher ausfahren
        digitalWrite(IN_1A, 0);
        digitalWrite(IN_1B, 1);
        digitalWrite(LED_IN, 0);
        digitalWrite(LED_OUT, 1);
     }
     else
     {
        // Fernseher einfahren
        digitalWrite(IN_1A, 0);
        digitalWrite(IN_1B, 1);
        digitalWrite(LED_IN, 1);
        digitalWrite(LED_OUT, 0);
     }
     
     return iRet;
}

uint8_t mypins::getTestPinState( void )
{
    return (digitalRead(TESTPIN));
}

void mypins::showEndStoppState( uint8_t stoppIn, uint8_t stoppOut)
{
    if(!stoppIn)
        digitalWrite(LED_IN, 1);
    else
        digitalWrite(LED_IN, 0);
    
    if(!stoppOut)
        digitalWrite(LED_OUT, 1);
    else
        digitalWrite(LED_OUT, 0);
}