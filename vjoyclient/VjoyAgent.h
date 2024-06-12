#pragma once
#include <string>
#include <functional>
#include <QVariantMap>
#include "stdafx.h"
#include "public.h"
#include "vjoyinterface.h"
namespace VJoy{


class CallBackTest
{
public:
	CallBackTest(std::function<void(void*, void*)> func) {
		m_func = func;
	};
	~CallBackTest() {};

	static void CALLBACK ffbCallBack(void* data, void* userdata) {
		m_func(data, userdata);
	};
private:
	static std::function<void(void*, void*)> m_func;
};

struct CtrlValue
{
	double Throttle=0;
	double Rudder=0;
	double Aileron=0;
	double Elevator=0;
};

class VjoyAgent
{
public:
	VjoyAgent();
	~VjoyAgent();
	bool init(unsigned int did);
	bool startffb();
	void terminate();
	long mapValue(int inputValue);
	bool setCtrlValue(const CtrlValue&);
private:

	void FfbFunction(void* data);

	void FfbFunction1(void* data, void* userdata);

	bool PacketType2Str(FFBPType Type, std::string& Str);
	bool EffectType2Str(FFBEType Type, std::string& Str);
	bool DevCtrl2Str(FFB_CTRL Ctrl, std::string& Str);
	bool EffectOpStr(FFBOP Op, std::string& Str);
	int Polar2Deg(unsigned char Polar);
	int Byte2Percent(unsigned char InByte);
	int TwosCompByte2Int(unsigned char in);

private:
	unsigned int m_deviceId;
	CallBackTest m_cbt;

	JOYSTICK_POSITION_V2 m_iReport; // The structure that holds the full position data
};

}

