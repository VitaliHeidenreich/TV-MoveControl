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
         * LED Farbe
         ***************************************/
        void setColor(pixel_t color);
        void setColor(byte red, byte green, byte blue);
        pixel_t getColor();
        uint8_t blinkCollision(uint8_t on);

        static uint8_t _AutoMove;
        static uint8_t _ManMoveDir;

        // Freigabe der Steuerung nach einer Definierten Zeit beim Aufstart
        static uint8_t initTimeOver;
        void startUpTimer( void );

        // Speichern der Angaben auf den EEPROM
        void saveActColor( void );
        void getSavedColor( void );

        // Settings for collision limit
        void saveUpperCollisionADCValue( uint16_t val );
        uint16_t getSavedUpperCollisionADCValue( void );

        void saveUpperTurnOnValue( uint16_t val );
        uint16_t getSavedUpperTurnOnValue( void );

        void saveLowerTurnOffValue( uint16_t val );
        uint16_t getSavedLowerTurnOffValue( void );

private:
        static pixel_t _Color;
        static uint16_t _UpperCollisionADCValue;
        static uint8_t _x;
};

#endif /* SETTINGS_H */