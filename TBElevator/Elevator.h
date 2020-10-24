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
	// By default
	// 	the motor is operating on pins 4, 5, 6, 7 (Stepper port pin assignment. Default 0b11110000 on arduino port D.)
	// 	the led is on pin 11
	// 	the night time switch on pin 9
	// 	the motor step interval is 1200 micros
	// 	the led blink speed is 500 millis
	// 	the serial ports are on 0,1 (Serial port pins mask for arduino port D.)
	TBElevator(
		unsigned int waitForPassengersSec = 5, int ledPin = 11, int switchPin = 9,
		unsigned int stepperMotorPort = 0xF0, unsigned long ledBlinkSpeedMillis = 500,
		unsigned long motorStepIntervalMicros = 1200, unsigned int serialMask = 0x30);

	const unsigned long MOTOR_STEP_INTERVAL;		  // microseconds
	const unsigned long WAIT_FOR_PASSENGERS_DURATION; // microseconds
	const unsigned long LED_BLINK_SPEED;			  // microseconds

	const int STATE_LED_PIN;
	const int NIGHT_TIME_SWITCH_PIN;

	ELEV_STATE &GetState()
	{
		return m_currentState;
	}
	int &GetSteps()
	{
		return m_totalSteps;
	}
	int &GetCurStep()
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
	void Tick(const unsigned long &micros, BTN_ACTION CurCalibBtnAction);
	void Pause();
	void Unpause();

private:
	void init();
	void SetLED(unsigned int state);
	void phase8(bool isClockwise);
	bool tryMove(bool down, const unsigned long &micros);

	//+++++++++++ Variables +++++++++++++++
	// Track current stepper phase by index
	int phaseIndex = 0;

	//++++++++++++++ Initialise Constants ++++++++++++++++

	// Binary mask indicating the pins used by the stepper motor
	const unsigned int STEPPER_PORT;

	// Binary mask indicating the pins used for serial communication
	const unsigned int SERIAL_MASK;

	// Motor 8-phase sequence lookup list
	// pins 7, 6, 5, 4 (0011, 0010, 0110, 0100, 1100, 1000, 1001, 0001)
	const unsigned int STEPPERPHASES8[8] = {0x30, 0x20, 0x60, 0x40, 0xC0, 0x80, 0x90, 0x10};
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