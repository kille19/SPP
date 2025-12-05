#include <iostream>
#include "XsensReader.h"
#include <windows.h>

int main() {
	std::cout << "Starting Xsens IMU Reader" << std::endl;
	
	XsensReader reader;

	if (!reader.initialize()) {
		std::cout << "ERROR: Failed to initialize XsensReader." << std::endl;
		return -1;
	}

	if (!reader.startMeasurement()) {
		std::cout << "ERROR: Failed to start measurement." << std::endl;
		return -1;
	}

	std::cout << "Reading from IMU... (Exit with CTRL+C)" << std::endl;

	while (true) {
		reader.readPacket();
		Sleep(1);
	}
}