
#include "App.h"
#include "config.h"
#include "WS2812B.h"
#include "BluetoothSerial.h"
#include <ctype.h>
#include "defines.h"

App::App()
{
    settings = new Settings();
    InOut = new mypins();
}

int charToInt(char c)
{
    return (int)c - 48;
}

// Get help
uint8_t getHelp(char *c)
{
    if( ((*(c+3) == 'H')||(*(c+3) == 'h')) &&
        ((*(c+2) == 'E')||(*(c+2) == 'e')) &&
        ((*(c+1) == 'L')||(*(c+1) == 'l')) &&
        ((*(c+0) == 'P')||(*(c+0) == 'p')) )
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
// Get help
uint8_t getStatus(char *c)
{
    if( ((*(c+5) == 'S')||(*(c+5) == 's')) &&
        ((*(c+4) == 'T')||(*(c+4) == 't')) &&
        ((*(c+3) == 'A')||(*(c+3) == 'a')) &&
        ((*(c+2) == 'T')||(*(c+2) == 't')) &&
        ((*(c+1) == 'U')||(*(c+1) == 'u')) &&
        ((*(c+0) == 'S')||(*(c+0) == 's')) )
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
uint8_t isNumber( char *c)
{
    for( int i = 2; i < 5; i++)
    {
        if( !isdigit(*(c+i)) )
            return 0;
    }
    return 1;
}

uint8_t App::readCommandCharFromSerial(char CommandChar)
{
    int16_t calcVal = -1;
    // static Variablen müssen initialisiert werden
    static char _AppBefehlBuffer[] = {'0', '0', '0', '0', '0', '0', '0', '0', '0', '0'}; //10 Elemente

    //Befüllung des Ringbuffers (kopieren von vorne nach hinten, beginnend am Ende)
    for (int i = (10 - 1); i > 0; i--)
    {
        _AppBefehlBuffer[i] = _AppBefehlBuffer[i - 1];
    }

    // Neues Zeichen in den Buffer[0] schieben
    _AppBefehlBuffer[0] = CommandChar;

    // Prüfe, ob ein befehl anliegt
    if ((_AppBefehlBuffer[9] == 'X') && (_AppBefehlBuffer[1] == '$') && ((_AppBefehlBuffer[0] == '\n') || (_AppBefehlBuffer[0] == '!')))
    {
        if( _AppBefehlBuffer[8] == 'R' )
        {
            if(_AppBefehlBuffer[7] == 'C')
            {
                Serial.write(">>>>>>> Aktuelles Strom ADC Limit ist: ");
                Serial.println( settings->readEEPromSavedMotorCollisionValue() );
            }
            else if(_AppBefehlBuffer[7] == 'N')
            {
                Serial.write(">>>>>>> Aktuelle Einschaltschwelle ist: ");
                Serial.println( settings->getSavedTurnOnValue() );
            }
            else if(_AppBefehlBuffer[7] == 'F')
            {
                Serial.write(">>>>>>> Aktuelle Ausschaltschwelle ist: ");
                Serial.println( settings->getSavedTurnOffValue() );
            }
            else
                Serial.write("ERROR: Wrong command pattern.\n");
        }
        // Einstellung der Zeit
        else if( _AppBefehlBuffer[8] == 'Z' )
        {
            if(settings->setTime( charToInt(_AppBefehlBuffer[7])*10+charToInt(_AppBefehlBuffer[6]), 
                charToInt(_AppBefehlBuffer[5])*10+charToInt(_AppBefehlBuffer[4]),
                charToInt(_AppBefehlBuffer[3])*10+charToInt(_AppBefehlBuffer[2])))
            {
                Serial.write(">>>>>>> Neue Zeit wurde eingestellt auf: ");
                Serial.print(charToInt(_AppBefehlBuffer[7])*10+charToInt(_AppBefehlBuffer[6])); Serial.print(":"); 
                Serial.print(charToInt(_AppBefehlBuffer[5])*10+charToInt(_AppBefehlBuffer[4])); Serial.print(":"); 
                Serial.println(charToInt(_AppBefehlBuffer[3])*10+charToInt(_AppBefehlBuffer[2]));
            }
            else
            {
                Serial.write(">>>>>>> ERROR: Fehler bei der Zeiteinstellung.");
            }
        }
        // Aktuelle Zeit ausgeben
        else if( _AppBefehlBuffer[8] == 'G' )
        {
            uint8_t h=0, m=0, s=0;
            Serial.write(">>>>>>> Aktuell eingestellte Zeit ist: ");
            settings->getTime( &h, &m, &s );
            Serial.print(h); Serial.print(":"); Serial.print(m); Serial.print(":"); Serial.println(s);
        }
        // Einstellung des Datums
        else if( _AppBefehlBuffer[8] == 'Y' )
        {
            Serial.write(">>>>>>> Neue Datum wurde eingestellt. ");
            settings->setDate( charToInt(_AppBefehlBuffer[7])*10+charToInt(_AppBefehlBuffer[6]), 
                charToInt(_AppBefehlBuffer[5])*10+charToInt(_AppBefehlBuffer[4]),
                charToInt(_AppBefehlBuffer[3])*10+charToInt(_AppBefehlBuffer[2]));
        }
        // Aktuelles Datum ausgeben
        else if( _AppBefehlBuffer[8] == 'X' )
        {
            uint8_t D, M, Y;
            Serial.write(">>>>>>> Aktuell eingestelltes Datum ist: ");
            settings->getDate( &Y, &M, &D );
            Serial.write(D); Serial.write(":"); Serial.write(M); Serial.write(":20"); Serial.write(Y); Serial.write("\n");
        }

        else if( _AppBefehlBuffer[8] == 'T' )
        {
            InOut->sendDebugMotorCurrent = 0;
            if( InOut->sendCurrentADCValues == 0 )
            {
                InOut->sendCurrentADCValues = 1;
                Serial.println( ">>>>>>> START: Lesen ADC Stromsensor" );
            }
            else
            {
                InOut->sendCurrentADCValues = 0;
                Serial.println( ">>>>>>> END: Lesen ADC Stromsensor" );
            }
        }
        else if( _AppBefehlBuffer[8] == 'M' )
        {
            if( InOut->sendMotorCurrentValues == 0 )
            {
                InOut->sendMotorCurrentValues = 1;
                Serial.println( ">>>>>>> START: Lesen Motor ADC Werte" );
            }
            else
            {
                // Delete: sendDebugMotorCurrent
                InOut->sendMotorCurrentValues = 0;
                Serial.println( ">>>>>>> END: Lesen Motor ADC Werte" );
            }
        }
        else if( _AppBefehlBuffer[8] == 'W' )
        {
            if( isNumber( _AppBefehlBuffer ))
            {
                calcVal = ( charToInt(_AppBefehlBuffer[5]) * 1000 +
                            charToInt(_AppBefehlBuffer[4]) * 100 +
                            charToInt(_AppBefehlBuffer[3]) * 10 +
                            charToInt(_AppBefehlBuffer[2]) * 1 );

                if( calcVal >= 1 && calcVal <= 4000 )
                {
                    if(_AppBefehlBuffer[7] == 'C')
                    {
                        settings->saveUpperCollisionADCValue( calcVal );
                        Serial.write(" >>> New collision detection limits is ");
                        Serial.print( calcVal ); Serial.println( "." );
                    }
                    else if(_AppBefehlBuffer[7] == 'E')
                    {
                        settings->saveTurnOnValue( calcVal );
                        Serial.write(" >>> New turn on detection limit is ");
                        Serial.print( calcVal ); Serial.println( "." );

                        if( settings->getSavedTurnOffValue_D() >= calcVal )
                        {
                            settings->saveTurnOffValue( calcVal - 20 );
                            Serial.write(" >>> Also turn on value have to be adapted because it have to be grater that turn off. New value is ");
                            Serial.print( calcVal + 20 ); Serial.println( "." );
                        }
                    }
                    else if(_AppBefehlBuffer[7] == 'A')
                    {
                        settings->saveTurnOffValue( calcVal );
                        Serial.write(" >>> New turn off detection limit is ");
                        Serial.print( calcVal ); Serial.println( "." );

                        if( settings->getSavedTurnOnValue_D() <= calcVal )
                        {
                            settings->saveTurnOffValue( calcVal + 20 );
                            Serial.write(" >>> Also turn on value have to be adapted because it have to be grater that turn off. New value is ");
                            Serial.print( calcVal + 20 ); Serial.println( "." );
                        }
                    }
                    else if(_AppBefehlBuffer[7] == 'D')
                    {
                        if( calcVal < 100 )
                        {
                            settings->saveDivTurnOnOffValue( calcVal );
                            Serial.write(" >>> New turn on and off adc deviation value is ");
                            Serial.print( calcVal ); Serial.println( "." );
                        }
                        else
                            Serial.write(" >>> ERROR: Make sure the new value for the diviation is not greater than 99.\n");
                    }
                    else if(_AppBefehlBuffer[7] == 'T')
                    {
                        if( calcVal < 100 )
                        {
                            settings->saveDivTurnOnOffValue( calcVal );
                            Serial.write(" >>> New turn on and off adc deviation value is ");
                            Serial.print( calcVal ); Serial.println( "." );
                        }
                        else
                            Serial.write(" >>> ERROR: Make sure the new value for the diviation is not greater than 99.\n");
                    }
                    else
                    {
                        Serial.write(" >>> ERROR: Wrong defined write function. Please try again.\n");
                    }
                }
            }
            else if( _AppBefehlBuffer[8] == 'U' )
            {
                Serial.write(" >>> Light Effects: ");
                if( settings->getColorAndLightBehavior() == 1 )
                    Serial.println(" between 0 and 5 am off");
                if( settings->getColorAndLightBehavior() == 2 )
                    Serial.println(" between 23 pm and 5 am dimmed \n between 0 and 4 am off");
                else
                    Serial.println(" no. always on");
            }
            else
            {
                Serial.write(" >>> ERROR: Please ensure the new value is a number and it's between 1 and 4000.\n");
            }
        }
        else
        {
            Serial.write(" >>> ERROR: ERROR_command_not_found. Try help or status.\n");
        }
    }
    else
    {
        if( getHelp(_AppBefehlBuffer) )
        {
            Serial.println( "\nHelp for write and read of the limits:" );
            Serial.println( "     Get ADC Values:" );
            Serial.println( "         XR000000$!   read collision detection limit and turn on detection limit" );
            
            Serial.println( "     Read ADC Values:" );
            Serial.println( "          XT000000$!   read of the currently ADC value (toggel: On/Off)");
            Serial.println( "          XM000000$!   read of the currently ADC value (toggel: On/Off)" );
            Serial.println( "     Write ADC Values:" );
            Serial.println( "          XWC0XXXX$!   write the new collision detection limit (XXXX = decimal value)");
            Serial.println( "          XWE0XXXX$!   write new turn on value (XXXX = decimal value)" );
            Serial.println( "          XWA0XXXX$!   write new turn off value (XXXX = decimal value)" );
            Serial.println( "          XWD000XX$!   write the diviation for the auto values (XXXX = decimal value)" );

            Serial.println( "     Time control:" );
            Serial.println( "         XZXXXXXX$!   set new time. hhmmss" );
            Serial.println( "         XG000000$!   get current time." );
            Serial.println( "         XYXXXXXX$!   set new date. DDMMYY" );
            Serial.println( "         XX000000$!   get current date.\n" );

            Serial.println( "Software details:");
            Serial.println( "     Firmware version 2.8.2 1 R1");
            Serial.println( "          08/24/2024 Moosburg");
        }
        else if( getStatus(_AppBefehlBuffer) )
        {
            Serial.println( "\nCurently settings:" );
            Serial.println( "\tCollision ADC limit:" );
            Serial.println( "\tTV on ADC on value:" );
            Serial.println( "\tTV on ADC off value:" );
        }
        else
        {
            //Serial.write(" >>> ERROR: ERROR_command_not_found. Try help or status.\n");
        }
    }

    return 0;
}


/** ===========================================================
 *  | Befehlsstruktur:                                        |
 *  |-----------------+---------------------+-----------------|
 *  | Start/Erkennung | Eigentlicher Befehl | Ende            |
 *  |-----------------+---------------------+-----------------+
 *  | "X"             | # X X X X X X       | '$\n' oder '$\t |
 *  |-----------------+---------------------+-----------------|
 *  
 *  Details in der Befehlsliste
*/

/***************************************************************************
 * Zusammenfügen der einzelnen übertragenen char aus der App  in Befehlsbuffer
 * Übergabeparameter: char aus der seriellen BT Übertragung
 * Rückgabe: uint8_t zur Anzeige ob Befehl aktiv war oder nicht
***************************************************************************/
uint8_t App::readCommandCharFromApp(char CommandChar)
{
    // static Variablen müssen initialisiert werden
    static char _AppBefehlBuffer[] = {'0', '0', '0', '0', '0', '0', '0', '0', '0', '0'}; //10 Elemente
    char _AppBefehl[] = {'0', '0', '0', '0', '0', '0', '0'};                             // 7 Elemente
    uint8_t iRet = 0;
    uint8_t i;

    //Befüllung des Ringbuffers (kopieren von vorne nach hinten, beginnend am Ende)
    for (i = (10 - 1); i > 0; i--)
    {
        _AppBefehlBuffer[i] = _AppBefehlBuffer[i - 1];
    }

    // Neues Zeichen in den Buffer[0] schieben
    _AppBefehlBuffer[0] = CommandChar;

    // Prüfe, ob ein befehl anliegt
    if ((_AppBefehlBuffer[9] == 'X') && (_AppBefehlBuffer[1] == '$') && ((_AppBefehlBuffer[0] == '\n') || (_AppBefehlBuffer[0] == '\t')))
    {
        // Vorläufig wurde was erkannt
        iRet = 1;

        //Erstellung lokale Kopie für Befehle inkl Drehung der Orientierung
        for (i = 7; i > 1; i--)
        {
            _AppBefehl[7 - i] = _AppBefehlBuffer[i];
        }

        switch (_AppBefehlBuffer[8])
        {
        // Auswerten der Farbe
        case 'F':
            if (_AppBefehlBuffer[0] == '\n') 
                CommSetColor(_AppBefehl);
            else
                iRet = 0;
            break;
        // Auswerten der Zeit
        case 'T':
            settings->setTime(
                charToInt(_AppBefehlBuffer[7])*10 + charToInt(_AppBefehlBuffer[6]),
                charToInt(_AppBefehlBuffer[5])*10 + charToInt(_AppBefehlBuffer[4]),
                charToInt(_AppBefehlBuffer[3])*10 + charToInt(_AppBefehlBuffer[2]));
            break;
        // Automatisches Bewegen zulassen
        case 'A':
                CommSetAutomover(_AppBefehl);
            break;
        // Nur manuelles Bewegen zulassen (per App)
        case 'M':
                CommManMove(_AppBefehl);
            break;
        // Geschwindigkeit und Frequenz einstellen (Inits werden in der config.h angegeben)
        // XXXX [Frequency 0-25000] XX [Speed 0-200]
        case 'S':
                //justSendTheFoundStringToSerial(_AppBefehlBuffer);
                setFrequenzAndSpeed( _AppBefehlBuffer );
            break;
        // Reset collision detected state by the app
        case 'R':
                settings->_AutoMove = 1;
                InOut->resetCollisionDetected();
                iRet = 0;
            break;
        case 'W':
                // nop
            break;
        case 'G':
                settings->actualValue[0] = 7;
            break;
        case 'L':
            settings->setColorAndLightBehavior(charToInt(_AppBefehlBuffer[2]));
            break;
        default:
            iRet = 0;
            break;
        }
    }

    // Trigger um ein Ereignis auszulösen
    return iRet;
}

void App::CommManMove(char AppBefehl[6])
{
    // Einfahren des Fernsehers
    if( _hexcharToUint8_t(AppBefehl[5]) == 1 )
        settings->_ManMoveDir = 1;
    // Ausfahren des Fernsehers
    else if( _hexcharToUint8_t(AppBefehl[5]) == 2 )
        settings->_ManMoveDir = 2;
    // Anhalten des Fernsehers
    else
        settings->_ManMoveDir = 0;
}

/***************************************************************************
 * Funktion aus dem App-Befehl die Farbe zu setzen
 * Übergabeparameter: Array mit dem entsprechenden Befehl
 * Rückgabe: kein
***************************************************************************/
void App::CommSetColor(char AppBefehl[6])
{
    pixel_t AppColor;

    //Auslesen der Farbe
    AppColor.red = _hexcharToUint8_t(AppBefehl[0]) * 16 + _hexcharToUint8_t(AppBefehl[1]);
    AppColor.green = _hexcharToUint8_t(AppBefehl[2]) * 16 + _hexcharToUint8_t(AppBefehl[3]);
    AppColor.blue = _hexcharToUint8_t(AppBefehl[4]) * 16 + _hexcharToUint8_t(AppBefehl[5]);

    //Schrieben der Farbe in die Einstellungen
    settings->setColor(AppColor);
    settings->colorchanged = 1;
}

/***************************************************************************
 * Funktion aus dem App-Befehl die Farbe zu setzen
 * Übergabeparameter: Array mit dem entsprechenden Befehl
 * Rückgabe: kein
***************************************************************************/
void App::CommSetAutomover(char AppBefehl[6])
{
    uint8_t iTest = 0;
    iTest = _hexcharToUint8_t(AppBefehl[0]) + _hexcharToUint8_t(AppBefehl[1]) + _hexcharToUint8_t(AppBefehl[2]) + _hexcharToUint8_t(AppBefehl[3]) + _hexcharToUint8_t(AppBefehl[4]) + _hexcharToUint8_t(AppBefehl[5]);
    if( iTest >= 6 )
        settings->_AutoMove = 1;
    else
        settings->_AutoMove = 0;
}

/***************************************************************************
 * Funktion aus dem App-Befehl die Uhrzeit zu setzen
 * Übergabeparameter: Array mit dem entsprechenden Befehl
 * Rückgabe: kein
***************************************************************************/
void App::CommSetTime(char AppBefehl[6])
{
    uint8_t AppHours;
    uint8_t AppMinutes;
    uint8_t AppSeconds;

    //Auslesen Stunden
    AppHours = _hexcharToUint8_t(AppBefehl[0]) * 10 + _hexcharToUint8_t(AppBefehl[1]);

    //Auslesen Minuten
    AppMinutes = _hexcharToUint8_t(AppBefehl[2]) * 10 + _hexcharToUint8_t(AppBefehl[3]);

    //Auslesen Sekunden
    AppSeconds = _hexcharToUint8_t(AppBefehl[4]) * 10 + _hexcharToUint8_t(AppBefehl[5]);

    //Verwerfen der versendeten Appwerte bei Werten außerhalb des Wertebereichs
    if ((AppHours <= 23) || (AppMinutes <= 59) || (AppSeconds <= 59))
    {
        settings->setTime( AppHours, AppMinutes, AppSeconds );
    }
    else
    {
        Serial.println( " >>> ERROR: check the setting and try again.");
    }
}

void App::setFrequenzAndSpeed( char *_AppBefehl )
{
    uint32_t speed = 220;
    uint32_t frequenz = 650;

    speed    = _hexcharToUint8_t(*(_AppBefehl+6))*16+_hexcharToUint8_t(*(_AppBefehl+7));
    frequenz = _hexcharToUint8_t(*(_AppBefehl+3))*256+_hexcharToUint8_t(*(_AppBefehl+4))*16+_hexcharToUint8_t(*(_AppBefehl+5));
    (speed > 200)? speed = 200:speed = speed;
    (frequenz > 25000)? frequenz = 25000: frequenz = frequenz;
    //Serial.print("Eingestellte Frequenz: "); Serial.println(frequenz);
    //Serial.print("Eingestellte Geschwindigkeit: "); Serial.println(speed);
    ledcSetup(0, frequenz, speed);
}



/****************************************************************************************************************
 * Hilfsfunktionen für Datenkonvertierung
 ***************************************************************************************************************/
byte App::_convertVarToByte(uint32_t ArrayData)
{
    byte var;
    if (CONV_VERSION == 1)
    {
        var = map(ArrayData, 0, 0xFFFFFF, 0, 0xFF);
    }
    else if (CONV_VERSION == 2)
    {
        var = ArrayData & 0xFF;
    }

    return var;
}

bool App::_convertVarToBool(uint32_t ArrayData)
{
    bool var = false;
    if (CONV_VERSION == 1)
    {
        if (ArrayData >= 1)
        {
            var = true;
        }
    }
    else if (CONV_VERSION == 2)
    {
        var = ArrayData & 0x1;
    }

    return var;
}

uint16_t App::_convertVarToUint16(uint32_t ArrayData)
{
    uint16_t var = 0;

    if (CONV_VERSION == 1)
    {
        var = map(ArrayData, 0, 0xFFFFFF, 0, 0xFFFF);
    }
    else if (CONV_VERSION == 2)
    {
        var = ArrayData & 0xFFFF;
    }

    return var;
}

/***************************************************************************
 * Funktion zum Konvertieren von (hex)char in (dec)uint8_t
 * in:  hexadezimaler char Zeichen
 * out: zugehöriger dezimaler int Wert
 **************************************************************************/
uint8_t App::_hexcharToUint8_t(char hexchar)
{
    if (hexchar >= '0' && hexchar <= '9')
        return hexchar - '0';
    if (hexchar >= 'A' && hexchar <= 'F')
        return hexchar - 'A' + 10;
    if (hexchar >= 'a' && hexchar <= 'f')
        return hexchar - 'a' + 10;
    return -1;
}


/***************************************************************************
     * Serielle Ausgabe des Befehls
     * Übergabeparameter: pointer auf Beginn des Buffer-Arrays
     * Rückgabe: kein
***************************************************************************/
void App::justSendTheFoundStringToSerial(char *p)
{
    uint8_t i;
    for (i = 7; i > 1; i--)
    {
        Serial.print(*(p + i));
    }
    Serial.print('\n');
}