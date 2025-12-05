#pragma once	

#include <xscontroller.h>
#include <xscontroller/xsscanner.h>
#include <xscontroller/xsdeviceptr.h>
#include <xscontroller/xsdevice_public.h>
#include <xstypes.h>
#include <xstypes/xsdatapacket.h>
#include <xstypes/xsoutputconfigurationarray.h>
#include <xscommon/xsens_mutex.h>

#include <deque>

class MyCallback :public XsCallback
{
public:
	MyCallback() {}

	void onLiveDataAvailable(XsDevice*, const XsDataPacket* packet) override
	{
		xsens::Lock lock(&m_mutex);
		m_packetBuffer.push_back(*packet);
	}

	bool getNextPacket(XsDataPacket& packet)
	{
		xsens::Lock lock(&m_mutex);

		if (m_packetBuffer.empty())
			return false;

		packet = m_packetBuffer.front();
		XsDataPacket oldestPacket = m_packetBuffer.front();
		m_packetBuffer.pop_front();
		return true;
	}
private:
	xsens::Mutex m_mutex;
	std::deque<XsDataPacket> m_packetBuffer;
};

class XsensReader {
public:
	XsensReader();
	~XsensReader();

	bool initialize();
	bool startMeasurement();
	bool readPacket();
	void shutdown();

private:
	XsControl* control = nullptr;
	XsDevice* device = nullptr;
	XsPortInfo  mtPort;

	MyCallback callback;
};