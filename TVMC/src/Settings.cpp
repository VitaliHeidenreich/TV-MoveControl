#include <Arduino.h>
#include "Settings.h"
#include <Wire.h>
#include "defines.h"

pixel_t Settings::_Color = {100,0,0};
uint16_t Settings::_UpperCollisionADCValue = 100;
uint16_t Settings::_TurnOnCurrentValue = 3040;
uint16_t Settings::_TurnOffCurrentValue = 3021;
uint16_t Settings::_DivOnAndOffADCValue = 10;
uint8_t Settings::_AutoMove     = 0;
uint8_t Settings::_BrightnessValue = 80;
uint32_t Settings::_eepromWriteCounter = 0;
/**
 * @brief Motor direction manual move
 * 
 * 0: NO
 * 1: IN
 * 2: OUT
 */
uint8_t Settings::_ManMoveDir   = 0;
uint8_t Settings::actualValue[10] = {0,0,0,0,0,0,0,0,0,0};
// Nur als Hilfsgröße
uint8_t Settings::_eepromInitialized = 0;
uint8_t Settings::colorAndLightBehavior = 2;
uint8_t Settings::colorchanged = 1;
uint8_t Settings::startUpIsDone = 0;

#define I2C_ADDRESS 0x52
#define I2C_DS3231  0x68

Settings::Settings()
{

}

// Just for debug
uint32_t Settings::getEeproWriteCount( void )
{
    return Settings::_eepromWriteCounter;
}

void Settings::incrementEeproWriteCount( void )
{
    Settings::_eepromWriteCounter++;
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
uint8_t Settings::getColorAndLightBehavior( )
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

    uint8_t hou;
    
    switch ( Settings::colorAndLightBehavior )
    {
        case 1:
            hou = Settings::getTimeHour( );
            if( (hou >= 0) && (hou <= 5) )
            {
                ret_Red   = 0;
                ret_Green = 0;
                ret_Blue  = 0;
            }
            break;
        case 3:
            ret_Red   = ret_Red   * 0.3;
            ret_Green = ret_Green * 0.3;
            ret_Blue  = ret_Blue  * 0.3;
            break;

        default:
            hou = Settings::getTimeHour( );
            if( (hou >= 23) || (hou <= 5) )
            {
                if( (hou >= 0) && (hou <= 4) )
                {
                    ret_Red   = 0;
                    ret_Green = 0;
                    ret_Blue  = 0;
                }
                else
                {
                    ret_Red   = ret_Red   * 0.05;
                    ret_Green = ret_Green * 0.05;
                    ret_Blue  = ret_Blue  * 0.05;
                }
            }
            else if( hou == 23 )
            {
                ret_Red   = ret_Red   * 0.2;
                ret_Green = ret_Green * 0.2;
                ret_Blue  = ret_Blue  * 0.2;
            }
            else{ /* nop */ }
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
void Settings::incrementStartUpCounter( )
{
    static uint16_t tikz = 0;
    
    if( tikz >= 200 )
        Settings::startUpIsDone = 1;
    else
        tikz ++;
}

uint8_t Settings::checkStartUpIsDone( )
{
    if( Settings::startUpIsDone == 1 )
        return 1;
    else if( Settings::startUpIsDone == 2 )
        return 1;
    else
        return 0;
}

/**********************************************************************
 * Letzte eingestellte Farbe einlesen und Speichern
 * NV RAM: 1 2 3
 *********************************************************************/
void Settings::getSavedColor( void )
{
    Wire.begin( 21, 22, 800000 );
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
    incrementEeproWriteCount( );
    Wire.begin( 21, 22, 800000 );
    Wire.beginTransmission( I2C_ADDRESS );
    Wire.write( 0x01 );
    Wire.write( _Color.red );
    Wire.write( _Color.green );
    Wire.write( _Color.blue );
    Wire.endTransmission( );
    delay(5); // important!
    _eepromWriteCounter++;
}

void Settings::saveActColor( int red, int green, int blue )
{
    incrementEeproWriteCount( );
    Wire.begin( 21, 22, 800000 );
    Wire.beginTransmission( I2C_ADDRESS );
    Wire.write( 0x01 );
    Wire.write( red );
    Wire.write( green );
    Wire.write( blue );
    Wire.endTransmission( );
    delay(5); // important!
    _eepromWriteCounter++;
}

/****************************************************************************************
 * @brief Get last saved max collision value
 * @details NV Usage 4, 5
 * @param no return value
 * @return uint16_t last safed collision value [0 - 4095]
 ****************************************************************************************/
uint16_t Settings::readEEPromSavedMotorCollisionValue( void )
{
    uint16_t msb = 0;
    uint16_t lsb = 0;
    Wire.begin( 21, 22, 800000 );
    Wire.beginTransmission( I2C_ADDRESS );
    Wire.write( 0x04 );
    Wire.endTransmission( );
    Wire.requestFrom( I2C_ADDRESS, 2);
    msb = Wire.read()*100;
    lsb = Wire.read();
    _UpperCollisionADCValue = msb + lsb;
    return _UpperCollisionADCValue;
}

uint16_t Settings::readSavedMotorCollisionValue_Direct( void )
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
    incrementEeproWriteCount( );
    Settings::_UpperCollisionADCValue = val;

    Wire.begin( 21, 22, 800000 );
    Wire.beginTransmission( I2C_ADDRESS );
    Wire.write( 0x04 );
    Wire.write( (uint8_t)(val/100) );
    Wire.write( val%100 );
    Wire.endTransmission( );
    delay(5); // important!
    _eepromWriteCounter++;
}

/****************************************************************************************
 * @brief Get last turnOn value
 * @details NV Usage 6, 7
 * @param no return value
 * @return uint16_t last safed turnOn value
 ****************************************************************************************/
uint16_t Settings::getSavedTurnOnValue( void )
{
    uint16_t msb = 0;
    uint16_t lsb = 0;
    Wire.begin( 21, 22, 800000 );
    Wire.beginTransmission( I2C_ADDRESS );
    Wire.write( 0x06 );
    Wire.endTransmission( );
    Wire.requestFrom( I2C_ADDRESS, 2);
    msb = Wire.read()*100;
    lsb = Wire.read();
    _TurnOnCurrentValue = msb + lsb;
    return _TurnOnCurrentValue;
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
    incrementEeproWriteCount( );
    Settings::_TurnOnCurrentValue = val;

    Wire.begin( 21, 22, 800000 );
    Wire.beginTransmission( I2C_ADDRESS );
    Wire.write( 0x06 );
    Wire.write( (uint8_t)(val/100) );
    Wire.write( val%100 );
    Wire.endTransmission( );
    delay(5); // important!
    _eepromWriteCounter++;
}

/****************************************************************************************
 * @brief Get last turnOff value
 * @details NV Usage 8, 9
 * @param no return value
 * @return uint16_t last safed turnOn value
 ****************************************************************************************/
uint16_t Settings::getSavedTurnOffValue( void )
{
    uint16_t msb = 0;
    uint16_t lsb = 0;
    Wire.begin( 21, 22, 800000 );
    Wire.beginTransmission( I2C_ADDRESS );
    Wire.write( 0x08 );
    Wire.endTransmission( );
    Wire.requestFrom( I2C_ADDRESS, 2);
    msb = Wire.read()*100;
    lsb = Wire.read();
    _TurnOffCurrentValue = msb + lsb;
    return _TurnOffCurrentValue;
}
uint16_t Settings::getSavedTurnOffValue_D( void )
{
    return Settings::_TurnOffCurrentValue;
}

/****************************************************************************************
 * @brief Save new turnOff value
 * @details NV Usage 8, 9
 * @param uint16_t value to safe
 * @return no return value
 ****************************************************************************************/
void Settings::saveTurnOffValue( uint16_t val )
{
    incrementEeproWriteCount( );
    Settings::_TurnOffCurrentValue = val;

    Wire.begin( 21, 22, 800000 );
    Wire.beginTransmission( I2C_ADDRESS );
    Wire.write( 0x08 );
    Wire.write( (uint8_t)(val/100) );
    Wire.write( val%100 );
    Wire.endTransmission( );
    delay(5); // important!
    _eepromWriteCounter++;
}

/****************************************************************************************
 * @brief Set diviation for the TV off and on values
 * @details NV Usage 8, 9
 * @param no return value
 * @return uint16_t last safed turnOn value
 ****************************************************************************************/
uint8_t Settings::getDivTurnOnOffValue( void )
{
    Wire.begin( 21, 22, 700000 );
    Wire.beginTransmission( I2C_ADDRESS );
    Wire.write( I2C_ADR_TV_DIV_VALUE );
    Wire.endTransmission( );
    Wire.requestFrom( I2C_ADDRESS, 1);
    _DivOnAndOffADCValue = Wire.read();
    return _TurnOffCurrentValue;
}
uint8_t Settings::getDivTurnOnOffValue_D( void )
{
    return Settings::_DivOnAndOffADCValue;
}

/****************************************************************************************
 * @brief Save diviation for the TV off and on values
 * @details NV Usage 8, 9
 * @param uint16_t value to safe
 * @return no return value
 ****************************************************************************************/
void Settings::saveDivTurnOnOffValue( uint8_t val )
{
    incrementEeproWriteCount( );
    Settings::_DivOnAndOffADCValue = val;

    Wire.begin( 21, 22, 700000 );
    Wire.beginTransmission( I2C_ADDRESS );
    Wire.write( I2C_ADR_TV_DIV_VALUE );
    Wire.write( val );
    Wire.endTransmission( );
    delay(5); // important!
    _eepromWriteCounter++;
}




/****************************************************************************************
 * @brief Set light effects 0: NO, 1 = 0%, 2 = 30%, 3 = 50%
 * @details NV Usage 8, 9
 * @param no return value
 * @return uint16_t last safed turnOn value
 ****************************************************************************************/
uint8_t Settings::getLightBehaviour( void )
{
    Wire.begin( 21, 22, 700000 );
    Wire.beginTransmission( I2C_ADDRESS );
    Wire.write( I2C_ADR_LIGHT_EFFECT );
    Wire.endTransmission( );
    Wire.requestFrom( I2C_ADDRESS, 1);
    colorAndLightBehavior = Wire.read();
    return colorAndLightBehavior;
}
uint8_t Settings::getLightBehaviour_D( void )
{
    return Settings::colorAndLightBehavior;
}

/****************************************************************************************
 * @brief Save diviation for the TV off and on values
 * @details NV Usage 8, 9
 * @param uint16_t value to safe
 * @return no return value
 ****************************************************************************************/
void Settings::setLightBehaviour( uint8_t val )
{
    incrementEeproWriteCount( );
    Settings::colorAndLightBehavior = val;

    Wire.begin( 21, 22, 700000 );
    Wire.beginTransmission( I2C_ADDRESS );
    Wire.write( I2C_ADR_LIGHT_EFFECT );
    Wire.write( val );
    Wire.endTransmission( );
    delay(5); // important!
    _eepromWriteCounter++;
}


/* #####################################################################################*/
/* #####################################################################################*/


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
        if( hours >= 20 )
            h = 0b00100000;
        else if( hours >= 10 )
            h = 0b00010000;
        else
            h = 0;

        h =  h + ( hours % 10 );
        s =  (uint8_t(seconds/10) << 4) | ( seconds % 10 );
        m =  (uint8_t(minutes/10) << 4) | ( minutes % 10 );

        Wire.begin( 21, 22, 300000 );
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
    uint8_t m=0, h=0, helper=0;
    Wire.begin( 21, 22, 300000 );
    Wire.beginTransmission( I2C_DS3231 );
    Wire.write( 0x01 );
    Wire.endTransmission( );
    Wire.requestFrom( I2C_DS3231, 2);

    m = Wire.read();
    h = Wire.read();
    delayMicroseconds(3);

    *minutes = (( m>>4 )*10) + ( m & 0x0f );
    
    if( h & 0b00100000 )
        helper = 20;
    else if( h & 0b00010000 )
        helper = 10;
    else
        helper = 0;

    *hours = helper + ( h & 0x0f );
}

/****************************************************************************************
 * @brief get hours clock.
 * 
 * @param Getter: hours
 * @return no return value
 ****************************************************************************************/
uint8_t Settings::getTimeHour( void )
{
    uint8_t h, hours;
    Wire.begin( 21, 22, 300000 );
    Wire.beginTransmission( I2C_DS3231 );
    Wire.write( 0x02 );
    Wire.endTransmission( );
    Wire.requestFrom( I2C_DS3231, 1);

    h = Wire.read();
    delayMicroseconds(3);

    if( h & 0b00100000 )
        hours = 20;
    else if( h & 0b00010000 )
        hours = 10;
    else
        hours = 0;

    hours = hours + (h & 0x0f);

    return hours;
}

/****************************************************************************************
 * @brief get real time clock. *hours, *minutes and *seconds. 
 * 
 * @param Getter: hours, minutes and seconds
 * @return no return value
 ****************************************************************************************/
void Settings::getTime( uint8_t *hours, uint8_t *minutes, uint8_t *seconds )
{
    uint8_t s=0, m=0, h=0, helper=0;
    Wire.begin( 21, 22, 300000 );

    Wire.beginTransmission( I2C_DS3231 );
    Wire.write( 0x00 );
    Wire.endTransmission();
    Wire.requestFrom( I2C_DS3231, 3);

    s = Wire.read();
    m = Wire.read();
    h = Wire.read();
    delayMicroseconds(3);

    *seconds = (( s>>4 )*10) + ( s & 0x0f );
    *minutes = (( m>>4 )*10) + ( m & 0x0f );

    if( h & 0b00100000 )
        helper = 20;
    else if( h & 0b00010000 )
        helper = 10;
    else
        helper = 0;

    *hours = helper + ( h & 0x0f );
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
    Wire.begin( 21, 22, 300000 );
    Wire.beginTransmission( I2C_DS3231 );
    Wire.write( 0x04 );
    Wire.endTransmission( );
    Wire.requestFrom( I2C_DS3231, 3);

    d = Wire.read();
    m = Wire.read();
    y = Wire.read();
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

        Wire.begin( 21, 22, 300000 );
        Wire.beginTransmission( I2C_DS3231 );
        Wire.write( 0x04 );
        Wire.write( d );
        Wire.write( m );
        Wire.write( y );
        Wire.endTransmission( );
        delayMicroseconds(4);

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
    Wire.begin( 21, 22, 300000 );
    Wire.beginTransmission( I2C_DS3231 );
    Wire.write( 0x0f );
    Wire.endTransmission( );
    Wire.requestFrom( I2C_DS3231, 1);

    lastState = Wire.read();
    delayMicroseconds(5);
    
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