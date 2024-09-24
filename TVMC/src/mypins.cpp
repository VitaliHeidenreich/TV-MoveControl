#include <Arduino.h>
#include "config.h"
#include "mypins.h"
#include "Settings.h"
#include "defines.h"

uint8_t mypins::collisionDetected = 0;
uint8_t mypins::sendCurrentADCValues = 0;
uint8_t mypins::sendMotorCurrentValues = 0;
uint8_t mypins::sendDebugMotorCurrent = 0;
uint32_t mypins::ActualStepTVBoard = 0;

uint8_t mypins::direction = 0;

//_TurnOnCurrentValue
Settings *settings;


mypins::mypins()
{
    settings = new Settings();
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

/*****************************************************************************************
 * @brief To get the info that the color was changed. After read the trigger will be reseted
 * 
 * @param local static variable
 * @return the trigger state, after read it will be resetet to 0
 ****************************************************************************************/
uint8_t mypins::getColorChangeTrigger()
{
    if( settings->colorchanged == 1)
    {
        settings->colorchanged = 0;
        return 1;
    }
    else
    {
        return 0;
    }
}

/*****************************************************************************************
 * @brief Reset the collision detected variable to 0 (static variable)
 * 
 * @param local static variable 
 * @return no
 ****************************************************************************************/
void mypins::resetCollisionDetected()
{
    collisionDetected = NO_COLLISION;
}

// Nur zum Testen
uint8_t mypins::rangeCheck( uint8_t dirOut )
{
    uint8_t iRet = 1;
    if (    ((dirOut == 1) && (ActualStepTVBoard >= MAX_STEP_TV)) || 
            ((dirOut == 0) && (ActualStepTVBoard == 0)) )
        iRet = 0;
    return iRet;
}

/*****************************************************************************************
 * @brief dir == 1 --> OUT
 * 
 * @param dir 
 * @return uint8_t 
 ****************************************************************************************/
uint8_t mypins::setMotorDir( uint8_t state )
{
    if( state == ON )
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

void sendDebugValueCurrentTVBoard( uint32_t mittelWert )
{
    static uint32_t debugVar = 0; // für Anfrage der Mittelwerte aus der UART
    static uint32_t debugVarCount = 1;

    if( mypins::sendCurrentADCValues == 1 )
    {
        if( debugVar >= 100 )
        {
            Serial.print( mittelWert ); Serial.print( "TVC Nr.: " ); Serial.println( debugVarCount );
            debugVarCount ++;
            debugVar = 0;
        }
        else
        {
            debugVar ++;
        }
    }
    else
    {
        debugVarCount = 1;
    }
}

void sendDebugValueCurrentMotor( uint32_t medianCValue )
{
    static uint32_t debugVar = 0; // für Anfrage der Mittelwerte aus der UART
    static uint32_t debugVarCount = 1;

    if( mypins::sendMotorCurrentValues == 1 )
    {
        if( debugVar >= 100 )
        {
            Serial.print( medianCValue ); Serial.print( "MC  Nr.: " ); Serial.println( debugVarCount );
            debugVarCount ++;
            debugVar = 0;
        }
        else
        {
            debugVar ++;
        }
    }
    else
    {
        debugVarCount = 1;
    }
}

/****************************************************************************************
 * @brief Read the tv state out of the current measurement (filtered)
 * 
 * @param local static variable 
 * @return tv on (1) and tv off (0) 
 ****************************************************************************************/
uint32_t mypins::getTVstate( )
{
    static uint32_t myVal[TV_MEASNUMB] = { settings->getSavedTurnOffValue_D() };
    static uint8_t iRet = 0; 
    static uint32_t iMit = 0;

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

    sendDebugValueCurrentTVBoard( iMit );
    
    if( iMit > settings->getSavedTurnOnValue_D() )
        iRet = ON;
    else if( iMit < settings->getSavedTurnOffValue_D() )
        iRet = OFF;
    else
        return iRet;

    return iRet;
}

/****************************************************************************************
 * @brief If the current is exeeting the current limit as a median value the 
 *          motor is blocked. so the static paramter collisionDetected will be set
 *          and motor shall be stoped.
 * 
 * @param local static variable 
 * @return motor blocked (1) and no issue (0) 
 ****************************************************************************************/
uint8_t mypins::checkMotorIsBlocked()
{
    static uint16_t currentValues[CURRENTNUMVAL] = {0};
    uint32_t medianCValue = 0;

    //Befüllung des Ringbuffers (kopieren von vorne nach hinten, beginnend am Ende)
    for (uint8_t i = (CURRENTNUMVAL - 1); i > 0; i--)
        currentValues[i] = currentValues[i - 1];

    /* write new value to the Buffer[0]*/
    currentValues[0] = analogRead(CURRENTMEASPIN);

    /* calculation median value */
    for (uint8_t i = 0; i < CURRENTNUMVAL; i++)
        medianCValue += currentValues[i];

    medianCValue = medianCValue / CURRENTNUMVAL;

    sendDebugValueCurrentMotor( medianCValue );

    /* testing the measured median val */
    /* clearing the collision detected only by other function */
    if( settings->getSavedUpperCollisionADCValue_D() <= medianCValue )
        collisionDetected = COLLISION;

    return collisionDetected;
}