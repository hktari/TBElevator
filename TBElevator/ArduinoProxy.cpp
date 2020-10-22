// TODO: implement the functions which map to and from the Arduino.h
#include <Arduino.h>
#include "ArduinoProxy.h"

void SetPortD(unsigned int val)
{
    PORTD = val;
}
unsigned int GetPortD()
{
    return PORTD;
}

void SetDDRD(unsigned int val)
{
    DDRD = val;
}
unsigned int GetDDRD()
{
    return DDRD;
}