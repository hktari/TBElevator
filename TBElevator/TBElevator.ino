enum class ELEV_STATE
{
    IDLE,
    CALIBRATION_STARTED,
    CALIBRATION_IN_PROGRESS,
    RUNNING,
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


const int CALIB_BTN_PIN = 12;
const int STATE_LED_PIN = 11;

long timeNow;
long timeInterval = 1200;
bool rotateDirection = true;
long savedTime = micros();
long ledSavedTime = millis();

long lastCalibBtnDown = 0;
bool longPressCondition = 0;
int prevCalibBtnState = LOW;
ELEV_STATE CurState = ELEV_STATE::IDLE;
BTN_ACTION CurCalibBtnAction = BTN_ACTION::NONE;

int elevSteps = 0;
int curStep = 0;
bool moveDown;
int ledState = LOW;

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
    CurState = state;
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
    else if (longPressCondition && (millis() - lastCalibBtnDown) > 3000)
    {
        CurCalibBtnAction = BTN_ACTION::LONG_PRESS;
        longPressCondition = false;
    }

    prevCalibBtnState = calibBtnState;
}

void setup() {
    // Set stepper pins to output
    DDRD = DDRD | STEPPERPORT;
    pinMode(CALIB_BTN_PIN, INPUT);
    pinMode(STATE_LED_PIN, OUTPUT);
    Serial.begin(9600);

    SetState(ELEV_STATE::IDLE);
}

void loop() {
    if (millis() - ledSavedTime > 3E4)
    {
        moveDown = !moveDown;
        ledSavedTime = millis();
        phaseIndex = 0;
    }
    tryMove(moveDown);

    return;
    HandleCalibBtn();

    if (CurCalibBtnAction == BTN_ACTION::LONG_PRESS)
    {
        SetState(ELEV_STATE::CALIBRATION_STARTED);
    }

    switch (CurState)
    {
    case ELEV_STATE::IDLE:
        if (CurCalibBtnAction == BTN_ACTION::DOWN && elevSteps != 0)
        {
            digitalWrite(STATE_LED_PIN, LOW);
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

        if (millis() - ledSavedTime > 30)
        {
            ledState = !ledState;
            ledSavedTime = millis();
        }

        digitalWrite(STATE_LED_PIN, ledState);
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
            SetState(ELEV_STATE::RUNNING);
        }
        break;
    case ELEV_STATE::RUNNING:
        digitalWrite(STATE_LED_PIN, LOW);

        if (tryMove(moveDown))
        {
            curStep--;
        }

        if (curStep == 0)
        {
            curStep = elevSteps;
            moveDown = !moveDown;
        }

        if (CurCalibBtnAction == BTN_ACTION::DOWN)
        {
            SetState(ELEV_STATE::IDLE);
        }
        break;
    }



    //turn90();
    //PORTD = PORTD & SERIALMASK;   
    //delay(30);
}