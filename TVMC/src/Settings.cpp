#include <Arduino.h>
#include "Settings.h"
#include <EEPROM.h>

pixel_t Settings::_Color = {100,100,100};
uint16_t Settings::_UpperCollisionADCValue = 1422;
uint16_t Settings::_TurnOnCurrentValue = 1405;
uint8_t Settings::_AutoMove     = 1;
uint8_t Settings::_ManMoveDir   = 0;
uint8_t Settings::initTimeOver  = 0;

#define EEPROM_SIZE 10

Settings::Settings()
{
        _Color = {100,100,100};
        _UpperCollisionADCValue = 1422;
        _TurnOnCurrentValue = 1405;
        EEPROM.begin(EEPROM_SIZE);
}

/****************************************
 * LED Farbe
 ***************************************/
void Settings::setColor(pixel_t c)
{
    Settings::_Color = c;
}

void Settings::setColor(byte red, byte green, byte blue)
{
    Settings::_Color.red = red;
    Settings::_Color.green = green;
    Settings::_Color.blue = blue;
}

pixel_t Settings::getColor()
{
    return Settings::_Color;
}


uint8_t Settings::blinkCollision(uint8_t on)
{
    static uint8_t blinkTikz = 0;
    static uint8_t iRet = 0;


    if( blinkTikz == 50 )
    {
        blinkTikz = 0;
        if(iRet)
        {
            iRet = 0;
        }
        else
        {
            iRet = 1;
        }
    }
    else
    {
        blinkTikz ++;
    }

    return  iRet;
}

/**********************************************************************
 * Beim Aufstart soll eine vorgegebene Zeit abgewartet werden,
 * damit der Stromsensor plausible Werte bekommt
 *********************************************************************/
void Settings::startUpTimer()
{
    static uint16_t tikz = 0;
    if( tikz >= 100 )
        initTimeOver = 1;
    else
        tikz ++;
}

/**********************************************************************
 * Letzte eingestellte Farbe einlesen und Speichern
 * NV RAM: 0 1 2
 *********************************************************************/
void Settings::getSavedColor( void )
{
    setColor(EEPROM.read(0),EEPROM.read(1),EEPROM.read(2));
}

void Settings::saveActColor( )
{
    EEPROM.write(0, _Color.red);
    EEPROM.write(1, _Color.green);
    EEPROM.write(2, _Color.blue);
    EEPROM.commit(); 
}

/**********************************************************************
 * Einstellen der Kollisionserkennung
 * NV RAM: Val 3 4
 *********************************************************************/
uint16_t Settings::getSavedUpperCollisionADCValue( void )
{
    return ( (EEPROM.read(3)*100) + EEPROM.read(4) );
}

void Settings::saveUpperCollisionADCValue( uint16_t val )
{
    Settings::_UpperCollisionADCValue = val;
    EEPROM.write(3, (uint8_t)(val/100));
    EEPROM.write(4, val%100);
    EEPROM.commit(); 
}

/**********************************************************************
 * Einstellen der Einschaltschwelle
 * NV RAM: Val 5 6
 *********************************************************************/
uint16_t Settings::getSavedTurnOnValue( void )
{
    return ( (EEPROM.read(5)*100) + EEPROM.read(6) );
}

void Settings::saveTurnOnValue( uint16_t val )
{
    Settings::_TurnOnCurrentValue = val;
    EEPROM.write(5, (uint8_t)(val/100));
    EEPROM.write(6, val%100);
    EEPROM.commit(); 
}
