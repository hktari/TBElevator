/*
  DSM2_tx implements the serial communication protocol used for operating
  the RF modules that can be found in many DSM2-compatible transmitters.

  Copyrigt (C) 2012  Erik Elmore <erik@ironsavior.net>

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.  
*/

#include <sys/timeb.h>
#include "..\ArduinoProxy.h"
#include <stdexcept>

const int DIGITAL_PIN_COUNT = 12; // 0 - 11
unsigned int digitalPinValues[DIGITAL_PIN_COUNT];

timeb t_start;
void SetPortD(unsigned int val)
{
	_portD = val;
}
unsigned int GetPortD()
{
	return _portD;
}
void SetDDRD(unsigned int val)
{
}
unsigned int GetDDRD()
{
	return 0;
}
void pinMode(unsigned int pin, unsigned int mode)
{
}
void digitalWrite(unsigned int pin, unsigned int val)
{
  if(pin < 0 || pin > DIGITAL_PIN_COUNT - 1)
  {
    throw std::invalid_argument("digital pin out of bounds trying to write");
  }

  digitalPinValues[pin] = val;
}

int digitalRead(unsigned int pin)
{
  if(pin < 0 || pin > DIGITAL_PIN_COUNT - 1)
  {
    throw std::invalid_argument("digital pin out of bounds trying to write");
  }

  return digitalPinValues[pin];
}

unsigned long millis() {
  timeb t_now;
  ftime(&t_now);
  return (t_now.time  - t_start.time) * 1000 + (t_now.millitm - t_start.millitm);
}

unsigned long micros() {
	timeb t_now;
	ftime(&t_now);
	return (t_now.time - t_start.time) * 1E6 + (t_now.millitm - t_start.millitm);
}

void delay(unsigned long ms) {
  unsigned long start = millis();
  while(millis() - start < ms){}
}

void initialize_mock_arduino() {
  ftime(&t_start);
}
