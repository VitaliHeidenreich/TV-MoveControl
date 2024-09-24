#include <Arduino.h>
#include "Settings.h"
#include <Wire.h>

pixel_t Settings::_Color = {100,100,100};
uint16_t Settings::_UpperCollisionADCValue = 1422;
uint16_t Settings::_TurnOnCurrentValue = 1405;
uint16_t Settings::_TurnOffCurrentValue = 1385;
uint8_t Settings::_AutoMove     = 1;
uint8_t Settings::_ManMoveDir   = 0;
uint8_t Settings::actualValue[10] = {0,0,0,0,0,0,0,0,0,0};
// Nur als Hilfsgröße
uint8_t Settings::_eepromInitialized = 0;
uint8_t Settings::colorAndLightBehavior = 0;
uint8_t Settings::colorchanged = 1;

#define I2C_ADDRESS 0x52
#define I2C_DS3231  0x68

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

/*****************************************************************************************
 * @brief Read last saved color from the eeprom and store it to static local variables
 * 
 * @param Setting for color change. 
 *                  0 = no
 *                  1 = turn off while night [24 - 5]
 *                  2 = turn on only at night [15-24]
 * @return no
 ****************************************************************************************/
void Settings::setColorAndLightBehavior( uint8_t setting )
{
    Settings::colorAndLightBehavior = setting;
}

/*****************************************************************************************
 * @brief Read last saved color from the eeprom and store it to static local variables
 * 
 * @param no
 * @return pixel_t {red,green,blue}
 ****************************************************************************************/
uint8_t Settings::getColorAndLightBehavior(byte red, byte green, byte blue)
{
    return Settings::colorAndLightBehavior;
}

/*****************************************************************************************
 * @brief Read last saved color from the eeprom and store it to static local variables
 * 
 * @param no
 * @return pixel_t {red,green,blue}
 ****************************************************************************************/
pixel_t Settings::getColor( void )
{
    uint8_t ret_Red = _Color.red;
    uint8_t ret_Green = _Color.green;
    uint8_t ret_Blue = _Color.blue;

    uint8_t hou, min;

    switch ( Settings::colorAndLightBehavior )
    {
        case 1:
            Settings::getTime( &hou, &min );
            if( (hou >= 0) && (hou <= 6) )
            {
                ret_Red   = 0;
                ret_Green = 0;
                ret_Blue  = 0;
            }
            break;

        case 2:
            Settings::getTime( &hou, &min );
            if( (hou >= 0) && (hou <= 6) )
            {
                ret_Red   = ret_Red   * 0.3;
                ret_Green = ret_Green * 0.3;
                ret_Blue  = ret_Blue  * 0.3;
            }
            break;

        case 3:
            ret_Red   = ret_Red   * 0.5;
            ret_Green = ret_Green * 0.5;
            ret_Blue  = ret_Blue  * 0.5;
            break;

        default:
            break;
    }

    return {ret_Red, ret_Green, ret_Blue};
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
uint8_t Settings::checkStartUpDone( uint8_t justRead )
{
    static uint16_t tikz = 0;
    if( tikz >= 100 )
        return 1;
    else
    {
        if(!justRead)
        {
            tikz ++;
        }
        return 0;
    }
}

/**********************************************************************
 * Letzte eingestellte Farbe einlesen und Speichern
 * NV RAM: 1 2 3
 *********************************************************************/
void Settings::getSavedColor( void )
{
    Wire.begin( 21, 22, 900000 );
    Wire.beginTransmission( I2C_ADDRESS );
    Wire.write( 0x01 );
    Wire.endTransmission( );
    Wire.requestFrom( I2C_ADDRESS, 3);
    _Color.red   = Wire.read();
    _Color.green = Wire.read();
    _Color.blue  = Wire.read();
    setColor( _Color.red, _Color.green, _Color.blue);
}

void Settings::saveActColor( )
{
    Wire.begin( 21, 22, 900000 );
    Wire.beginTransmission( I2C_ADDRESS );
    Wire.write( 0x01 );
    Wire.write( _Color.red );
    Wire.write( _Color.green );
    Wire.write( _Color.blue );
    Wire.endTransmission( );
    delay(5); // important!
}

/****************************************************************************************
 * @brief Get last saved max collision value
 * @details NV Usage 4, 5
 * @param no return value
 * @return uint16_t last safed collision value [0 - 4095]
 ****************************************************************************************/
uint16_t Settings::getSavedUpperCollisionADCValue( void )
{
    Wire.begin( 21, 22, 900000 );
    Wire.beginTransmission( I2C_ADDRESS );
    Wire.write( 0x04 );
    Wire.endTransmission( );
    Wire.requestFrom( I2C_ADDRESS, 2);
    Settings::_UpperCollisionADCValue = (Wire.read()*100) + Wire.read();
    return Settings::_UpperCollisionADCValue;
}

uint16_t Settings::getSavedUpperCollisionADCValue_D( void )
{
    return Settings::_UpperCollisionADCValue;
}

/****************************************************************************************
 * @brief Safe new max collision value  [0 - 4095]
 * @details NV Usage 4, 5
 * @param uint16_t value to safe
 * @return no return value
 ****************************************************************************************/
void Settings::saveUpperCollisionADCValue( uint16_t val )
{
    Settings::_UpperCollisionADCValue = val;

    Wire.begin( 21, 22, 900000 );
    Wire.beginTransmission( I2C_ADDRESS );
    Wire.write( 0x04 );
    Wire.write( (uint8_t)(val/100) );
    Wire.write( val%100 );
    Wire.endTransmission( );
    delay(5); // important!
}

/****************************************************************************************
 * @brief Get last turnOn value
 * @details NV Usage 6, 7
 * @param no return value
 * @return uint16_t last safed turnOn value
 ****************************************************************************************/
uint16_t Settings::getSavedTurnOnValue( void )
{
    Wire.begin( 21, 22, 900000 );
    Wire.beginTransmission( I2C_ADDRESS );
    Wire.write( 0x06 );
    Wire.endTransmission( );
    Wire.requestFrom( I2C_ADDRESS, 2);
    Settings::_TurnOnCurrentValue = (Wire.read()*100) + Wire.read();
    return Settings::_TurnOnCurrentValue;
}
uint16_t Settings::getSavedTurnOnValue_D( void )
{
    return Settings::_TurnOnCurrentValue;
}

/****************************************************************************************
 * @brief Save new turnOn value
 * @details NV Usage 6, 7
 * @param uint16_t value to safe
 * @return no return value
 ****************************************************************************************/
void Settings::saveTurnOnValue( uint16_t val )
{
    Settings::_TurnOnCurrentValue = val;

    Wire.begin( 21, 22, 900000 );
    Wire.beginTransmission( I2C_ADDRESS );
    Wire.write( 0x06 );
    Wire.write( (uint8_t)(val/100) );
    Wire.write( val%100 );
    Wire.endTransmission( );
    delay(5); // important!
}

/****************************************************************************************
 * @brief Get last turnOff value
 * @details NV Usage 8, 9
 * @param no return value
 * @return uint16_t last safed turnOn value
 ****************************************************************************************/
uint16_t Settings::getSavedTurnOffValue( void )
{
    Wire.begin( 21, 22, 900000 );
    Wire.beginTransmission( I2C_ADDRESS );
    Wire.write( 0x08 );
    Wire.endTransmission( );
    Wire.requestFrom( I2C_ADDRESS, 2);
    Settings::_TurnOnCurrentValue = (Wire.read()*100) + Wire.read();
    return Settings::_TurnOnCurrentValue;
}
uint16_t Settings::getSavedTurnOffValue_D( void )
{
    return Settings::_TurnOnCurrentValue;
}

/****************************************************************************************
 * @brief Save new turnOff value
 * @details NV Usage 8, 9
 * @param uint16_t value to safe
 * @return no return value
 ****************************************************************************************/
void Settings::saveTurnOffValue( uint16_t val )
{
    Settings::_TurnOnCurrentValue = val;

    Wire.begin( 21, 22, 900000 );
    Wire.beginTransmission( I2C_ADDRESS );
    Wire.write( 0x08 );
    Wire.write( (uint8_t)(val/100) );
    Wire.write( val%100 );
    Wire.endTransmission( );
    delay(5); // important!
}

/* #####################################################################################*/
/* #####################################################################################*/

/****************************************************************************************
 * @brief get real time clock. *hours, *minutes and *seconds. 
 * 
 * @param Getter: hours, minutes and seconds
 * @return no return value
 ****************************************************************************************/
void Settings::getTime( uint8_t *hours, uint8_t *minutes, uint8_t *seconds )
{
    uint8_t s, m, h;
    Wire.begin( 21, 22, 400000 );
    Wire.beginTransmission( I2C_DS3231 );
    Wire.write( 0x00 );
    Wire.requestFrom( I2C_DS3231, 3);
    s = Wire.read();
    m = Wire.read();
    h = Wire.read();
    Wire.endTransmission( );

    *seconds = (( s>>4 )*10) + ( s & 0x0f );
    *minutes = (( m>>4 )*10) + ( m & 0x0f );
    *hours   = ((h>>4)&0x03)*10 + (h & 0x0f);
}

/****************************************************************************************
 * @brief Set real time clock. hours, minutes and seconds. 
 * 
 * @param Setter: hours, minutes and seconds
 * @return If input value valid it returns 1
 ****************************************************************************************/
uint8_t Settings::setTime( uint8_t hours, uint8_t minutes, uint8_t seconds )
{
    uint8_t h = 0, m = 0, s = 0;

    if( (hours <= 23) && (minutes <= 59) && (seconds <= 59) )
    {
        h = (uint8_t(hours  /10) << 4) | ( hours   % 10 );
        s = (uint8_t(seconds/10) << 4) | ( seconds % 10 );
        m = (uint8_t(minutes/10) << 4) | ( minutes % 10 );

        Wire.begin( 21, 22, 400000 );
        Wire.beginTransmission( I2C_DS3231 );
        Wire.write( 0x00 );
        Wire.write( s );
        Wire.write( m );
        Wire.write( h );
        Wire.endTransmission( );
        delay(5); // important!

        return 1;
    }
    else
    {
        return 0;
    }
}

/****************************************************************************************
 * @brief get real time clock. *hours and *minutes. 
 * 
 * @param Getter: hours, minutes
 * @return no return value
 ****************************************************************************************/
void Settings::getTime( uint8_t *hours, uint8_t *minutes )
{
    uint8_t m, h;
    Wire.begin( 21, 22, 400000 );
    Wire.beginTransmission( I2C_DS3231 );
    Wire.write( 0x01 );
    Wire.requestFrom( I2C_DS3231, 2);
    m = Wire.read();
    h = Wire.read();
    Wire.endTransmission( );

    *minutes = (( m>>4 )*10) + ( m & 0x0f );
    *hours   = ((h>>4)&0x03)*10 + (h & 0x0f);
}

/****************************************************************************************
 * @brief get date. *hours, *minutes and *seconds. 
 * 
 * @param Getter: hours, minutes and seconds
 * @return no return value
 ****************************************************************************************/
void Settings::getDate( uint8_t *year, uint8_t *month, uint8_t *date )
{
    uint8_t d, m, y;
    Wire.begin( 21, 22, 400000 );
    Wire.beginTransmission( I2C_DS3231 );
    Wire.write( 0x04 );
    Wire.requestFrom( I2C_DS3231, 3);
    d = Wire.read();
    m = Wire.read();
    y = Wire.read();
    Wire.endTransmission( );
    delayMicroseconds(3);

    *date   = ( (( d>>4 )&0x07) *10) + ( d & 0x0f );
    *month  = ( (( m>>4 )&0x07) *10) + ( m & 0x0f );
    *year   =     (y>>4) * 10 + (y & 0x0f);
}

/****************************************************************************************
 * @brief Set date. hours, minutes and seconds. 
 * 
 * @param Setter: hours [0-99], minutes [0-59] and seconds [0-59]
 * @return If input value valid it returns 1
 ****************************************************************************************/
uint8_t Settings::setDate( uint8_t year, uint8_t month, uint8_t date )
{
    uint8_t y = 0, m = 0, d = 0;

    if( (year <= 99) && (month <= 12) && (date <= 31) )
    {
        y = (uint8_t(year  / 10) << 4) | ( year  % 10 );
        m = (uint8_t(month / 10) << 4) | ( month % 10 );
        d = (uint8_t(date  / 10) << 4) | ( date  % 10 );

        Wire.begin( 21, 22, 400000 );
        Wire.beginTransmission( I2C_DS3231 );
        Wire.write( 0x04 );
        Wire.write( d );
        Wire.write( m );
        Wire.write( y );
        Wire.endTransmission( );
        delayMicroseconds(3);

        return 1;
    }
    else
    {
        return 0;
    }
}

uint8_t Settings::checkForPowerLossRtc()
{
    uint8_t lastState;
    Wire.begin( 21, 22, 400000 );
    Wire.beginTransmission( I2C_DS3231 );
    Wire.write( 0x0f );
    Wire.requestFrom( I2C_DS3231, 1);
    lastState = Wire.read();
    Wire.endTransmission( );
    
    if ( lastState & 0x80)
    {
        Wire.beginTransmission( I2C_DS3231 );
        Wire.write( 0x0f );
        Wire.write( lastState & 0x7f );
        Wire.endTransmission( );
        delayMicroseconds(3);
        return 1;
    }
    else
    {
        return 0;
    }
}