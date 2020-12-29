#include <Arduino.h>
#include "config.h"
#include "mypins.h"
#include "WS2812B.h"

WS2812 *Led;
mypins InOut;

void setup()
{
  InOut = mypins();
  //Led = new WS2812((gpio_num_t)LED_PIN,5,0);
}


void loop()
{
  uint8_t dirOut = 0;
  /**********************************************************************
   * Den Zustand des Fernsehers feststellen
   *********************************************************************/
  dirOut = InOut.setMotorDir( InOut.getTestPinState() );  // ToDo: change

  /**********************************************************************
   * Position des Fernstehers feststellen
   * * Out-Stopp ist    ET2
   * * In-Stopp ist     ET1
   *********************************************************************/
  if( ((GET_ET1) && (dirOut==1)) || ((GET_ET2) && (dirOut==0)) )
  {
    SET_MOT_SPEED(MAX_PWM);
    digitalWrite(EN_A, 1);
  }
  else
  {
    SET_MOT_SPEED(0);
    digitalWrite(EN_A, 0);
  }
    

  InOut.showEndStoppState( GET_ET1, GET_ET2);

  // LED steuerung
  //Led->setAllPixels(170,0,0);
  //Led->show();
}