#include <Arduino.h>
#include "config.h"
#include "mypins.h"
#include "WS2812B.h"
#include "App.h"

#include "BluetoothSerial.h"
#include "WS2812B.h"
#include "time.h"
#include "Zeitmaster.h"

#define LED_COUNT 112

WS2812 *Led;
mypins InOut;
Settings st;
Zeitmaster *pZeit;

hw_timer_t * timer = NULL;
hw_timer_t * testTimer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
uint8_t event;

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

    // Timer Funktionen
    event = 0;
    timer = timerBegin(1, 80, true);
    timerAttachInterrupt(timer, &onTimer, true);
    timerAlarmWrite(timer, 20000, true);
    timerAlarmEnable(timer);

    InOut = mypins();
    Led = new WS2812((gpio_num_t)LED_PIN,LED_COUNT);
    st = Settings();
    //Led->clear();
    Led->setAllPixels({0,0,0});
    Led->show();

    // Hinzufügen Ecxhtzeituhr
    pZeit = new Zeitmaster();

    if (!SerialBT.begin("MyTV_Movit_V2"))
    {
        // nop
    }

    pinMode(INTERRUPT_PIN, INPUT);
    attachInterrupt(INTERRUPT_PIN, Ext_INT1_ISR, RISING);
    
    // Einlesen der NV Werte
    st.getSavedColor( );
    st.getSavedTurnOnValue( );
    st.getSavedUpperCollisionADCValue( );
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
  
    // Eventgetriggerte Steuerung der Bewegung und der LEDs
    if( event )
    {
        // Einlesen des Inputstreams from Bluetooth
        if (SerialBT.available())
            appinterpreter.readCommandCharFromApp( (char)SerialBT.read() );

        // Einlesen des Inputstreams from Serial
        if (Serial.available())
            appinterpreter.readCommandCharFromSerial( (char)Serial.read() );

        // Startup Timer um zuerst gültige Werte fürs Verfahren zu bekommen
        st.startUpTimer();

        tvState = InOut.getTVstate( InOut.sendCurrentADCValues );

        // LED Steuerung R/G/B
        if( !InOut.collisionDetected )
        {
            if( InOut.colorchanged )
            {
                targetColor = st.getColor();
                iLedCntr = 0;
                st.saveActColor( );
                InOut.colorchanged = 0;
            }
            InOut.collisionDetected = InOut.getFiltMotCurrent();
        }
        else
        {
            targetColor = {255,0,0};
            iLedCntr = 0;
        }

        // change LED color
        if( iLedCntr < LED_COUNT )
        {
            Led->setPixel( iLedCntr, targetColor );
            Led->show();
            iLedCntr ++;
        }

        // Rücksetzen des Events
        event = 0;
    }
    
    /*************************************************************************************
     * solange keine Kollision erkannt wurde und die Startzeit abgewartet wurde:
     * Bewege den Fernseher falls nötig / gewünscht
     ************************************************************************************/
    if( !InOut.collisionDetected && st.initTimeOver )
    {
        // Automatisches Verfahren des Fernsehers
        if( st._AutoMove ) 
            dirOut = InOut.setMotorDir( tvState );  // Test for steps: change "tvState" to InOut.getTestPinState()
        // Manuelles Verfahren des Fernsehers - Initial nicht aktiv 
        else
            dirOut = InOut.setMotorDir( (st._ManMoveDir == 2) ? 0 : 1 );

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