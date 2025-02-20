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
uint32_t mypins::iMitRes = 2500;
uint32_t mypins::iSpecialEffect = 0;

//_TurnOnCurrentValue
Settings *settings;


mypins::mypins()
{
    settings = new Settings();

    // Eingang Entstopps
    pinMode(ET1, INPUT);
    pinMode(ET2, INPUT);

    // Richtungspins auf Ausgang schalten
    pinMode(DRK, OUTPUT);
    pinMode(BRK, OUTPUT);

    // Test Taste (Nur zum Testen)
    pinMode(BTN_IN, INPUT_PULLUP);
    pinMode(BTN_OUT, INPUT_PULLUP);
    pinMode(BTN_MULTI, INPUT_PULLUP);

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
    static uint8_t lastHourVal = 12;
    uint8_t actHourVal = 13;
    actHourVal = settings->getTimeHour();

    if( settings->colorchanged == 1 || (lastHourVal != actHourVal) )
    {
        settings->colorchanged = 0;
        lastHourVal = actHourVal;
        return 1;
    }
    else
    {
        return 0;
    }
}

void mypins::setColorChangeTrigger()
{
    settings->colorchanged = 1;
}


/*****************************************************************************************
 * @brief dir == 1 --> OUT
 * 
 * @param dir 
 * @return uint8_t 
 ****************************************************************************************/
uint8_t mypins::setMotorDir( uint8_t tvState )
{
    uint8_t direction_out;
    // digitalRead(0)
    if( tvState == ON )
    {
        // Fernseher ausfahren
        if( MOTOR_DIR_INV )
            digitalWrite(DRK, 0);
        else
            digitalWrite(DRK, 1);
        direction_out = 1;
    }
    else
    {
        // Fernseher einfahren
        if( MOTOR_DIR_INV )
            digitalWrite(DRK, 1);
        else
            digitalWrite(DRK, 0);
        direction_out = 0;
    }
    
    return direction_out;
}

// Zum Einstellen der Motorgeschwindigkeit (beide Motoren, falls vorhanden)
void mypins::setMotorSpeed( uint16_t speed )
{
    if(speed > 0)
    {
        digitalWrite(BRK, 1);
    } 
    else
    {
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
            Serial.print( "TVC Nr. " );
            Serial.print( debugVarCount );
            Serial.print( " : " );
            Serial.print( mittelWert );
            Serial.print( "\t(" );
            Serial.print( settings->getSavedTurnOffValue_D() );
            Serial.print( ":" );
            Serial.print( settings->getSavedTurnOnValue_D() );
            Serial.print( ")\n" );
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
            Serial.print( "MC  Nr." ); Serial.print( debugVarCount ); Serial.print( ": " );Serial.println( medianCValue ); 
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
    static uint8_t  iRet = 0; 
    static uint32_t iMit = 0;

    // Read analog value
    // Befüllung des Ringbuffers (kopieren von vorne nach hinten, beginnend am Ende)
    for (uint32_t i = (TV_MEASNUMB - 1); i > 0; i--)
        myVal[i] = myVal[i - 1];

    // Neues Zeichen in den Buffer[0] schieben
    myVal[0] = analogRead( TVPIN );

    // Bilden des Mittelwertes
    for(uint32_t i = 0; i < TV_MEASNUMB; i++)
        iMit = iMit +  myVal[i];
    
    iMit = iMit / TV_MEASNUMB;
    iMitRes = iMit;
    sendDebugValueCurrentTVBoard( iMitRes );
    
    if( ( iMit > settings->getSavedTurnOnValue_D() ) )
    {
        iRet = ON;
    }
    else if( iMit < settings->getSavedTurnOffValue_D() )
    {
        iRet = OFF;
    }
    else
    {
        return iRet;
    }
    
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
void mypins::checkMotorForCollision()
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
    if( settings->readSavedMotorCollisionValue_Direct() <= medianCValue )
        mypins::collisionDetected = COLLISION;
}

uint8_t mypins::motorCollisionDetected()
{
    return mypins::collisionDetected;
}

/*****************************************************************************************
 * @brief Reset the collision detected variable to 0 (static variable)
 * 
 * @param local static variable 
 * @return no
 ****************************************************************************************/
void mypins::resetCollisionDetected()
{
    mypins::collisionDetected = NO_COLLISION;
    mypins::setColorChangeTrigger( );
}

uint8_t mypins::readBtnIn( void )
{
    if(digitalRead(BTN_IN))
        return 0;
    return 1;
}
uint8_t mypins::readBtnOut( void )
{
    if(digitalRead(BTN_OUT))
        return 0;
    return 1;
}
uint8_t mypins::readBtnMulti( void )
{
    if(digitalRead(BTN_MULTI))
        return 0;
    return 1;
}

/*****************************************************************************************
 * @brief Read and use the button state
 * 
 * @return retur shown a event was found or not
 ****************************************************************************************/
uint8_t mypins::stateBtnInFnx()
{
    static uint8_t lastState = 0;
    static uint32_t time = 0;

    if( !readBtnMulti( ) )
    {
        if( readBtnIn() )
        {
            time += INTERRUPTTIME;

            if( time > SHORTTIMEPUSH )
            {
                settings->_AutoMove = 0;
                settings->_ManMoveDir = 1;
                lastState = 2;
            }
        }
        else
        {
            // Test ob es ein Kurzdruck war --> Toggle
            if( time > DEBOUNCETIME && time <= SHORTTIMEPUSH )
            {
                if( lastState == 1 )
                {
                    settings->_AutoMove = 0;
                    settings->_ManMoveDir = 0;
                    lastState = 0;
                }
                else
                {
                    settings->_AutoMove = 0;
                    settings->_ManMoveDir = 1;
                    lastState = 1;
                }
            }

            if( lastState == 2 )
            {
                settings->_AutoMove = 0;
                settings->_ManMoveDir = 0;
                lastState = 0;
            }

            time = 0;
        }
    }
    else
    {
        lastState = 0;
    }

    return 0;
}

/*****************************************************************************************
 * @brief Read and use the button state
 * 
 * @return retur shown a event was found or not
 ****************************************************************************************/
uint8_t mypins::stateBtnOutFnx()
{
    static uint8_t lastState = 0;
    static uint32_t time = 0;

    if( !readBtnMulti( ) )
    {
        if( readBtnOut( ) )
        {
            time += INTERRUPTTIME;

            if( time > SHORTTIMEPUSH )
            {
                settings->_AutoMove = 0;
                settings->_ManMoveDir = 2;
                lastState = 2;
            }
        }
        else
        {
            // Test ob es ein Kurzdruck war --> Toggle
            if( time > DEBOUNCETIME && time <= SHORTTIMEPUSH )
            {
                if( lastState == 1 )
                {
                    settings->_AutoMove = 0;
                    settings->_ManMoveDir = 0;
                    lastState = 0;
                }
                else
                {
                    settings->_AutoMove = 0;
                    settings->_ManMoveDir = 2;
                    lastState = 1;
                }
            }

            if( lastState == 2 )
            {
                settings->_AutoMove = 0;
                settings->_ManMoveDir = 0;
                lastState = 0;
            }

            time = 0;
        }
    }
    else
    {
        lastState = 0;
    }

    return 0;
}

/*****************************************************************************************
 * @brief Read and use the button state
 * 
 * @return retur shown a event was found or not
 ****************************************************************************************/
uint8_t mypins::stateBtnMultiFnx()
{
    static uint32_t time = 0;
    uint8_t bntEvent = 0;

    if( readBtnMulti( ) )
    {
        time += INTERRUPTTIME;

        if ( time > 2000 )
        {
            // Ein- oder Ausschaltschwelle anlernen
            if( readBtnOut( ) )
            {
                // Einschaltschwelle setzen
                if( (iMitRes - 8) > settings->getSavedTurnOffValue_D( ) )
                {
                    settings->saveTurnOnValue( iMitRes - settings->getDivTurnOnOffValue( ) );
                    iSpecialEffect = 250;
                    iSpecialEffectColor = GREEN;
                }
                bntEvent = 1;
                Serial.print("Anlernen TurnOn\n");
            }
            if( readBtnIn( ) )
            {
                // Ausschaltschwelle setzen
                if( (iMitRes + 8) < settings->getSavedTurnOnValue_D( ) )
                {
                    settings->saveTurnOffValue( iMitRes + settings->getDivTurnOnOffValue( ) );
                }
                bntEvent = 1;
                Serial.print("Anlernen TurnOff\n");
            }
            if( bntEvent )
            {
                time = 0;
            }
        }
    }
    else
    {
        if( time > DEBOUNCETIME && time <= 1000 )
        {
            // Change color
            mypins::changeToRandomColor();
        }
        else if( time > 1000 && time <= 4000 )
        {
            // Automove enabled
            settings->_AutoMove = 1;
            bntEvent = 1;
        }
        else if( time > 4000 && time < 20000 )
        {
            // Reset collision
            mypins::resetCollisionDetected();
            bntEvent = 1;
        }
        else
        {
            // nop
        }

        time = 0;
    }

    if( bntEvent )
    {
        time = 0;
        bntEvent = 0;
        return 1;
    }
    else
    {
        return 0;
    }
}

uint8_t mypins::checkBtnFunctions()
{
    stateBtnInFnx();
    stateBtnOutFnx();
    stateBtnMultiFnx();

    return 0;
}

void mypins::changeToRandomColor()
{
    static uint8_t iNextState = ((uint8_t)esp_random())%3;
    iNextState ++;
    if( iNextState > 10 )
        iNextState = 0;

    if( !motorCollisionDetected( ) )
    {
        switch ( iNextState )
        {
        case 0:
            settings->setColor(0xff, 0, 0);
            break;
        case 1:
            settings->setColor(0xff, 0x80, 0);
            break;
        case 2:
            settings->setColor(0, 0, 0xff);
            break;
        case 3:
            settings->setColor(0, 0xff, 0);
            break;      
        case 4:
            settings->setColor(0xff, 0, 0xff);
            break;
        case 5:
            settings->setColor(0, 0xff, 0xff);
            break;
        case 6:
            settings->setColor(0xff, 0xa5, 0);
            break;
        case 7:
            settings->setColor(0xcd, 0x85, 0x3f);
            break;
        case 8:
            settings->setColor(0xe6, 0xe6, 0xfa);
            break; 
        case 9:
            settings->setColor(0xcd, 0x85, 0x3f);
            break;
        case 10:
            settings->setColor(0, 0, 0);
            break; 
        default:
            settings->setColor(10, 10, 10);
            break;
        }
        mypins::setColorChangeTrigger( );
    }
}