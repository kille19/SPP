#include "XsensReader.h"
#include <iostream>

XsensReader::XsensReader() {}

XsensReader::~XsensReader() {
	shutdown();	// Clean-up
}

bool XsensReader::initialize() {

	std::cout << "Creating XsControl oject..." << std::endl;

	control = XsControl::construct();

	if (!control) {
		std::cout << "ERROR: Failed to construct XsControl object." << std::endl;
		return false;
	}

	std::cout << "Scanning for Xsens devices..." << std::endl;

	XsPortInfoArray ports = XsScanner::scanPorts(XBR_2000k, 100, false);

	XsPortInfo mtPort;
	for (auto const& portInfo : ports)
	{
		if (portInfo.deviceId().isMti() || portInfo.deviceId().isMtig() || portInfo.deviceId().isMti6X0())
		{
			mtPort = portInfo;
			break;
		}
	}

	if (mtPort.empty()) {
		std::cout << "ERROR: No Xsens MTi device found." << std::endl;
		return false;
	}

	std::cout << "Connecting to device on port: " << mtPort.portName().toStdString() << " @ baudrate " << mtPort.baudrate() << std::endl;

	if (!control->openPort(mtPort.portName().toStdString(), mtPort.baudrate())) {
		std::cout << "ERROR: Could not open port." << std::endl;
		return false;
	}

	device = control->device(mtPort.deviceId());

	if (!device) {
		std::cout << "ERROR: Could not connect to device." << std::endl;
		return false;
	}

	std::cout << "Switching to configuration mode..." << std::endl;

	if (!device->gotoConfig()) {
		std::cout << "ERROR: Could not switch to configuration mode." << std::endl;
		return false;
	}

	std::cout << "Configuring output settings for accelerometer and gyroscope at 100Hz..." << std::endl;

	XsOutputConfigurationArray config;
	config.push_back(XsOutputConfiguration(XDI_Acceleration, 100));
	config.push_back(XsOutputConfiguration(XDI_RateOfTurn, 100));

	if (!device->setOutputConfiguration(config)) {
		std::cout << "ERROR: Could not set output configuration." << std::endl;
		return false;
	}

	device->addCallbackHandler(&callback);

	std::cout << "Initialization successful." << std::endl;
	return true;
}

bool XsensReader::startMeasurement() {

	std::cout << "Switching to measurement mode..." << std::endl;

	if (!device->gotoMeasurement()) {
		std::cout << "ERROR: Could not switch to measurement mode." << std::endl;
		return false;
	}
	std::cout << "Measurement started." << std::endl;
	return true;
}

bool XsensReader::readPacket() {

	XsDataPacket packet;

	if (!callback.getNextPacket(packet)) {
		return false;
	}

	if (packet.containsCalibratedData()) {
		XsVector acc = packet.calibratedAcceleration();
		std::cout << "\r"
			<< "Acc X:" << acc[0] 
			<< ", Acc Y:" << acc[1] 
			<< ", Acc Z" << acc[2]
			<< std::endl;

		XsVector gyr = packet.calibratedGyroscopeData();
		std::cout << "\r"
			<< "| Gyr X:" << gyr[0] 
			<< ", Gyr Y" << gyr[1] 
			<< ", Gyr Z" << gyr[2]
			<< std::endl;
	}
	return true;
}

void XsensReader::shutdown() {
	if (control) {
		std::cout << "Closing device connection..." << std::endl;
		control->closePort(mtPort.portName().toStdString());
		control->destruct();
		control = nullptr;
	}
}