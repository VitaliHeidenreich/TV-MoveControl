#include <Arduino.h>
#include "config.h"
#include "mypins.h"
#include "WS2812B.h"
#include "App.h"

#include "BluetoothSerial.h"
#include "WS2812B.h"
#include "time.h"

//0F903A

WS2812 *Led;
mypins InOut;
Settings st;

static uint32_t anzeiger = 0;

hw_timer_t * timer = NULL;
hw_timer_t * testTimer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
uint8_t event;
uint8_t senderTrigger = 0;

/***************************************************************************
 * Anlegen der Peripherie Instanzen
 **************************************************************************/
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
    InOut = mypins();
    Led = new WS2812((gpio_num_t)LED_PIN,112);
    st = Settings();
    Led->clear();

    if (!SerialBT.begin("MyTV_TEST")){
        Serial.println("An error occurred initializing Bluetooth");
    }

    // Timer Funktionen
    event = 0;
    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &onTimer, true);
    timerAlarmWrite(timer, 10000, true);
    timerAlarmEnable(timer);
    
    st.getSavedColor( );
    Serial.println("ESP32 gestartet.");
}


void loop()
{ 
  static uint8_t dirOut = 0;
  static uint8_t tvState = 0;
  static uint8_t lastStateCollision = 1;
  /**********************************************************************
   * solange keine Kollision erkannt wurde:
   * Den Zustand des Fernsehers feststellen und einstellen der Richtung
   * Abfrage erfolgt alle 500ms
   *********************************************************************/
  
    // Eventgetriggerte Steuerung der Bewegung und der LEDs
    if( event )
    {
        st.startUpTimer();

        tvState = InOut.getTVstate( );

        // Also set leds
        // LED Steuerung R/G/B
        if( !InOut.collisionDetected || lastStateCollision )
        {
            if( InOut.colorchanged )
            {
                Led->setAllPixels(st.getColor());
                Led->show();
                InOut.colorchanged = 0;
                st.saveActColor( );
            }
            InOut.collisionDetected = InOut.getFiltMotCurrent();
            lastStateCollision = 0;
        }

        senderTrigger ++;
        event = 0;
    }
    
    /**********************************************************************
     * solange keine Kollision erkannt wurde und die Startzeit abgewartet wurde:
     * Bewege den Fernseher falls nötig / gewünscht
     *********************************************************************/
    if( !InOut.collisionDetected && st.moveEna )
    {
        // Automatisches Verfahren des Fernsehers
        if( st._AutoMove ) // Initial nicht aktiv
        {
            dirOut = InOut.setMotorDir( tvState );  // ToDo: change InOut.getTestPinState() to "tvState" 

            /**********************************************************************
             * Ist-Position des Fernstehers feststellen und Abgleich mit dem Soll
             * * Out-Stopp ist    ET2
             * * In-Stopp ist     ET1
             *********************************************************************/
            if(((( OUT_SENSSTATE ) && ( dirOut==1 )) || (( IN_SENSSTATE ) && ( dirOut==0 ))))
            {
                // Korrektur der Geschwindigkeiten
                if(dirOut)
                    InOut.setMotorSpeed( MAX_PWM - 10 );
                else
                    InOut.setMotorSpeed( MAX_PWM );
            }
            else
            {
                InOut.setMotorSpeed( 0 );
            }
        }
        else
        {
            // Manuelles Verfahren des Fernsehers
            // wenn 0, dann ist keine Richtung in der App gewählt
            if(st._ManMoveDir || !InOut.getTestPinState() )
            {
                dirOut = InOut.setMotorDir( (st._ManMoveDir == 2) ? 0 : 1 );

                if( ( (OUT_SENSSTATE)&&(dirOut==1) ) || ( (IN_SENSSTATE)&&((dirOut==0) ) ) || !InOut.getTestPinState() )
                {
                    // Korrektur der Geschwindigkeiten
                    if(dirOut)
                        InOut.setMotorSpeed( MAX_PWM - 10  );
                    else
                        InOut.setMotorSpeed( MAX_PWM );
                }
                else
                {
                    InOut.setMotorSpeed( 0 );
                }
            }
            else
            {
                InOut.setMotorSpeed( 0 );
            }
        }
    }
    // Collision detected
    else
    {
        InOut.setMotorSpeed( 0 );
        InOut.setOnboardLed( 1 );
    }

    //SerialBT.available()
    if (SerialBT.available())
    {
        // (char)SerialBT.read()
        appinterpreter.readCommandCharFromApp( (char)SerialBT.read()/*(char)Serial.read()*/ );
    }
    

    // if( senderTrigger >= 100 )
    // {
    //     Serial.println((anzeiger++));  
    //     senderTrigger = 0;
    // }
}