#pragma once

#include <cstdint>

enum class ELEV_STATE
{
	IDLE,
	CALIBRATION_STARTED,
	CALIBRATION_IN_PROGRESS,
	RUNNING,
};
enum class BTN_ACTION
{
	NONE = 0,
	DOWN = 1,
	UP = 2,
	LONG_PRESS = 3
};

class TBElevator 
{
public:
	TBElevator();

	ELEV_STATE GetState() 
	{
		return m_currentState;
	}

	void SetState(ELEV_STATE state);
	void Tick(unsigned long& millis);

	void phase8(bool isClockwise);
	bool tryMove(bool down);

private:
	//+++++++++++ Variables +++++++++++++++
	// Track current stepper phase by index
	int phaseIndex = 0;

	//++++++++++++++ Initialise Constants ++++++++++++++++
	// Stepper port pin assignment. Default 0b11110000 on port D.
	const uint8_t STEPPERPORT = 0xF0;
	// Serial port pins mask for port D.
	const uint8_t SERIALMASK = 0x03;
	// Motor 8-phase sequence lookup list
	// pins 7, 6, 5, 4 (0011, 0010, 0110, 0100, 1100, 1000, 1001, 0001)
	const uint8_t STEPPERPHASES8[8] = { 0x30, 0x20, 0x60, 0x40, 0xC0, 0x80, 0x90, 0x10 };
	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++


	ELEV_STATE m_currentState;

	bool rotateDirection = true;
	long savedTime;
	long timeNow;
	long timeInterval = 1200;

	bool waitingForPassengers = false;
	const long WAIT_FOR_PASSENGERS_DURATION = 5000; // ms
	long waitForPassengersTimestamp;

	int elevSteps = 0;
	int curStep = 0;
	bool moveDown;
	int ledState;
	long ledBlinkTimestamp = 0;
	const long LED_BLINK_SPEED = 500; // ms
};