
#include "App.h"
#include "config.h"
#include "WS2812B.h"
#include "BluetoothSerial.h"



App::App()
{
    settings = new Settings();
    InOut = new mypins();
}

int charToInt(char c)
{
    return (int)c - 48;
}

uint8_t App::readCommandCharFromSerial(char CommandChar)
{
    uint16_t calcVal = 0;
    // static Variablen müssen initialisiert werden
    static char _AppBefehlBuffer[] = {'0', '0', '0', '0', '0', '0', '0', '0', '0', '0'}; //10 Elemente
    //Befüllung des Ringbuffers (kopieren von vorne nach hinten, beginnend am Ende)
    for (int i = (10 - 1); i > 0; i--)
    {
        _AppBefehlBuffer[i] = _AppBefehlBuffer[i - 1];
    }
    // Neues Zeichen in den Buffer[0] schieben
    _AppBefehlBuffer[0] = CommandChar;

    if ((_AppBefehlBuffer[9] == '!') && (_AppBefehlBuffer[8] == '$') && (_AppBefehlBuffer[1] == '$') && (_AppBefehlBuffer[0] == '!'))
    {
        if( _AppBefehlBuffer[7] == 'R' )
        {
            Serial.write(" >>> Aktuelles Strom ADC Limit ist: ");
            Serial.println( settings->getSavedUpperCollisionADCValue() );

            Serial.write(" >>> Aktuelle Einschaltschwelle ist: ");
            Serial.println( settings->getSavedTurnOnValue() );
        }
        else if( _AppBefehlBuffer[7] == 'T' )
        {
            if( InOut->sendCurrentADCValues == 0 )
            {
                InOut->sendCurrentADCValues = 1;
                Serial.println( ">>>>>>> START" );
            }
            else
            {
                InOut->sendCurrentADCValues = 0;
                Serial.println( ">>>>>>> END" );
            }
        }
        else if( _AppBefehlBuffer[7] == 'W' )
        {
            calcVal = ( charToInt(_AppBefehlBuffer[5]) * 1000 +
                            charToInt(_AppBefehlBuffer[4]) * 100 +
                            charToInt(_AppBefehlBuffer[3]) * 10 +
                            charToInt(_AppBefehlBuffer[2]) * 1 );

            if(_AppBefehlBuffer[6] == 'C')
            {
                settings->saveUpperCollisionADCValue( calcVal );
                Serial.write(" >>> Neues Strom ADC Limit ist: ");
                Serial.println( calcVal );
            }
            if(_AppBefehlBuffer[6] == 'E')
            {
                settings->saveTurnOnValue( calcVal );
                Serial.write(" >>> Neue Einschaltschwelle ist: ");
                Serial.println( calcVal );
            }
        }
        else
        {
            Serial.write(" >>> ERROR_not_command_found\n");
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
 * Zusammenfügen der einzelnen übertragenen char aus der App in Befehlsbuffer
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
            if (_AppBefehlBuffer[0] == '\n')
            {
                Serial.print("Zeit erkannt ");
                //CommSetTime(_AppBefehl);
            }
            else
                iRet = 0;
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
                InOut->collisionDetected = 0;
                iRet = 0;
            break;
        case 'W':
                // nop
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
    InOut->colorchanged = 1;
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
    if ((AppHours > 23) || (AppMinutes > 59) || (AppSeconds > 59))
    {
        //AppHours = _interpreterzeitmaster->getHours();
        //AppMinutes = _interpreterzeitmaster->getMinutes();
        //AppSeconds = _interpreterzeitmaster->getSeconds();
    }

    //Auslesen der bereits enthaltenen Datumsinformation
    //AppDate = _interpreterzeitmaster->getDate();
    //AppMonth = _interpreterzeitmaster->getMonth();yxy
    //AppYear = _interpreterzeitmaster->getYear();

    //Schreiben der Uhrzeit auf die Echtzeituhr
    //_interpreterzeitmaster->setTimeDate(AppHours, AppMinutes, AppSeconds, AppDate, AppMonth, AppYear);
}

void App::setFrequenzAndSpeed( char *_AppBefehl )
{
    uint32_t speed = MAX_SPEED;
    uint32_t frequenz = FREQUENZ;

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