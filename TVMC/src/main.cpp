#include <Arduino.h>
#include "config.h"
#include "init_and_help_fnx.h"
#include "WS2812B.h"

WS2812 *Led;

void setup()
{
  init_gpios();
  Led = new WS2812((gpio_num_t)LED_PIN,5,0);
}


void loop()
{
  uint8_t dirOut = 0;

  /**********************************************************************
   * Den Zustand des Fernsehers feststellen
   *********************************************************************/
  if( GET_TVSTATE )
  {
    // Fernseher ausfahren
    digitalWrite(IN_1A, 0);
    digitalWrite(IN_1B, 1);
    dirOut = 1;
  }
  else
  {
    // Fernseher einfahren
    digitalWrite(IN_1A, 1);
    digitalWrite(IN_1B, 0);
    dirOut = 0;
  }

  /**********************************************************************
   * Position des Fernstehers feststellen
   *********************************************************************/
  if( (!GET_ET1 && dirOut==1) || (!GET_ET2 && dirOut==0) )
  {
    ledcWrite(0, 180);
  }
  else
  {
    ledcWrite(0, 0);
  }


  // LED steuerung
  Led->setAllPixels({120,0,0});
  Led->show();
}