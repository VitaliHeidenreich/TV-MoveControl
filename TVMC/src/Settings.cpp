#include <Arduino.h>
#include "Settings.h"

pixel_t Settings::_Color = {100,100,100};
uint8_t Settings::_AutoMove = 1;
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