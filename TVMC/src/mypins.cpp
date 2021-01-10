#include <Arduino.h>
#include "config.h"
#include "mypins.h"

mypins::mypins()
{
    // Fernseher-USB-Stecker
    pinMode(TVSTATE, INPUT);

    // Eingang Entstopps
    pinMode(ET1, INPUT);
    pinMode(ET2, INPUT);

    // Motor 1 und 2 Enabler
    ledcSetup(0, FREQUENZ, AUFLOESUNG);
    ledcAttachPin(EN_A, 0);
    ledcAttachPin(EN_B, 0);

    // Richtungspins auf Ausgang schalten
    pinMode(IN_1A, OUTPUT);
    pinMode(IN_2A, OUTPUT);
    pinMode(IN_1B, OUTPUT);
    pinMode(IN_2B, OUTPUT);

    // Test Taste (Nur zum Testen)
    pinMode(TESTPIN,INPUT_PULLUP);

    // Ueberwachungs-LEDs als Ausgang
    pinMode(LED_IN,  OUTPUT);
    pinMode(LED_OUT, OUTPUT);

    // Derzeit nicht verwendete Pins
    digitalWrite(IN_2A, 0);
    digitalWrite(IN_2B, 0);
}

uint8_t mypins::setMotorDir( uint8_t dir )
{
     if( dir )
     {
        // Fernseher ausfahren
        digitalWrite(IN_1A, 0);
        digitalWrite(IN_2A, 1);
        return 1;
     }
     else
     {
        // Fernseher einfahren
        digitalWrite(IN_1A, 1);
        digitalWrite(IN_2A, 0);
        return 0;
     }
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

// Zum Einstellen der Motorgeschwindigkeit (beide Motoren, falls vorhanden)
void mypins::setMotorSpeed( uint8_t speed )
{
    ledcWrite(0, speed);
}

uint8_t mypins::getTVstate( void )
{
    uint8_t iRet = 0;
    uint32_t iMit = 0;
    static uint16_t myVal[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    static uint8_t valMem = 0;

    // Read analog value
    myVal[valMem] = analogRead(ANALOGPIN);;

    // Bilden des Mittelwertes
    for(uint8_t i = 0; i < 20; i++)
        iMit = iMit +  myVal[i];
    iMit = iMit / 20;

    Serial.println(iMit);

    // Set to next Position
    if( valMem < 19 )
        valMem ++;
    else
        valMem = 0;
    
    return iRet;
}