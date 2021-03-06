#include <Arduino.h>
#include "Settings.h"

pixel_t Settings::_Color = {100,100,100};
uint8_t Settings::_AutoMove = 0;
uint8_t Settings::_ManMoveDir = 0;


Settings::Settings()
{
        _Color.red = 0;
        _Color.green = 0;
        _Color.blue = 0;
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