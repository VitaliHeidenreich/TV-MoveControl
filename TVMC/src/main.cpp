#include <Arduino.h>
#include "config.h"
#include "mypins.h"
#include "WS2812B.h"

WS2812 *Led;
mypins InOut;

hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
uint8_t event;
 
void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  event = 1;
  portEXIT_CRITICAL_ISR(&timerMux);
 
}

void setup()
{
  Serial.begin(115200);
  InOut = mypins();
  Led = new WS2812((gpio_num_t)LED_PIN,3);
  Led->clear();

  // Timer Funktionen
  event = 0;
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 500000, true);
  timerAlarmEnable(timer);
}


void loop()
{
  static uint8_t dirOut = 0;
  static uint8_t tvState = 0;
  /**********************************************************************
   * Den Zustand des Fernsehers feststellen und einstellen der Richtung
   *********************************************************************/
  if( event )
  {
    tvState = InOut.getTVstate( );
    event = 0;
  }

  dirOut = InOut.setMotorDir( InOut.getTestPinState() );  // ToDo: change to "tvState
  

  /**********************************************************************
   * Position des Fernstehers feststellen
   * * Out-Stopp ist    ET2
   * * In-Stopp ist     ET1
   *********************************************************************/
  if( ( (GET_ET1)&&(dirOut==1) ) || ( (GET_ET2)&&(dirOut==0) ) )
  {
    InOut.setMotorSpeed( MAX_PWM );
  }
  else
  {
    InOut.setMotorSpeed( 0 );
  }
  
  InOut.showEndStoppState( GET_ET1, GET_ET2);

  // LED steuerung R/G/B
  Led->setAllPixels(255,255,255);
  Led->show();
}