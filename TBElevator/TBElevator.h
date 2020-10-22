#pragma once

#define DEFAULT_TIME 0UL

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

	unsigned long MOTOR_STEP_INTERVAL = 1200; // microseconds
	const unsigned long WAIT_FOR_PASSENGERS_DURATION = 5E6; // microseconds
	const unsigned long LED_BLINK_SPEED = 500000; // microseconds

	static const int CALIB_BTN_PIN = 13;
	static const int STATE_LED_PIN = 11;

	ELEV_STATE& GetState() 
	{
		return m_currentState;
	}
	int& GetSteps()
	{
		return m_totalSteps;
	}
	int& GetCurStep()
	{
		return m_curStep;
	}
	bool IsMovingDown()
	{
		return m_currentState == ELEV_STATE::RUNNING && m_moveDown && !IsWaitingForPassengers();
	}
	bool IsMovingUp()
	{
		return m_currentState == ELEV_STATE::RUNNING && !m_moveDown && !IsWaitingForPassengers();
	}
	bool IsWaitingForPassengers()
	{
		return m_isWaitingForPassengers;
	}

	void SetState(ELEV_STATE state);
	void Tick(const unsigned long& micros, BTN_ACTION CurCalibBtnAction);
private:

	void SetLED(unsigned int state);
	void phase8(bool isClockwise);
	bool tryMove(bool down, const unsigned long& micros);

	//+++++++++++ Variables +++++++++++++++
	// Track current stepper phase by index
	int phaseIndex = 0;

	//++++++++++++++ Initialise Constants ++++++++++++++++
	// Stepper port pin assignment. Default 0b11110000 on port D.
	const unsigned int STEPPERPORT = 0xF0;
	// Serial port pins mask for port D.
	const unsigned int SERIALMASK = 0x03;
	// Motor 8-phase sequence lookup list
	// pins 7, 6, 5, 4 (0011, 0010, 0110, 0100, 1100, 1000, 1001, 0001)
	const unsigned int STEPPERPHASES8[8] = { 0x30, 0x20, 0x60, 0x40, 0xC0, 0x80, 0x90, 0x10 };
	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++


	ELEV_STATE m_currentState;
	int m_totalSteps;

	bool rotateDirection = true;
	unsigned long savedTime = DEFAULT_TIME;
	unsigned long timeNow = DEFAULT_TIME;

	bool m_isWaitingForPassengers = false;
	unsigned long waitForPassengersTimestamp = DEFAULT_TIME;
	int m_curStep = 0;
	bool m_moveDown;
	int ledState;
	unsigned long ledBlinkTimestamp = DEFAULT_TIME;
};