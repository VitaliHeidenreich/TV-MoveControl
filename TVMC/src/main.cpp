#include <Arduino.h>
#include "config.h"
#include "mypins.h"
#include "WS2812B.h"
#include "App.h"

#include "BluetoothSerial.h"
#include "WS2812B.h"
#include "time.h"

WS2812 *Led;
mypins InOut;
Settings st;

hw_timer_t * timer = NULL;
hw_timer_t * testTimer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
uint8_t event;
uint8_t senderTrigger = 0;
#if DEBUG == 2
    uint8_t semaphore = 1;
#endif

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
    timerAlarmWrite(timer, 10000, true);
    timerAlarmEnable(timer);

    InOut = mypins();
    Led = new WS2812((gpio_num_t)LED_PIN,112);
    st = Settings();
    Led->clear();

    if (!SerialBT.begin("MyTV_TEST")){
        //Serial.println("An error occurred initializing Bluetooth");
    }

    pinMode(INTERRUPT_PIN, INPUT);
    attachInterrupt(INTERRUPT_PIN, Ext_INT1_ISR, RISING);
    
    st.getSavedColor( );
    Serial.println("ESP32 gestartet.");
}

static uint8_t dirOut = 0;
static uint8_t tvState = 0;

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
        st.startUpTimer();

        tvState = InOut.getTVstate( );

        // Also set leds
        // LED Steuerung R/G/B
        if( !InOut.collisionDetected )
        {
            if( InOut.colorchanged )
            {
                Led->setAllPixels(st.getColor());
                Led->show();
                st.saveActColor( );
                InOut.colorchanged = 0;
            }
            InOut.collisionDetected = InOut.getFiltMotCurrent();
        }
        else
        {
            Led->setAllPixels({255,0,0});
            Led->show();
        }

        senderTrigger ++;  // Braucht man das noch ???
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
        if((( OUT_SENSSTATE ) && ( dirOut==1 ) && InOut.rangeCheck(dirOut)) || (( IN_SENSSTATE ) && ( dirOut==0 ) && InOut.rangeCheck(dirOut)))
        {
            InOut.setMotorSpeed( MAX_SPEED );
            #if DEBUG == 2
            if( semaphore == 1 )
            {
                semaphore = 0;
            }
            #endif
        }
        else
        {
            InOut.setMotorSpeed( 0 );
            #if DEBUG == 2
                if( semaphore == 0 )
                {
                    Serial.print("End Steps: ");
                    Serial.print(InOut.ActualStepTVBoard);
                    Serial.print(" DirOut: ");
                    Serial.println(dirOut);
                    semaphore = 1;
                }
            #endif
        }
    }
    // Collision detected
    else
    {
        InOut.setMotorSpeed( 0 );
    }
    
    // Einlesen des Inputstreams
    if (SerialBT.available())
    {
        appinterpreter.readCommandCharFromApp( (char)SerialBT.read() );
    }
}

/************************************************************************************************/
/************************************************************************************************/
/************************************************************************************************/
/************************************************************************************************/