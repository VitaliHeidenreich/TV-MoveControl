#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>
#include "WS2812B.h"
#include <Wire.h>


class Settings
{
        public:
                //Konstruktor
                Settings();

                // Just for debug
                uint32_t getEeproWriteCount( void );

                /****************************************
                 * LED Farbe
                 ***************************************/
                void setColor(pixel_t color);
                void setColor(byte red, byte green, byte blue);
                pixel_t getColor( void );
                uint8_t blinkCollision(uint8_t on);

                static uint8_t _AutoMove;
                static uint8_t _ManMoveDir;

                // Freigabe der Steuerung nach einer Definierten Zeit beim Aufstart
                void incrementStartUpCounter( void );
                uint8_t checkStartUpIsDone( void );

                // Speichern der Angaben auf den EEPROM
                void saveActColor( void );
                void saveActColor( int red, int green, int blue );
                void getSavedColor( void );

                // Settings for collision limit
                void saveUpperCollisionADCValue( uint16_t val );
                uint16_t readEEPromSavedMotorCollisionValue( void );
                uint16_t readSavedMotorCollisionValue_Direct( void );

                void saveTurnOnValue( uint16_t val );
                uint16_t getSavedTurnOnValue( void );
                uint16_t getSavedTurnOnValue_D( void );

                void saveTurnOffValue( uint16_t val );
                uint16_t getSavedTurnOffValue( void );
                uint16_t getSavedTurnOffValue_D( void );

                // Test function  eeprom
                void setDirectColor( pixel_t c );
                void getDirectColor( pixel_t *c );

                uint8_t getDivTurnOnOffValue( void );
                uint8_t getDivTurnOnOffValue_D( void );
                void saveDivTurnOnOffValue( uint8_t val );

                /******************************************************************
                 * vector  0: - Trigger if != 0
                 * vector  1: - color red
                 * vector  2: - color green
                 * vector  3: - color blue
                 * vector  4: - max collision current value msb
                 * vector  5: - max collision current value lsb
                 * vector  6: - ADC turn  on value msb
                 * vector  7: - ADC turn  on value lsb
                 * vector  8: - ADC turn off value msb
                 * vector  9: - ADC turn off value lsb
                 * vector 10: - actual ADC current value msb
                 * vector 11: - actual ADC current value lsb
                *******************************************************************/
                static uint8_t actualValue[10];

                uint8_t setTime( uint8_t hours, uint8_t minutes, uint8_t seconds );
                void getTime( uint8_t *hours, uint8_t *minutes, uint8_t *seconds );
                void getTime( uint8_t *hours, uint8_t *minutes );
                uint8_t getTimeHour( void );

                uint8_t setDate( uint8_t year, uint8_t month, uint8_t date );
                void getDate( uint8_t *year, uint8_t *month, uint8_t *date );

                uint8_t checkForPowerLossRtc();

                void setColorAndLightBehavior( uint8_t setting );
                uint8_t getColorAndLightBehavior( );

                void setLightBehaviour( uint8_t val );
                uint8_t getLightBehaviour( void );
                uint8_t getLightBehaviour_D( void );

                static uint8_t colorchanged;

        private:
                void incrementEeproWriteCount();
                static pixel_t _Color;
                static uint16_t _UpperCollisionADCValue;
                static uint16_t _TurnOnCurrentValue;
                static uint16_t _DivOnAndOffADCValue;
                static uint16_t _TurnOffCurrentValue;
                static uint8_t _BrightnessValue;
                static uint8_t _x;
                static uint8_t _eepromInitialized;
                static uint8_t colorAndLightBehavior;
                static uint8_t startUpIsDone;
                static uint32_t _eepromWriteCounter;
};

#endif /* SETTINGS_H */