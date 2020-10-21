#include "..\ArduinoProxy.h"
#include "..\TBElevator.h"
#include "lib/catch.hpp"

// ********** HELPER METHODS *************
//
unsigned long time_step = 0;
unsigned long time_interval;
void reset_time(unsigned long set_time_interval)
{
	time_interval = set_time_interval;
	time_step = 0;
}

unsigned long advance_time()
{
	return time_interval * time_step++;
}
void calibrate_elev(TBElevator& elev, int steps)
{
	elev.Tick(advance_time(), BTN_ACTION::LONG_PRESS);
	elev.Tick(advance_time(), BTN_ACTION::DOWN);
	for(int i = 0; i < steps; i++)
	{
		elev.Tick(advance_time(), BTN_ACTION::NONE);
	}
	elev.Tick(advance_time(), BTN_ACTION::DOWN);
}
// **************************************

TEST_CASE("Elevator state initializes with Idle")
{
	initialize_mock_arduino();
	TBElevator elev = TBElevator();
	REQUIRE(elev.GetState() == ELEV_STATE::IDLE);
}

TEST_CASE("Elevator calibration")
{
	initialize_mock_arduino();
	TBElevator elev = TBElevator();
	reset_time(elev.MOTOR_STEP_INTERVAL);

	elev.Tick(advance_time(), BTN_ACTION::LONG_PRESS);
	REQUIRE(elev.GetState() == ELEV_STATE::CALIBRATION_STARTED);

	elev.Tick(advance_time(), BTN_ACTION::DOWN);
	REQUIRE(elev.GetState() == ELEV_STATE::CALIBRATION_IN_PROGRESS);

	elev.Tick(advance_time(), BTN_ACTION::NONE);
	elev.Tick(advance_time(), BTN_ACTION::DOWN);
	REQUIRE(elev.GetState() == ELEV_STATE::RUNNING);
	REQUIRE(elev.IsMovingDown());
	REQUIRE(elev.GetSteps() == 1);	
}

TEST_CASE("Elevator waits for passengers after reaching bottom")
{
}

TEST_CASE("Elevator goes back up after reaching bottom")
{
	initialize_mock_arduino();
	TBElevator elev;
	reset_time(elev.MOTOR_STEP_INTERVAL);
	calibrate_elev(elev, 2);

	elev.Tick(advance_time(), BTN_ACTION::NONE);
	elev.Tick(advance_time(), BTN_ACTION::NONE);
	REQUIRE(elev.IsMovingUp());
}

TEST_CASE("Elevator goes back down after reaching top")
{

}

TEST_CASE("Elevator stays in idle when button is pressed and calibration hasn't occured yet")
{

}

TEST_CASE("Elevator goes into idle when button is pressed in running")
{

}

TEST_CASE("Elevator goes into running when button is pressed in idle and calibrated")
{

}

