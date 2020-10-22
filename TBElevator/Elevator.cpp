#include "Elevator.h"
#ifndef TEST_ENV
#include <Arduino.h>
#endif
#include "ArduinoProxy.h"

TBElevator::TBElevator()
	: m_currentState(ELEV_STATE::IDLE), ledState(LOW), savedTime(micros()), ledBlinkTimestamp(micros()), waitForPassengersTimestamp(micros())
{
	// Set stepper pins to output
	SetDDRD(GetDDRD() | STEPPERPORT);
}

void TBElevator::phase8(bool isClockwise) {
	// Clear stepper pins
	SetPortD(GetPortD() & SERIALMASK);

	// Alter rotation direction
	bool rotateDirection;
	if (isClockwise == true) {
		rotateDirection = true;
	}
	else {
		rotateDirection = false;
	}

	if (rotateDirection == true) {
		// Output the next stepper phase - Clockwise
		SetPortD(GetPortD() | STEPPERPHASES8[phaseIndex]);
		if (phaseIndex == 7) {
			phaseIndex = 0;
		}
		else {
			phaseIndex++;
		}
	}
	else {
		// Output the next stepper phase - Counter Clockwise
		SetPortD(GetPortD() | STEPPERPHASES8[phaseIndex]);
		if (phaseIndex == 0) {
			phaseIndex = 7;
		}
		else {
			phaseIndex--;
		}
	}
}

bool TBElevator::tryMove(bool down, const unsigned long& micros)
{
	timeNow = micros;
	if (timeNow - savedTime >= MOTOR_STEP_INTERVAL) {
		phase8(down);
		savedTime = micros;
		return true;
	}
	return false;
}

void TBElevator::SetState(ELEV_STATE state)
{
	m_currentState = state;
}

void TBElevator::SetLED(unsigned int state)
{
	ledState = state;
	digitalWrite(STATE_LED_PIN, state);
}

void TBElevator::Tick(const unsigned long& micros, BTN_ACTION CurCalibBtnAction)
{
	//BTN_ACTION CurCalibBtnAction = BTN_ACTION::NONE; // FIX THIS
	if(CurCalibBtnAction == BTN_ACTION::LONG_PRESS)
	{
		m_currentState = ELEV_STATE::CALIBRATION_STARTED;
	}

	switch (m_currentState)
	{
	case ELEV_STATE::IDLE:
		if (CurCalibBtnAction == BTN_ACTION::DOWN && m_totalSteps != 0)
		{
			SetLED(LOW);
			m_isWaitingForPassengers = false; // Start running immediately
			SetState(ELEV_STATE::RUNNING);
		}
		break;
	case ELEV_STATE::CALIBRATION_STARTED:
		// Set LED high
		SetLED(HIGH);
		// User lowers the lift to the ground and creates tension in the wire
		// User presses the calibration button
		if (CurCalibBtnAction == BTN_ACTION::DOWN)
		{
			m_totalSteps = m_curStep = 0;
			savedTime = micros;
			SetState(ELEV_STATE::CALIBRATION_IN_PROGRESS);
		}
		break;
	case ELEV_STATE::CALIBRATION_IN_PROGRESS:
		// User presses calibration button
		if (CurCalibBtnAction == BTN_ACTION::DOWN)
		{
			m_curStep = m_totalSteps;
			m_moveDown = true;
			m_isWaitingForPassengers = false;
			SetLED(LOW);
			SetState(ELEV_STATE::RUNNING);
		}
		else
		{
			if (micros - ledBlinkTimestamp >= LED_BLINK_SPEED)
			{
				ledBlinkTimestamp = micros;
				SetLED(!ledState);
			}

			// Start moving up and count steps
			if (tryMove(false, micros))
			{
				m_totalSteps++;
			}
		}
		break;
	case ELEV_STATE::RUNNING:

		if (!m_isWaitingForPassengers)
		{
			if (tryMove(m_moveDown, micros))
			{
				m_curStep--;
			}
		}
		else if (micros - waitForPassengersTimestamp >= WAIT_FOR_PASSENGERS_DURATION)
		{
			m_isWaitingForPassengers = false;
		}

		if (m_curStep == 0)
		{
			m_curStep = m_totalSteps;
			m_moveDown = !m_moveDown;
			m_isWaitingForPassengers = true;
			waitForPassengersTimestamp = micros;
		}

		if (CurCalibBtnAction == BTN_ACTION::DOWN)
		{
			SetState(ELEV_STATE::IDLE);

			// Clear stepper pins
			SetPortD(GetPortD() & SERIALMASK);
		}
		break;
	}

}

