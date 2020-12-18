
#include "config.h"
#include "init_and_help_fnx.h"


void setup() {
  init_gpios();
}

void loop() {
  uint8_t dirOut = 0;

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


  if(!GET_ET1 && dirOut==1)
  {
    ledcWrite(0, 180);
  }
  else if(!GET_ET2 && dirOut==0)
  {
    ledcWrite(0, 180);
  }
  else
  {
    ledcWrite(0, 0);
  }
  
  
}