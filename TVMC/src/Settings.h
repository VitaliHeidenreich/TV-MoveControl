#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>
#include "WS2812B.h"


class Settings
{
        
public:
        //Konstruktor
        Settings();

        /****************************************
         * LED Helligkeit
         ***************************************/
        void setBrightnessPercent(byte Brightness);
        byte getBrightnessPercent();

        /****************************************
         * LED Farbe
         ***************************************/
        void setColor(pixel_t color);
        void setColor(byte red, byte green, byte blue);
        pixel_t getColor();
        uint8_t blinkCollision(uint8_t on);

        static uint8_t _AutoMove;
        static uint8_t _ManMoveDir;

private:
        static byte _Brightness;
        static pixel_t _Color;
        static uint8_t _x;
};

#endif /* SETTINGS_H */