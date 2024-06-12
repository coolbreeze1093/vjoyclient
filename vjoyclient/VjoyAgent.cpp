#include "VjoyAgent.h"
#include <iostream>
using namespace VJoy;
std::function<void(void*, void*)> CallBackTest::m_func;

VjoyAgent::VjoyAgent() :m_cbt(std::bind(&VjoyAgent::FfbFunction1, this, std::placeholders::_1, std::placeholders::_2))
{
	
}

VjoyAgent::~VjoyAgent()
{
}

bool VjoyAgent::init(unsigned int did)
{
	m_deviceId = did;

	// Define the effect names
	//static FFBEType FfbEffect = (FFBEType)-1;
	//std::string FfbEffectName[] = { "NONE", "Constant Force", "Ramp", "Square", "Sine", "Triangle", "Sawtooth Up",\
	"Sawtooth Down", "Spring", "Damper", "Inertia", "Friction", "Custom Force" };
	;

	// Get the driver attributes (Vendor ID, Product ID, Version Number)
	if (!vJoyEnabled())
	{
		std::cerr << "Function vJoyEnabled Failed - make sure that vJoy is installed and enabled\n";
		return false;
	}
	else
	{
		std::wcout << L"Vendor: " << static_cast<TCHAR*> (GetvJoyManufacturerString())
			<< L"\nProduct: "<< static_cast<TCHAR*>(GetvJoyProductString()) 
			<< "\nVersion Number: " << static_cast<TCHAR*>(GetvJoySerialNumberString()) <<"\n";
	};

	// Get the status of the vJoy device before trying to acquire it
	VjdStat status = GetVJDStatus(m_deviceId);

	switch (status)
	{
	case VJD_STAT_OWN:
		std::cout << "vJoy device "<< m_deviceId <<" is already owned by this feeder\n";
		break;
	case VJD_STAT_FREE:
		std::cout << "vJoy device "<< m_deviceId <<" is free\n";
		break;
	case VJD_STAT_BUSY:
		std::cout << "vJoy device " << m_deviceId << " is already owned by another feeder\nCannot continue\n";
		return false;
	case VJD_STAT_MISS:
		std::cout << "vJoy device "<< m_deviceId <<" is not installed or disabled\nCannot continue\n";
		return false;
	default:
		std::cout << "vJoy device "<< m_deviceId <<" % d general error\nCannot continue\n";
		return false;
	};

	// Acquire the vJoy device
	if (!AcquireVJD(m_deviceId))
	{
		std::cout << "Failed to acquire vJoy device number "<< m_deviceId<<"\n";
		terminate();
		return false;
	}
	else
	{
		std::cout << "Acquired device number "<< m_deviceId <<" - OK\n";
	}
	
	
	return true;
}

bool VjoyAgent::startffb()
{
	// Start FFB
	bool Ffbstarted = FfbStart(m_deviceId);
	if (!Ffbstarted)
	{
		std::cout << "Failed to start FFB on vJoy device number "<<m_deviceId<<".\n";
		terminate();
		return false;
	}
	else
	{
		std::cout << "Started FFB on vJoy device number " << m_deviceId << " - OK\n";
	}

	// Register Generic callback function
	// At this point you instruct the Receptor which callback function to call with every FFB packet it receives
	// It is the role of the designer to register the right FFB callback function
	FfbRegisterGenCB(CallBackTest::ffbCallBack, NULL);

	return true;
}

void VjoyAgent::terminate()
{
	RelinquishVJD(m_deviceId);
}

long VjoyAgent::mapValue(int inputValue) {
	int minInput = 0;
	int maxInput = 180;
	LONG minOutput = 0;
	LONG maxOutput = 32767;

	LONG mappedValue = minOutput + ((inputValue - minInput) * (maxOutput - minOutput)) / (maxInput - minInput);
	return mappedValue;
}

bool VjoyAgent::setCtrlValue(const CtrlValue&value)
{
	// Start endless loop
	// The loop injects position data to the vJoy device
	// If it fails it let's the user try again
	//
	// FFB Note:
	// All FFB activity is performed in a separate thread created when registered the callback function   

	// Set destenition vJoy device
	BYTE id = (BYTE)m_deviceId;
	m_iReport.bDevice = id;

	// Set position data of 3 first axes
	m_iReport.wAxisX = mapValue(value.Throttle);
	m_iReport.wAxisY = mapValue(value.Rudder);
	m_iReport.wAxisZ = mapValue(value.Aileron);
	m_iReport.wAxisXRot = mapValue(value.Elevator);

	// Set position data of first 8 buttons
	//Btns = 1 << (Z / 4000);
	//m_iReport.lButtons = Btns;

	// Send position data to vJoy device
	void* pPositionMessage = (PVOID)(&m_iReport);
	if (!UpdateVJD(m_deviceId, pPositionMessage))
	{
		std::cout << "Feeding vJoy device number " << m_deviceId << " failed - try to enable device then press enter\n";
		AcquireVJD(m_deviceId);
		terminate();
		init(m_deviceId);
		return false;
	}


	return true;
}



// Generic callback function
void VjoyAgent::FfbFunction(void* data)
{
	FFB_DATA* FfbData = (FFB_DATA*)data;
	int size = FfbData->size;
	std::cout << "\nFFB Size "<< size<<"\n";

	std::cout << "Cmd:"<< FfbData->cmd <<"08.8X ";
	std::cout << "ID:"<< FfbData->data[0] <<"02.2X ";
	std::cout << "Size:"<< static_cast<int>(FfbData->size - 8) <<"02.2d ";
	std::cout << " - ";
	for (UINT i = 0; i < FfbData->size - 8; i++)
		std::cout << (UINT)FfbData->data<<"  02.2X";
	std::cout << ("\n");
}

void VjoyAgent::FfbFunction1(void* data, void* userdata)
{
	// Packet Header
	std::cout << "\n ============= FFB Packet size Size " << static_cast<int>(((FFB_DATA*)data)->size)<<" =============\n";

	/////// Packet Device ID, and Type Block Index (if exists)
#pragma region Packet Device ID, and Type Block Index
	int DeviceID, BlockIndex;
	FFBPType	Type;
	std::string	TypeStr;

	if (ERROR_SUCCESS == Ffb_h_DeviceID((FFB_DATA*)data, &DeviceID))
		std::cout << "\n > Device ID: "<< DeviceID;
	if (ERROR_SUCCESS == Ffb_h_Type((FFB_DATA*)data, &Type))
	{
		if (!PacketType2Str(Type, TypeStr))
			std::cout << "\n > Packet Type: " << Type;
		else
			std::cout << "\n > Packet Type: " << TypeStr;

	}
	if (ERROR_SUCCESS == Ffb_h_EBI((FFB_DATA*)data, &BlockIndex))
		std::cout << "\n > Effect Block Index: " << BlockIndex;
#pragma endregion


	/////// Effect Report
#pragma region Effect Report
	FFB_EFF_CONST Effect;
	if (ERROR_SUCCESS == Ffb_h_Eff_Report((FFB_DATA*)data, &Effect))
	{
		if (!EffectType2Str(Effect.EffectType, TypeStr))
			std::cout << "\n >> Effect Report: "<< Effect.EffectType<<"02x";
		else
			std::cout << "\n >> Effect Report: "<< TypeStr;

		if (Effect.Polar)
		{
			std::cout << "\n >> Direction: " << Polar2Deg(Effect.Direction) << " deg ("<< Effect.Direction <<"02x)";


		}
		else
		{
			std::cout << "\n >> X Direction: " << Effect.DirX <<"02x";
			std::cout << "\n >> Y Direction: "<< Effect.DirY <<"02x";
		};

		if (Effect.Duration == 0xFFFF)
			std::cout << "\n >> Duration: Infinit";
		else
			std::cout << "\n >> Duration: " << static_cast<int>(Effect.Duration)<<" MilliSec";

		if (Effect.TrigerRpt == 0xFFFF)
			std::cout << "\n >> Trigger Repeat: Infinit";
		else
			std::cout << "\n >> Trigger Repeat: "<< static_cast<int>(Effect.TrigerRpt);

		if (Effect.SamplePrd == 0xFFFF)
			std::cout << "\n >> Sample Period: Infinit";
		else
			std::cout << "\n >> Sample Period: "<< static_cast<int>(Effect.SamplePrd);


		std::cout << "\n >> Gain: %d%%", Byte2Percent(Effect.Gain);

	};
#pragma endregion
#pragma region PID Device Control
	FFB_CTRL	Control;
	std::string	CtrlStr;
	if (ERROR_SUCCESS == Ffb_h_DevCtrl((FFB_DATA*)data, &Control) && DevCtrl2Str(Control, CtrlStr))
		std::cout << "\n >> PID Device Control: "<< CtrlStr;

#pragma endregion
#pragma region Effect Operation
	FFB_EFF_OP	Operation;
	std::string	EffOpStr;
	if (ERROR_SUCCESS == Ffb_h_EffOp((FFB_DATA*)data, &Operation) && EffectOpStr(Operation.EffectOp, EffOpStr))
	{
		std::cout << "\n >> Effect Operation: "<< EffOpStr;
		if (Operation.LoopCount == 0xFF)
			std::cout << "\n >> Loop until stopped";
		else
			std::cout << "\n >> Loop "<< static_cast<int>(Operation.LoopCount)<<" times";

	};
#pragma endregion
#pragma region Global Device Gain
	BYTE Gain;
	if (ERROR_SUCCESS == Ffb_h_DevGain((FFB_DATA*)data, &Gain))
		std::cout << "\n >> Global Device Gain: "<< Byte2Percent(Gain);

#pragma endregion
#pragma region Condition
	FFB_EFF_COND Condition;
	if (ERROR_SUCCESS == Ffb_h_Eff_Cond((FFB_DATA*)data, &Condition))
	{
		if (Condition.isY)
			std::cout << "\n >> Y Axis";
		else
			std::cout << ("\n >> X Axis");
		std::cout << "\n >> Center Point Offset: "<< TwosCompByte2Int(Condition.CenterPointOffset) * 10000 / 127;
		std::cout << "\n >> Positive Coefficient: "<< TwosCompByte2Int(Condition.PosCoeff) * 10000 / 127;
		std::cout << "\n >> Negative Coefficient: "<< TwosCompByte2Int(Condition.NegCoeff) * 10000 / 127;
		std::cout << "\n >> Positive Saturation: "<< Condition.PosSatur * 10000 / 255;
		std::cout << "\n >> Negative Saturation: "<< Condition.NegSatur * 10000 / 255;
		std::cout << "\n >> Dead Band: "<< Condition.DeadBand * 10000 / 255;
	}
#pragma endregion
#pragma region Envelope
	FFB_EFF_ENVLP Envelope;
	if (ERROR_SUCCESS == Ffb_h_Eff_Envlp((FFB_DATA*)data, &Envelope))
	{
		std::cout << "\n >> Attack Level: " << Envelope.AttackLevel * 10000 / 255;
		std::cout << "\n >> Fade Level: "<< Envelope.FadeLevel * 10000 / 255;
		std::cout << "\n >> Attack Time: "<< static_cast<int>(Envelope.AttackTime);
		std::cout << "\n >> Fade Time: "<< static_cast<int>(Envelope.FadeTime);
	};

#pragma endregion
#pragma region Periodic
	FFB_EFF_PERIOD EffPrd;
	if (ERROR_SUCCESS == Ffb_h_Eff_Period((FFB_DATA*)data, &EffPrd))
	{
		std::cout << "\n >> Magnitude: "<< EffPrd.Magnitude * 10000 / 255;
		std::cout << "\n >> Offset: " << TwosCompByte2Int(EffPrd.Offset) * 10000 / 127;
		std::cout << "\n >> Phase: " << EffPrd.Phase * 3600 / 255;
		std::cout << "\n >> Period: " << static_cast<int>(EffPrd.Period);
	};
#pragma endregion

#pragma region Effect Type
	FFBEType EffectType;
	if (ERROR_SUCCESS == Ffb_h_EffNew((FFB_DATA*)data, &EffectType))
	{
		if (EffectType2Str(EffectType, TypeStr))
			std::cout << "\n >> Effect Type: "<< TypeStr;
		else
			std::cout << "\n >> Effect Type: Unknown";
	}

#pragma endregion

#pragma region Ramp Effect
	FFB_EFF_RAMP RampEffect;
	if (ERROR_SUCCESS == Ffb_h_Eff_Ramp((FFB_DATA*)data, &RampEffect))
	{
		std::cout << "\n >> Ramp Start: "<< TwosCompByte2Int(RampEffect.Start) * 10000 / 127;
		std::cout << "\n >> Ramp End: "<< TwosCompByte2Int(RampEffect.End) * 10000 / 127;
	};

#pragma endregion

	std::cout << "\n";
	FfbFunction(data);
	std::cout << "\n ====================================================\n";

}

// Convert Packet type to String
bool VjoyAgent::PacketType2Str(FFBPType Type, std::string& Str)
{
	bool stat = TRUE;

	switch (Type)
	{
	case PT_EFFREP:
		Str = "Effect Report";
		break;
	case PT_ENVREP:
		Str = "Envelope Report";
		break;
	case PT_CONDREP:
		Str = "Condition Report";
		break;
	case PT_PRIDREP:
		Str = "Periodic Report";
		break;
	case PT_CONSTREP:
		Str = "Constant Force Report";
		break;
	case PT_RAMPREP:
		Str = "Ramp Force Report";
		break;
	case PT_CSTMREP:
		Str = "Custom Force Data Report";
		break;
	case PT_SMPLREP:
		Str = "Download Force Sample";
		break;
	case PT_EFOPREP:
		Str = "Effect Operation Report";
		break;
	case PT_BLKFRREP:
		Str = "PID Block Free Report";
		break;
	case PT_CTRLREP:
		Str = "PID Device Contro";
		break;
	case PT_GAINREP:
		Str = "Device Gain Report";
		break;
	case PT_SETCREP:
		Str = "Set Custom Force Report";
		break;
	case PT_NEWEFREP:
		Str = "Create New Effect Report";
		break;
	case PT_BLKLDREP:
		Str = "Block Load Report";
		break;
	case PT_POOLREP:
		Str = "PID Pool Report";
		break;
	default:
		stat = FALSE;
		break;
	}

	return stat;
}

// Convert Effect type to String
bool VjoyAgent::EffectType2Str(FFBEType Type, std::string& Str)
{
	bool stat = TRUE;

	switch (Type)
	{
	case ET_NONE:
		stat = FALSE;
		break;
	case ET_CONST:
		Str = "Constant Force";
		break;
	case ET_RAMP:
		Str = "Ramp";
		break;
	case ET_SQR:
		Str = "Square";
		break;
	case ET_SINE:
		Str = "Sine";
		break;
	case ET_TRNGL:
		Str = "Triangle";
		break;
	case ET_STUP:
		Str = "Sawtooth Up";
		break;
	case ET_STDN:
		Str = "Sawtooth Down";
		break;
	case ET_SPRNG:
		Str = "Spring";
		break;
	case ET_DMPR:
		Str = "Damper";
		break;
	case ET_INRT:
		Str = "Inertia";
		break;
	case ET_FRCTN:
		Str = "Friction";
		break;
	case ET_CSTM:
		Str = "Custom Force";
		break;
	default:
		stat = FALSE;
		break;
	};

	return stat;
}

// Convert PID Device Control to String
bool VjoyAgent::DevCtrl2Str(FFB_CTRL Ctrl, std::string& Str)
{
	bool stat = TRUE;

	switch (Ctrl)
	{
	case CTRL_ENACT:
		Str = "Enable Actuators";
		break;
	case CTRL_DISACT:
		Str = "Disable Actuators";
		break;
	case CTRL_STOPALL:
		Str = "Stop All Effects";
		break;
	case CTRL_DEVRST:
		Str = "Device Reset";
		break;
	case CTRL_DEVPAUSE:
		Str = "Device Pause";
		break;
	case CTRL_DEVCONT:
		Str = "Device Continue";
		break;
	default:
		stat = FALSE;
		break;
	}

	return stat;
}

// Convert Effect operation to string
bool VjoyAgent::EffectOpStr(FFBOP Op, std::string& Str)
{
	bool stat = TRUE;

	switch (Op)
	{
	case EFF_START:
		Str = "Effect Start";
		break;
	case EFF_SOLO:
		Str = "Effect Solo Start";
		break;
	case EFF_STOP:
		Str = "Effect Stop";
		break;
	default:
		stat = FALSE;
		break;
	}
	return stat;
}

// Polar values (0x00-0xFF) to Degrees (0-360)
int VjoyAgent::Polar2Deg(unsigned char Polar)
{
	return ((UINT)Polar * 360) / 255;
}

// Convert range 0x00-0xFF to 0%-100%
int VjoyAgent::Byte2Percent(unsigned char InByte)
{
	return ((UINT)InByte * 100) / 255;
}

// Convert One-Byte 2's complement input to integer
int VjoyAgent::TwosCompByte2Int(unsigned char in)
{
	int tmp;
	BYTE inv = ~in;
	bool isNeg = in >> 7;
	if (isNeg)
	{
		tmp = (int)(inv);
		tmp = -1 * tmp;
		return tmp;
	}
	else
		return (int)in;
}
