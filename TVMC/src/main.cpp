#include <Arduino.h>
#include "config.h"
#include "mypins.h"
#include "WS2812B.h"
#include "App.h"
#include "BluetoothSerial.h"
#include "defines.h"

#define LED_COUNT 112
#define MAX_GET_VAL 8

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

void IRAM_ATTR Ext_INT1_ISR()
{
    if(mypins::direction == 1)
    {
        if( mypins::ActualStepTVBoard < MAX_STEP_TV )
            mypins::ActualStepTVBoard ++;
    }
    else
    {
        if( mypins::ActualStepTVBoard > 0 )
            mypins::ActualStepTVBoard --;
    }
}

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


void setup()
{
    Serial.begin(115200);
    // ets_delay_us(1000);
    // Timer Funktionen
    
    set = new Settings();

    event = 0;
    timer = timerBegin(1, 80, true);
    timerAttachInterrupt(timer, &onTimer, true);
    timerAlarmWrite(timer, 20000, true);
    timerAlarmEnable(timer);

    

    InOut = mypins();
    Led = new WS2812((gpio_num_t)LED_PIN,LED_COUNT);
    
    Led->clear();
    Led->setAllPixels({0,0,0});
    Led->show();

    if (!SerialBT.begin("MyTV_Movit_V2"))
    {
        Serial.println("ERROR: Bluetooth kann nicht gestartet werden!");
    }

    pinMode(INTERRUPT_PIN, INPUT);
    attachInterrupt(INTERRUPT_PIN, Ext_INT1_ISR, RISING);
    
    // Einlesen der NV Werte
    set->getSavedColor( );
    set->getSavedTurnOnValue( );
    set->getSavedUpperCollisionADCValue( );
    Serial.println("TVMC gestartet!");
}

static uint8_t dirOut = 0;
static uint8_t tvState = 0;
static uint8_t iLedCntr = 0;
static pixel_t targetColor = {0,0,0};

void loop()
{ 
    /*************************************************************************************
     * solange keine Kollision erkannt wurde:
     * Den Zustand des Fernsehers feststellen und einstellen der Richtung
     * Abfrage erfolgt alle 500ms
     ************************************************************************************/
    /************************************************************************************
     * Eventgetriggerte Steuerung der Bewegung und der LEDs
     ************************************************************************************/
    if( event )
    {
        /* Einlesen des Inputstreams from Bluetooth */
        if (SerialBT.available())
            appinterpreter.readCommandCharFromApp( (char)SerialBT.read() );

        /* Einlesen des Inputstreams from Serial */
        if (Serial.available())
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
            set->actualValue[0]=0;
        }

        /* Startup Timer um zuerst gueltige Werte fuers Verfahren zu bekommen */
        set->startUpTimer();

        tvState = InOut.getTVstate( );

        /* LED Steuerung R/G/B */
        if( !InOut.collisionDetected )
        {
            if( InOut.colorchanged )
            {
                targetColor = set->getColor();
                iLedCntr = 0;  
                set->saveActColor( );
                InOut.colorchanged = 0;
            }
            InOut.collisionDetected = InOut.checkMotorIsBlocked();
        }
        else
        {
            InOut.checkMotorIsBlocked(); // Damit 
            targetColor = {255,0,0};
            iLedCntr = 0;
        }

        /* Change LED color */
        if( iLedCntr < LED_COUNT )
        {
            Led->setPixel( iLedCntr, targetColor );
            Led->show();
            iLedCntr ++;
        }

        /* Ruecksetzen des Events */
        event = 0;
    }
    
    /*************************************************************************************
     * solange keine Kollision erkannt wurde und die Startzeit abgewartet wurde:
     * Bewege den Fernseher falls nötig / gewünscht
     ************************************************************************************/
    if( !InOut.collisionDetected && set->startUpTimer() )
    {
        /* Automatisches Verfahren des Fernsehers */
        if( set->_AutoMove ) 
            dirOut = InOut.setMotorDir( tvState );
        /* Manuelles Verfahren des Fernsehers - Initial nicht aktiv */
        else
            dirOut = InOut.setMotorDir( (set->_ManMoveDir == 2) ? 0 : 1 );

        /*************************************************************************************
         * Ist-Position des Fernstehers feststellen und Abgleich mit dem Soll
         * * Out-Stopp ist    ET2       Ausführung als Schließer
         * * In-Stopp ist     ET1       Ausführung als Schließer
         ************************************************************************************/
        if((( OUT_SENSSTATE ) && ( dirOut==1 )) || (( IN_SENSSTATE ) && ( dirOut==0 )))
            InOut.setMotorSpeed( 1 );
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