#include "LowPower.h"

enum class ELEV_STATE
{
	/// <summary>
	/// The elevator has been paused. Can only be swtiched to from RUNNING state
	/// </summary>
	IDLE,

	/// <summary>
	/// The initial state, the elevator has to be calibrated.
	/// </summary>
	CALIBRATION_STARTED,

	CALIBRATION_IN_PROGRESS,

	/// <summary>
	/// The elevator has been calibrated and is moving up and down every x min. If night mode is off,
	/// it checks the photoresistor before starting movement.
	/// </summary>
	RUNNING,

	/// <summary>
	/// Arduino goes into power down mode until the next ride is pending
	/// </summary>
	SLEEPING
};
enum class  BTN_ACTION
{
	NONE = 0,
	DOWN = 1,
	UP = 2,
	LONG_PRESS = 3
};

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

const unsigned long WAIT_FOR_PASSENGERS_DURATION = 5000; // milliseconds
const unsigned long LED_BLINK_SPEED = 1500; // milliseconds
const unsigned long TRIGGER_CALIBRATION_DURATION = 3000; // millis

const int CALIB_BTN_PIN = 12;
const int STATE_LED_PIN = 11;


const uint8_t NIGHT_TIME_SWITCH_PIN = A2;
const uint8_t NIGHT_SENSOR_PIN = A0;

unsigned long timeNow;
long timeInterval = 1200;
bool rotateDirection = true;
unsigned long savedTime = micros();

unsigned long lastCalibBtnDown = 0;
bool longPressCondition = 0;
int prevCalibBtnState = LOW;
ELEV_STATE curState = ELEV_STATE::CALIBRATION_STARTED;
BTN_ACTION CurCalibBtnAction = BTN_ACTION::NONE;

int totalElevSteps = 0;
int curStep = 0;
bool moveDown;
int ledState = LOW;

unsigned long prevMillis, nowMillis = 0;

//#region Forward declarations
//enum class ELEV_STATE;
//void SetState(ELEV_STATE);
//void HandleCalibBtn();
//bool tryMove(bool);
//void phase8(bool);
//#endregion

void SetState(ELEV_STATE state)
{
	Serial.println((int)state);
	curState = state;
}


void phase8(bool isClockwise) {
	// Clear stepper pins
	PORTD = PORTD & SERIALMASK;

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
		PORTD = PORTD | STEPPERPHASES8[phaseIndex];
		if (phaseIndex == 7) {
			phaseIndex = 0;
		}
		else {
			phaseIndex++;
		}
	}
	else {
		// Output the next stepper phase - Counter Clockwise
		PORTD = PORTD | STEPPERPHASES8[phaseIndex];
		if (phaseIndex == 0) {
			phaseIndex = 7;
		}
		else {
			phaseIndex--;
		}
	}
}

bool tryMove(bool down)
{
	timeNow = micros();

	if (timeNow - savedTime > timeInterval) {
		phase8(down);
		savedTime = micros();
		return true;
	}
	return false;
}

void HandleCalibBtn()
{
	int calibBtnState = digitalRead(CALIB_BTN_PIN);

	CurCalibBtnAction = BTN_ACTION::NONE;

	if (calibBtnState == HIGH && prevCalibBtnState != calibBtnState)
	{
		lastCalibBtnDown = millis();
		longPressCondition = true;
		CurCalibBtnAction = BTN_ACTION::DOWN;
	}
	else if (calibBtnState == LOW && prevCalibBtnState != calibBtnState)
	{
		longPressCondition = false;
		CurCalibBtnAction = BTN_ACTION::UP;
	}
	else if (longPressCondition && (millis() - lastCalibBtnDown) > TRIGGER_CALIBRATION_DURATION)
	{
		CurCalibBtnAction = BTN_ACTION::LONG_PRESS;
		longPressCondition = false;
	}

	prevCalibBtnState = calibBtnState;
}

void setLED(unsigned int state)
{
	ledState = state;
	digitalWrite(STATE_LED_PIN, state);
}

void setDirection(bool down)
{
	moveDown = down;
	curStep = totalElevSteps;
	phaseIndex = 0;
}


void clearMotorPorts()
{
	// Clear stepper pins
	PORTD = PORTD & SERIALMASK;
}
bool nightTime()
{
	return analogRead(NIGHT_SENSOR_PIN) <= 70;
}

void blinkLED(const unsigned long& blinkSpeed = LED_BLINK_SPEED)
{
	static unsigned long ledTimer = 0;
	ledTimer += nowMillis - prevMillis;
	if (ledTimer >= blinkSpeed)
	{
		ledTimer = 0;
		setLED(!ledState);
	}
}

void setup() {
	// Set stepper pins to output
	DDRD = DDRD | STEPPERPORT;
	pinMode(CALIB_BTN_PIN, INPUT);
	pinMode(STATE_LED_PIN, OUTPUT);
	//pinMode(NIGHT_SENSOR_PIN, INPUT);
	pinMode(NIGHT_TIME_SWITCH_PIN, INPUT);
	Serial.begin(9600);
	Serial.println();
	SetState(ELEV_STATE::CALIBRATION_STARTED);
}

bool nightTimeEnabled()
{
	return digitalRead(NIGHT_TIME_SWITCH_PIN) == HIGH;
}

void loop() {
	nowMillis = millis();

	static unsigned long elev_timer = 0;
	const unsigned long ELEVATOR_RUN_INTERVAL = 720000; // 12 min
	//const unsigned long ELEVATOR_RUN_INTERVAL = 10000; // 12 min
	HandleCalibBtn();

	//Serial.println((int)curState);

	switch (curState)
	{
	case ELEV_STATE::SLEEPING:
		elev_timer += nowMillis - prevMillis;

		// Make sure that if night time is disabled that it's not currently night
		if (elev_timer >= ELEVATOR_RUN_INTERVAL && (nightTimeEnabled() || !nightTime()))
		{
			setLED(LOW);
			SetState(ELEV_STATE::RUNNING);
		}
		else
		{
			/*Serial.print("Night sensor: ");
			Serial.println(analogRead(NIGHT_SENSOR_PIN));

			Serial.print("Night mode enabled: ");
			Serial.println(nightTimeEnabled());
			Serial.println("goign to sleep");
			Serial.flush();*/
			LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
			elev_timer += 8000; // account for clock not running while power down;
		}
		break;
	case ELEV_STATE::IDLE:
		if (CurCalibBtnAction == BTN_ACTION::DOWN && totalElevSteps != 0)
		{
			SetState(ELEV_STATE::RUNNING);
		}
		break;
	case ELEV_STATE::CALIBRATION_STARTED:
		// Set LED high
		setLED(HIGH);

		// User lowers the lift to the ground and creates tension in the wire
		// User presses the calibration button
		if (CurCalibBtnAction == BTN_ACTION::DOWN)
		{
			totalElevSteps = curStep = 0;
			setLED(LOW);
			SetState(ELEV_STATE::CALIBRATION_IN_PROGRESS);
		}
		break;
	case ELEV_STATE::CALIBRATION_IN_PROGRESS:
		blinkLED();

		// Start moving up and count steps
		if (tryMove(false))
		{
			totalElevSteps++;
		}

		// User presses calibration button
		if (CurCalibBtnAction == BTN_ACTION::DOWN)
		{
			setDirection(true);
			setLED(LOW);
			SetState(ELEV_STATE::RUNNING);
		}
		break;
	case ELEV_STATE::RUNNING:
		if (!nightTimeEnabled() && nightTime())
		{
			blinkLED(LED_BLINK_SPEED * 2); // blink slower to indicate last night ride
		}

		if (tryMove(moveDown))
		{
			curStep--;
		}

		if (curStep == 0)
		{
			setDirection(!moveDown);
			clearMotorPorts();

			// Prevent overflow, reset timer
			elev_timer = 0;
			SetState(ELEV_STATE::SLEEPING);
			setLED(LOW);
		}
		else if (CurCalibBtnAction == BTN_ACTION::DOWN)
		{
			setLED(LOW);
			SetState(ELEV_STATE::IDLE);
			clearMotorPorts();
		}

		break;
	}
	//turn90();
	//PORTD = PORTD & SERIALMASK;   
	//delay(30);

	prevMillis = nowMillis;
}
