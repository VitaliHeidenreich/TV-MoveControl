#pragma once

#include "Arduino.h"
#include "Settings.h"
#include "mypins.h"

#define DEBUG_APPINTERPRETER 0

class App
{
public:
     //Konstruktor
     App();

     /****************************************
     * App Befehle einlesen
     ***************************************/
     uint8_t readCommandCharFromApp(char CommandChar);

     void setCommand(char AppBefehl[6]);
     uint8_t *getCommand();


private:
     /****************************************
     * Ansteuerbefehle aus der App
     ***************************************/
     void CommSetColor(char AppBefehl[6]);
     void CommSetBrightness(char AppBefehl[6]);
     void CommSetTime(char AppBefehl[6]);
     void CommSetAutomover(char AppBefehl[6]);
     void CommManMove(char AppBefehl[6]);
     // Geschwindigkeit und Frequenz des Boards per App einstellen
     void setFrequenzAndSpeed( char *_AppBefehl );


     /****************************************
      * Hilfsfunktionen für Datenkonvertierung
     ***************************************/
     byte _convertVarToByte(uint32_t ArrayData);
     bool _convertVarToBool(uint32_t ArrayData);
     uint16_t _convertVarToUint16(uint32_t ArrayData);
     uint8_t _hexcharToUint8_t(char);
     void justSendTheFoundStringToSerial(char *p);
     uint8_t AppBefehl[6];

     /****************************************
      * Direct key to class Settings
     ***************************************/
     Settings *settings;
     mypins *InOut;
};
