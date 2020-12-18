#include "config.h"

void init_gpios(void)
{
    pinMode(TVSTATE,INPUT);
    pinMode(ET1,INPUT);
    pinMode(ET2,INPUT);

    //pinMode(EN_A,OUTPUT);
    ledcSetup(0, 1000, 8);
    ledcAttachPin(EN_A, 0);
    //pinMode(EN_B,OUTPUT);
    ledcSetup(0, 1000, 8);
    ledcAttachPin(EN_B, 1);

    pinMode(IN_1A,OUTPUT);
    pinMode(IN_2A,OUTPUT);
    pinMode(IN_1B,OUTPUT);
    pinMode(IN_2B,OUTPUT);
}