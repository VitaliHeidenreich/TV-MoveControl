#include <Arduino.h>
#include "config.h"
#include "mypins.h"
#include "WS2812B.h"
#include "App.h"
#include <EEPROM.h>

#include "BluetoothSerial.h"
#include "WS2812B.h"
#include "time.h"

#define EEPROM_SIZE 4


WS2812 *Led;
mypins InOut;
Settings st;

hw_timer_t * timer = NULL;
hw_timer_t * testTimer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
uint8_t event;

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

    if (!SerialBT.begin("MyTV_017558HM"))
    {
        Serial.println("An error occurred initializing Bluetooth");
    }

    // Timer Funktionen
    event = 0;
    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &onTimer, true);
    timerAlarmWrite(timer, 10000, true);
    timerAlarmEnable(timer);

    EEPROM.begin(EEPROM_SIZE);
    st.setColor(EEPROM.read(0),EEPROM.read(1),EEPROM.read(2));
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
                // Zum Testen
                EEPROM.write(0, st.getColor().red);
                EEPROM.write(1, st.getColor().green);
                EEPROM.write(2, st.getColor().blue);
                EEPROM.commit();
            }
            InOut.collisionDetected = InOut.getFiltMotCurrent();
            lastStateCollision = 0;
        }
        else
        {
            //Show collosion detected by blinking the lights
            if( st.blinkCollision( 1 ) )
            {
                Led->setAllPixels({0,0,0});
            }
            else
            {
                Led->setAllPixels(st.getColor());
            }
            lastStateCollision = 1;
        }
        
        event = 0;
    }
    
    /**********************************************************************
     * solange keine Kollision erkannt wurde:
     * Bewege den Fernseher falls nötig / gewünscht
     *********************************************************************/
    if( !InOut.collisionDetected )
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
            if( ( ( (OUT_SENSSTATE)&&(dirOut==1) ) || ( (IN_SENSSTATE)&&(dirOut==0) ) ) )
            {
                InOut.setMotorSpeed( MAX_PWM );
            }
            else
            {
                InOut.setMotorSpeed( 0 );
            }
        }
        // Manuelles Verfahren des Fernsehers
        else
        {
            if(st._ManMoveDir || !InOut.getTestPinState() ) // wenn 0, dann ist keine Richtung in der App gewählt
            {
                dirOut = InOut.setMotorDir( (st._ManMoveDir == 2) ? 0 : 1 );

                if( ( (OUT_SENSSTATE)&&(dirOut==1) ) || ( (IN_SENSSTATE)&&((dirOut==0) ) ) || !InOut.getTestPinState() )
                {
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


    char c;
    if (SerialBT.available())
    {
        c = (char)SerialBT.read();
        Serial.println(c);
        //ToDo: Erster Test wird werden, die Zeit ueber die App vorzugeben
        appinterpreter.readCommandCharFromApp( c );
    }
}