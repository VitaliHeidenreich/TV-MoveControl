#ifndef CONFIG_H
#define CONFIG_H
#pragma once
#include <Arduino.h>
#include "Settings.h"


/***************************************************************************
 * Serielle Verbindung
 **************************************************************************/
#define SERIAL_SPEED 115200

 
#define TVSTATE 5
//Endstipps
#define ET1     16
#define ET2     4
// Motoren
// #define EN_A    13
#define EN_B    25
// #define IN_2A   26
// #define IN_1A   27
#define IN_2B   14
#define IN_1B   12
// Test Pin
#define TESTPIN 23

// LED
#define LED_PIN 2
#define LED_IN  18
#define LED_OUT 19

// Weitere Parameter
#define FREQUENZ 20
#define AUFLOESUNG 8
#define MAX_PWM  115

// Einlesen und steuern TV Zustand
#define TVPIN   35
#define TV_MEASNUMB   250
#define TVONVALUE 2535
#define TVOFFVALUE 2100


// Over Current Protection Motor
#define CURRENTMEASPIN  34
#define CURRENTNUMVAL   30
#define OVERCURDETVAL   1920


/***************************************************************************
 * App Interpreter Einstellungen --> Communication sign
 **************************************************************************/
#define CONV_VERSION 2


#endif /* MYHEADER_H */