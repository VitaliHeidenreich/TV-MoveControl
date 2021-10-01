#include <Arduino.h>
#include "Settings.h"
#include <EEPROM.h>

pixel_t Settings::_Color = {100,100,100};
uint8_t Settings::_AutoMove = 0;
uint8_t Settings::_ManMoveDir = 0;
uint8_t Settings::moveEna = 1;

#define EEPROM_SIZE 4

Settings::Settings()
{
        _Color.red = 0;
        _Color.green = 0;
        _Color.blue = 0;

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

/****************************************
 * LED Helligkeit
 ***************************************/
void Settings::setBrightnessPercent(byte Brightness)
{
    Settings::_Brightness = map(Brightness, 0, 100, 0, 255);
}

byte Settings::getBrightnessPercent()
{
    return map(Settings::_Brightness, 0, 255, 0, 100);
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
    if( tikz >= 300 )
        moveEna = 1;
    else
        tikz ++;
}


void Settings::getSavedColor( void )
{
    setColor(EEPROM.read(0),EEPROM.read(1),EEPROM.read(2));
}

void Settings::saveActColor( )
{
    //Zum Testen
    EEPROM.write(0, _Color.red);
    EEPROM.write(1, _Color.green);
    EEPROM.write(2, _Color.blue);
    EEPROM.commit(); 
}

