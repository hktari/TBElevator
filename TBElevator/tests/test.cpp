#include "..\ArduinoProxy.h"
#include "..\TBElevator.h"
#include <iostream>

int main()
{
	initialize_mock_arduino();
	TBElevator elevator = TBElevator();
	elevator.SetState(ELEV_STATE::CALIBRATION_STARTED);
	std::cout << "finished tests" << std::endl;
}