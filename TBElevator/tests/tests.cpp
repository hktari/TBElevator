#include "..\ArduinoProxy.h"
#include "..\TBElevator.h"
#include "lib/catch.hpp"

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

	elev.Tick(30, BTN_ACTION::LONG_PRESS);
	REQUIRE(elev.GetState() == ELEV_STATE::CALIBRATION_STARTED);

	elev.Tick(30, BTN_ACTION::DOWN);
	REQUIRE(elev.GetState() == ELEV_STATE::CALIBRATION_IN_PROGRESS);

	elev.Tick(30, BTN_ACTION::DOWN);
	REQUIRE(elev.GetState() == ELEV_STATE::RUNNING);	
}
