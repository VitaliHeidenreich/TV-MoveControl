#ifndef DEFINES_H
#define DEFINES_H

/***************************************************************************
 * Serielle Verbindung
 **************************************************************************/
#define SERIAL_SPEED 115200

/***************************************************************************
 * App Interpreter Einstellungen --> Communication sign
 **************************************************************************/
#define CONV_VERSION 2

#define ON 1
#define OFF 0

#define COLLISION 1
#define NO_COLLISION 0

// Anzahl der LEDS --> Fernseher: 112
#define LED_CNT_SUM 30
#define LED_CNT_UPPER_ROW 10

// Timer defines
#define PRESCALER 80
#define INTERRUPTTIME 20 // in ms

#define RED     {100,0,0}
#define GREEN   {0,100,0}
#define BLUE    {0,0,100}

// EEPROM address maping
#define I2C_ADR_COLOR           0x01
#define I2C_ADR_MOTOR_COLLISION 0x04
#define I2C_ADR_TV_ON_VALUE     0x06
#define I2C_ADR_TV_OFF_VALUE    0x08
#define I2C_ADR_TV_DIV_VALUE    0x10
#define I2C_ADR_LIGHT_EFFECT    0x11
#define I2C_ADR_OTHER           0x12


//Endstipps
#define ET1             16
#define ET2             4
// Motoren
#define DRK             25
#define BRK             12

// LED
#define LED_PIN         14

// Button defines
#define BTN_IN          18
#define BTN_OUT         19
#define BTN_MULTI       23

// Motoransteuerungsvarianten
// 1 Blockansteuerung
// 0 PWM
#define MOT_CONT_PR     0

#define INTERRUPT_PIN   17

// Einlesen und steuern TV Zustand
#define TVPIN           35
#define TV_MEASNUMB     1000

// Timings
#define DEBOUNCETIME    45
#define SHORTTIMEPUSH   300

// Over Current Protection Motor
#define CURRENTMEASPIN  34
#define CURRENTNUMVAL   50

#endif /* DEFINES_H */