#include "TBElevator.h"
#include "ArduinoProxy.h"

const int CALIB_BTN_PIN = 13;
const int STATE_LED_PIN = 11;

TBElevator::TBElevator()
	: m_currentState(ELEV_STATE::IDLE), savedTime(micros()), ledState(LOW)
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

bool TBElevator::tryMove(bool down)
{
	timeNow = micros();
	if (timeNow - savedTime > timeInterval) {
		phase8(down);
		savedTime = micros();
		return true;
	}
	return false;
}

void TBElevator::SetState(ELEV_STATE state)
{
	m_currentState = state;
}

void TBElevator::Tick(const unsigned long& millis, BTN_ACTION CurCalibBtnAction)
{
	//BTN_ACTION CurCalibBtnAction = BTN_ACTION::NONE; // FIX THIS
	if(CurCalibBtnAction == BTN_ACTION::LONG_PRESS)
	{
		m_currentState = ELEV_STATE::CALIBRATION_STARTED;
	}

	switch (m_currentState)
	{
	case ELEV_STATE::IDLE:
		if (CurCalibBtnAction == BTN_ACTION::DOWN && elevSteps != 0)
		{
			digitalWrite(STATE_LED_PIN, LOW);
			waitingForPassengers = false; // Start running immediately
			SetState(ELEV_STATE::RUNNING);
		}
		break;
	case ELEV_STATE::CALIBRATION_STARTED:
		// Set LED high
		digitalWrite(STATE_LED_PIN, HIGH);
		// User lowers the lift to the ground and creates tension in the wire
		// User presses the calibration button
		if (CurCalibBtnAction == BTN_ACTION::DOWN)
		{
			elevSteps = curStep = 0;
			SetState(ELEV_STATE::CALIBRATION_IN_PROGRESS);
		}
		break;
	case ELEV_STATE::CALIBRATION_IN_PROGRESS:
		if (millis - ledBlinkTimestamp > LED_BLINK_SPEED)
		{
			ledState = !ledState;
			ledBlinkTimestamp = millis;
			digitalWrite(STATE_LED_PIN, ledState);
		}

		// Start moving up and count steps
		if (tryMove(false))
		{
			elevSteps++;
		}

		// User presses calibration button
		if (CurCalibBtnAction == BTN_ACTION::DOWN)
		{
			curStep = elevSteps;
			moveDown = true;
			waitingForPassengers = false;
			digitalWrite(STATE_LED_PIN, LOW);
			SetState(ELEV_STATE::RUNNING);
		}
		break;
	case ELEV_STATE::RUNNING:

		if (!waitingForPassengers)
		{
			if (tryMove(moveDown))
			{
				curStep--;
			}
		}
		else if (millis - waitForPassengersTimestamp > WAIT_FOR_PASSENGERS_DURATION)
		{
			waitingForPassengers = false;
		}

		if (curStep == 0)
		{
			curStep = elevSteps;
			moveDown = !moveDown;
			waitingForPassengers = true;
			waitForPassengersTimestamp = millis;
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

