#include <Arduino.h>
#include "config.h"
#include "mypins.h"
#include "WS2812B.h"
#include "App.h"
#include "BluetoothSerial.h"
#include "defines.h"

#define MAX_GET_VAL 10

#define COUNT_UP 0
#define JUST_READ 1

// I2C device found at address 0x52 --> EEPROM
// I2C device found at address 0x68 --> ds3231

WS2812 *Led;
mypins InOut;
Settings *set;

hw_timer_t * timer = NULL;
hw_timer_t * testTimer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
uint8_t event;

uint8_t start_string[] = "START";
uint8_t next_string[] = "NEXT";
uint8_t ende_string[] = "END";

/******************************************************************************************
 * Anlegen der Peripherie Instanzen
 *****************************************************************************************/
BluetoothSerial SerialBT;
App appinterpreter;
 
void IRAM_ATTR onTimer()
{
  portENTER_CRITICAL_ISR(&timerMux);
  event = 1;
  portEXIT_CRITICAL_ISR(&timerMux);
}

uint8_t toggleIntager( uint8_t input )
{
    if( input )
        return 0;
    else
        return 1;
}

static uint8_t dirOut = 0;
static uint8_t tvState = 0;
static uint8_t iLedCntr = 0;
static pixel_t targetColor = {0,0,0};

void setup()
{
    Serial.begin(115200);
    // ets_delay_us(1000);
    // Timer Funktionen
    
    set = new Settings();

    event = 0;
    timer = timerBegin(1, PRESCALER, true); // 80000MHz / 80
    timerAttachInterrupt(timer, &onTimer, true); // Attached interrupt function
    timerAlarmWrite(timer, INTERRUPTTIME * 1000, true); // in ms as interrupt
    timerAlarmEnable(timer);

    //set->setColor({0,0,70});

    InOut = mypins();
    Led = new WS2812((gpio_num_t)LED_PIN,LED_CNT_SUM);
    // Led->setAllPixels(100, 0, 0);
    Led->turnOffAllPixels( );

    if (!SerialBT.begin("MyTV_Movit_V2"))
    {
        Serial.println("ERROR: Bluetooth kann nicht gestartet werden!");
    }
    
    // Einlesen der NV Werte
    set->getSavedColor( );
    set->getSavedTurnOnValue( );
    set->getSavedTurnOffValue( );
    set->readEEPromSavedMotorCollisionValue( );
    Serial.println("TVMC gestartet! To get help tipe \"help\" in to the console. For the the status \"status\".");

    if( set->checkForPowerLossRtc() )
    {
        Serial.println("RTC power loss was detected! RTC will set initialy to 12:00:00");
        set->setTime(12,27,58);
        set->setDate(1,1,25);
    }
    
    InOut.setMotorSpeed( 0 );
    InOut.setColorChangeTrigger();
}

uint8_t motorSpeed = 100;

void loop()
{ 
    /*************************************************************************************
     * solange keine Kollision erkannt wurde:
     * Den Zustand des Fernsehers feststellen und einstellen der Richtung
     * Abfrage erfolgt alle 500ms
     * 
     * Eventgetriggerte Steuerung der Bewegung und der LEDs
     ************************************************************************************/
    if( event )
    {
        // What for valid current values
        set->incrementStartUpCounter( );

        // Motor block state check
        InOut.checkMotorForCollision();

        /* Einlesen des Inputstreams from Bluetooth */
        while (SerialBT.available())
            appinterpreter.readCommandCharFromApp( (char)SerialBT.read() );

        /* Einlesen des Inputstreams from Serial */
        while (Serial.available())
            appinterpreter.readCommandCharFromSerial( (char)Serial.read() );
        
        /* Senden der letzten Einstellungen */
        if (set->actualValue[0] == 7)
        {
            SerialBT.write( start_string, 6);
            for( int i = 1; i < MAX_GET_VAL; i++ )
            {
                SerialBT.write(set->actualValue[i]); SerialBT.write( next_string, 5);
            }
            SerialBT.write( ende_string, 4);
            set->actualValue[0] = 0;
        }

        tvState = InOut.getTVstate( );

        /* Startup Timer um zuerst gueltige Werte fuers Verfahren zu bekommen */
        if( InOut.getColorChangeTrigger() )
        {
            targetColor = set->getColor();
            iLedCntr = 0; 
        }

        /* Change LED color */
        if ( set->checkStartUpIsDone( ) )
        {
            if ( InOut.motorCollisionDetected( ) )
            {
                Led->alternateColor(80, 0, 0);
            }    
            else 
            {
                    if ( !ENDSTOP_OUT  &&  dirOut==1 && motorSpeed > 0 )
                    {
                        Led->showMovingLights(0, targetColor); iLedCntr = 0; 
                    }
                    else if( !ENDSTOP_IN && dirOut==0 && motorSpeed > 0 )
                    {
                        Led->showMovingLights(1, targetColor); iLedCntr = 0; 
                    }
                    else
                    {
                        if( iLedCntr < LED_CNT_SUM )
                        {
                            Led->setPixel( iLedCntr, targetColor );
                            Led->show();
                            iLedCntr ++;
                        }
                    }
            }
        }
        else
        {
            /* startup lights */
            Led->alternateColor( );
        }

        // Read buttons
        InOut.checkBtnFunctions();

        /* Ruecksetzen des Events */
        event = 0;
    }
     
    /*************************************************************************************
     * solange keine Kollision erkannt wurde und die Startzeit abgewartet wurde:
     * Bewege den Fernseher falls nötig / gewünscht
     ************************************************************************************/
    if( !InOut.motorCollisionDetected() && set->checkStartUpIsDone( ) )
    {
        /* Automatisches Verfahren des Fernsehers */
        if( set->_AutoMove ) 
        {
            dirOut = InOut.setMotorDir( tvState );
            motorSpeed = 100;
        }
        /* Manuelles Verfahren des Fernsehers - Initial nicht aktiv */
        else
        {
            // Soll der Fernseher ausgefahren werden?
            if ( set->_ManMoveDir == 2 )
            {
                dirOut = InOut.setMotorDir( 0 );
                motorSpeed = 100;
            }
            else if( set->_ManMoveDir == 1 )
            {
                dirOut = InOut.setMotorDir( 1 );
                motorSpeed = 100;
            }
            else
                motorSpeed = 0;
        }
        
        /*************************************************************************************
         * Ist-Position des Fernstehers feststellen und Abgleich mit dem Soll
         * * Out-Stopp ist    ET1      Ausführung als Schließer
         * * In-Stopp ist     ET2      Ausführung als Schließer
         ************************************************************************************/
        if(( ( !ENDSTOP_OUT  &&  dirOut==1) ) || ( !ENDSTOP_IN && dirOut==0 ))
            InOut.setMotorSpeed( motorSpeed );
        else
            InOut.setMotorSpeed( 0 );
    }
    // Collision detected
    else
    {
        InOut.setMotorSpeed( 0 );
    }
}
/************************************************************************************************/