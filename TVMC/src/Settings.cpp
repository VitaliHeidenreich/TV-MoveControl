#include <Arduino.h>
#include "Settings.h"
#include <Wire.h>

pixel_t Settings::_Color = {100,100,100};
uint16_t Settings::_UpperCollisionADCValue = 1422;
uint16_t Settings::_TurnOnCurrentValue = 1405;
uint8_t Settings::_AutoMove     = 1;
uint8_t Settings::_ManMoveDir   = 0;
uint8_t Settings::actualValue[8] = {0,0,0,0,0,0,0,0};
// Nur als Hilfsgröße
uint8_t Settings::_eepromInitialized = 0;

#define I2C_ADDRESS 0x52

Settings::Settings()
{

}

/****************************************
 * LED Farbe
 ***************************************/
void Settings::setColor(pixel_t c)
{
    Settings::_Color = c;
    Settings::saveActColor( );
}

void Settings::setColor(byte red, byte green, byte blue)
{
    Settings::_Color.red = red;
    Settings::_Color.green = green;
    Settings::_Color.blue = blue;
    Settings::saveActColor( );
}

pixel_t Settings::getColor()
{
    Wire.begin( 21, 22, 700000 );
    Wire.beginTransmission( I2C_ADDRESS );
    Wire.write( 0x01 );
    Wire.endTransmission( );
    Wire.requestFrom( I2C_ADDRESS, 3);
    _Color.red = Wire.read();
    _Color.green = Wire.read();
    _Color.blue = Wire.read();
    return {_Color.red, _Color.green, _Color.blue};
}


uint8_t Settings::blinkCollision(uint8_t on)
{
    static uint8_t blinkTikz = 0;
    static uint8_t iRet = 0;


    if( blinkTikz == 50 )
    {
        blinkTikz = 0;
        if(iRet)
            iRet = 0;
        else
            iRet = 1;
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
uint8_t Settings::startUpTimer()
{
    static uint16_t tikz = 0;
    if( tikz >= 100 )
        return 1;
    else
    {
        tikz ++;
        return 0;
    }
}

/**********************************************************************
 * Letzte eingestellte Farbe einlesen und Speichern
 * NV RAM: 1 2 3
 *********************************************************************/
void Settings::getSavedColor( void )
{
    Wire.begin( 21, 22, 700000 );
    Wire.beginTransmission( I2C_ADDRESS );
    Wire.write( 0x01 );
    Wire.endTransmission( );
    Wire.requestFrom( I2C_ADDRESS, 3);
    _Color.red = Wire.read();
    _Color.green = Wire.read();
    _Color.blue = Wire.read();
    setColor( _Color.red, _Color.green, _Color.blue);
}

void Settings::saveActColor( )
{
    Wire.begin( 21, 22, 700000 );
    Wire.beginTransmission( I2C_ADDRESS );
    Wire.write( 0x01 );
    Wire.write( _Color.red );
    Wire.write( _Color.green );
    Wire.write( _Color.blue );
    Wire.endTransmission( );
    delay(5); // important!
}

/**********************************************************************
 * Einstellen der Kollisionserkennung
 * NV RAM: Val 4 5
 *********************************************************************/
uint16_t Settings::getSavedUpperCollisionADCValue( void )
{
    Wire.begin( 21, 22, 700000 );
    Wire.beginTransmission( I2C_ADDRESS );
    Wire.write( 0x04 );
    Wire.endTransmission( );
    Wire.requestFrom( I2C_ADDRESS, 2);
    Settings::_UpperCollisionADCValue = (Wire.read()*100) + Wire.read();
    return Settings::_UpperCollisionADCValue;
}

void Settings::saveUpperCollisionADCValue( uint16_t val )
{
    Settings::_UpperCollisionADCValue = val;

    Wire.begin( 21, 22, 700000 );
    Wire.beginTransmission( I2C_ADDRESS );
    Wire.write( 0x04 );
    Wire.write( (uint8_t)(val/100) );
    Wire.write( val%100 );
    Wire.endTransmission( );
    delay(5); // important!
}

/**********************************************************************
 * Einstellen der Einschaltschwelle
 * NV RAM: Val 6 7
 *********************************************************************/
uint16_t Settings::getSavedTurnOnValue( void )
{
    Wire.begin( 21, 22, 700000 );
    Wire.beginTransmission( I2C_ADDRESS );
    Wire.write( 0x06 );
    Wire.endTransmission( );
    Wire.requestFrom( I2C_ADDRESS, 2);
    Settings::_TurnOnCurrentValue = (Wire.read()*100) + Wire.read();
    return Settings::_TurnOnCurrentValue;
}

void Settings::saveTurnOnValue( uint16_t val )
{
    Settings::_TurnOnCurrentValue = val;

    Wire.begin( 21, 22, 700000 );
    Wire.beginTransmission( I2C_ADDRESS );
    Wire.write( 0x06 );
    Wire.write( (uint8_t)(val/100) );
    Wire.write( val%100 );
    Wire.endTransmission( );
    delay(5); // important!
}
