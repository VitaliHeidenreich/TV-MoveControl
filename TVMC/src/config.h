#ifndef CONFIG_H
#define CONFIG_H
#pragma once
#include <Arduino.h>
#include "Settings.h"


/***************************************************************************
 * Serielle Verbindung
 **************************************************************************/
#define SERIAL_SPEED 115200

 
#define TESTBUTTON      5
//Endstipps
#define ET1             16
#define ET2             4
// Motoren
#define DRK             25
#define BRK             12
// Test Pin
#define TESTPIN         23

// LED
#define LED_PIN         2
#define LED_IN          18
#define LED_MOT_ON      19

// Motoransteuerungsvarianten
// 1 Blockansteuerung
// 0 PWM
#define MOT_CONT_PR     0

// Weitere Parameter
#define FREQUENZ        650
#define AUFLOESUNG      8
#define MAX_SPEED       220
#define MIN_SPEED       100
#define MAX_STEP_TV     10000

// Einlesen und steuern TV Zustand
#define TVPIN           35
#define TV_MEASNUMB     1000
#define TVONVALUE       1405
#define TVOFFVALUE      1390


// Over Current Protection Motor
#define CURRENTMEASPIN  34
#define CURRENTNUMVAL   50
#define OVERCURDETVAL   1422

// Debug switch
#define DEBUG           1

#define INTERRUPT_PIN   17


/***************************************************************************
 * App Interpreter Einstellungen --> Communication sign
 **************************************************************************/
#define CONV_VERSION 2


#endif /* MYHEADER_H */