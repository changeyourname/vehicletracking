#include "nixieFsm.hpp"
#include "pcf8583.hpp"

uint8_t BCDToDecimal (uint8_t bcdByte)
{
  return (((bcdByte & 0xF0) >> 4) * 10) + (bcdByte & 0x0F);
}
 
uint8_t DecimalToBCD (uint8_t decimalByte)
{
  return (((decimalByte / 10) << 4) | (decimalByte % 10));
}

//--I2C Master-----------------------------------------------------------------
uint8_t hoursToSet;
uint8_t minutesToSet;
uint8_t secondsToSet;

typedef  xpcc::SoftwareI2cMaster< SCLPin, SDAPin, xpcc::I2cMaster::Baudrate::Standard > sI2CMaster;
xpcc::PCF8583<sI2CMaster> pcf8583;

uint8_t rtcData[10] = {0, 0, 0, 0, 12, 0, 0, 0};

class I2CMaster
{
public:
	void run(void)
	{
         pcf8583.read(0x00, rtcData, 8);
	}

private:
	xpcc::ShortTimeout timeout;
};

I2CMaster i2CMaster;

void getTimeFromRTC(void)
{
	static bool rtcSynchronized = false;

	if(rtcSynchronized == false)
	{
		if((nixieClock.minute % 10) == 0x00)
		{
			i2CMaster.run();
			nixieClock.hour   = (((rtcData[3]) >> 4) * 10) + (rtcData[3] & 0x0F);
			nixieClock.minute = (((rtcData[2]) >> 4) * 10) + (rtcData[2] & 0x0F);
			nixieClock.second = (((rtcData[1]) >> 4) * 10) + (rtcData[1] & 0x0F);
			rtcSynchronized   = true;
		}
	}

	if(nixieClock.minute != 0x00)
	{
		rtcSynchronized = false;
	}
}
//--State Startup--------------------------------------------------------------
void /*Nixie::Fsm::*/stateStartup_Enter(void)
{
	nixieClock.greenLed.On();
	xpcc::delayMilliseconds(1000);
	nixieClock.greenLed.Off();

	ButtonInput::setInput(Gpio::InputType::PullUp);	
}

void /*Nixie::Fsm::*/stateStartup_Update(void)
{
	SCLPin::setOutput();
	SDAPin::setOutput();

	DcfReceiver::Init();

	nixieClock.display.RefreshCathodes_Init();	
	while(nixieClock.display.RefreshCathodes()) {};

	stateMachine.transitionTo(stateDisplayTime);
}

void /*Nixie::Fsm::*/stateStartup_Exit(void)
{
	nixieClock.hour   = 0x00;
	nixieClock.minute = 0x00;
	nixieClock.second = 0x00;

	getTimeFromRTC();
}

State /*Nixie::Fsm::*/stateStartup = State(stateStartup_Enter, stateStartup_Update, stateStartup_Exit);

//--State DisplayTime----------------------------------------------------------
void stateDisplayTime_EventsProcess(void)
{
	if(nixieClock.button.ButtonGetState() == Nixie::eButtonStateLongPressed)
	{
		stateMachine.transitionTo(statePreSetTime);
	}

	if(nixieClock.iRrmntSmallChina00.keyCode == IRrmntSmallChina00::KeyCode_OnOff)
	{
		stateMachine.transitionTo(stateSetTimeIR);
	}

	if(nixieClock.iRrmntSmallChina00.keyCode == IRrmntSmallChina00::KeyCode_Source)
	{
		nixieClock.display.RefreshCathodes_Init();
		while(nixieClock.display.RefreshCathodes())
		{;}
	}

	if(nixieClock.iRrmntSmallChina00.keyCode == IRrmntSmallChina00::KeyCode_Record)
	{
		stateMachine.transitionTo(stateDisplayDcf);
	}
}

#define blinkRule_d true

bool dcfUnsynchr    		= true;
bool dcfUnsynchrOld 		= false;
bool dcfUnsynchrBlinking 	= false;

void stateDisplayTime_Enter(void)
{
	if(dcfUnsynchrBlinking)
	{
		nixieClock.display.Blink(blinkRule_d);
	}	
}

void stateDisplayTime_Update(void)
{
	getTimeFromRTC();

	nixieClock.display.DisplayTime(nixieClock.hour, nixieClock.minute, nixieClock.second);

	//--blink when no synchro with DCF
	dcfUnsynchr = (xpcc::Clock::now() - nixieClock.timestampTimeUpdated > unsynchronizedDCFTimeout_d);

	if((dcfUnsynchr == true) && (dcfUnsynchrOld == false))
	{
		nixieClock.display.Blink(blinkRule_d);
		dcfUnsynchrBlinking = true;
	}

	if((dcfUnsynchr == false) && (dcfUnsynchrOld == true))
	{
		nixieClock.display.Blink((uint16_t)0);
		dcfUnsynchrBlinking = false;		
	}

	dcfUnsynchrOld = dcfUnsynchr;
	//--!blink when no synchro with DCF

	stateDisplayTime_EventsProcess();

	if(!DcfReceiver::DcfSignallDetected == true)
	{
		stateMachine.transitionTo(stateDisplayUnschrTime);
	}	
}

void stateDisplayTime_Exit(void)
{
	hoursToSet   = (((rtcData[3]) >> 4) * 10) + (rtcData[3] & 0x0F);
	minutesToSet = (((rtcData[2]) >> 4) * 10) + (rtcData[2] & 0x0F);
	secondsToSet = (((rtcData[1]) >> 4) * 10) + (rtcData[1] & 0x0F);
}

State stateDisplayTime = State(stateDisplayTime_Enter, stateDisplayTime_Update, stateDisplayTime_Exit);

//--State stateDisplayUnschrTime-----------------------------------------------
void stateDisplayUnschrTime_Enter(void)
{
	nixieClock.display.Blink((uint16_t)0);
}

void stateDisplayUnschrTime_Update(void)
{
	getTimeFromRTC();

	nixieClock.display.DisplayData(nixieClock.hour, nixieClock.minute, nixieClock.second);

	stateDisplayTime_EventsProcess();

	if(DcfReceiver::DcfSignallDetected == true)
	{
		stateMachine.transitionTo(stateDisplayTime);
	}		
}

void stateDisplayUnschrTime_Exit(void)
{
	hoursToSet   = (((rtcData[3]) >> 4) * 10) + (rtcData[3] & 0x0F);
	minutesToSet = (((rtcData[2]) >> 4) * 10) + (rtcData[2] & 0x0F);
	secondsToSet = (((rtcData[1]) >> 4) * 10) + (rtcData[1] & 0x0F);
}

State stateDisplayUnschrTime = State(stateDisplayUnschrTime_Enter, stateDisplayUnschrTime_Update, stateDisplayUnschrTime_Exit);

//--State DisplayDcf----------------------------------------------------------
xpcc::ShortTimeout timeoutForStartBit;

void stateDisplayDcf_Enter(void)
{
	// nixieClock.display.DisplayDataBCD(1, 1, 1);
	// timeoutForStartBit.restart(1000);

	// while(!timeoutForStartBit.isExpired()) {};
}

void stateDisplayDcf_Update(void)
{
	if(nixieClock.iRrmntSmallChina00.keyCode == IRrmntSmallChina00::KeyCode_TimeShift)
	{
		stateMachine.transitionTo(stateDisplayTime);
	}

	if(DcfReceiver::StartBit == 1)
	{
		nixieClock.display.DisplayDataBCD(1, 1, 1);
		timeoutForStartBit.restart(5000);
	}
	else
	{
		if(timeoutForStartBit.isExpired())
		{
			nixieClock.display.DisplayData(DcfTime::getHours(), DcfTime::getMinutes(), DcfTime::getSeconds());
		}
	}

	if(DcfReceiver::SensorValue == BIT_0)
	{
		nixieClock.display.DotsSet();
	}
	else
	{
		nixieClock.display.DotsReset();
	}
}

void stateDisplayDcf_Exit(void)
{

}

State stateDisplayDcf = State(stateDisplayDcf_Enter, stateDisplayDcf_Update, stateDisplayDcf_Exit);

//--State SetTime--------------------------------------------------------------
uint8_t digitToSetupIndex;

void statePreSetTime_Enter(void)
{
	nixieClock.SetTime_Enter();
}

void statePreSetTime_Update(void)
{
	if(nixieClock.button.ButtonGetState() == Nixie::eButtonStateReleased)
	{
		stateMachine.transitionTo(stateSetTime);
	}	
}

void statePreSetTime_Exit(void)
{
	digitToSetupIndex = 0x00;
}
State statePreSetTime = State(statePreSetTime_Enter, statePreSetTime_Update, statePreSetTime_Exit);


void statePreSetDigit_Enter(void)
{
}
void statePreSetDigit_Update(void)
{
	if(nixieClock.button.ButtonGetState() == Nixie::eButtonStateReleased)
	{
		stateMachine.transitionTo(stateSetTime);
	}	
}
void statePreSetDigit_Exit(void)
{
}
State statePreSetDigit = State(statePreSetDigit_Enter, statePreSetDigit_Update, statePreSetDigit_Exit);

void /*Nixie::Fsm::*/stateSetTime_Enter(void)
{
	nixieClock.display.SetDisplayIntoNormal();
	switch(digitToSetupIndex)
	{
		case 0:
			nixieClock.display.SetDigitIntoSetup(0x00);
			nixieClock.display.SetDigitIntoSetup(0x01);
			break;
		case 1:
			nixieClock.display.SetDigitIntoSetup(0x02);
			nixieClock.display.SetDigitIntoSetup(0x03);
			break;
		case 2:
			nixieClock.display.SetDigitIntoSetup(0x04);
			nixieClock.display.SetDigitIntoSetup(0x05);
			break;
		case 3:
			stateMachine.immediateTransitionTo(stateStartup);				
			break;
		default:
			break;
	}
}

void /*Nixie::Fsm::*/stateSetTime_Update(void)
{
	Nixie::ButtonState buttonState = nixieClock.button.ButtonGetState();

	if(buttonState == Nixie::eButtonStateLongPressed)
	{
		digitToSetupIndex = ((digitToSetupIndex++ >= 3) ? 0 : digitToSetupIndex);
		if(digitToSetupIndex == 3)
		{
			stateMachine.immediateTransitionTo(stateStartup);
		}
		else
		{
			stateMachine.immediateTransitionTo(statePreSetDigit);
		}
	}
	else if(buttonState == Nixie::eButtonStateReleased)
	{
		switch(digitToSetupIndex)
		{
			case 0:
				hoursToSet = ((++hoursToSet > 23) ? 0 : hoursToSet);
				break;
			case 1:
				minutesToSet = ((++minutesToSet > 59) ? 0 : minutesToSet);
				break;
			case 2:
				secondsToSet = ((++secondsToSet > 59) ? 0 : secondsToSet);
				break;
			default:
				break;
		}
	}
	nixieClock.display.DisplayData(hoursToSet, minutesToSet, secondsToSet);
}

void /*Nixie::Fsm::*/stateSetTime_Exit(void)
{
   nixieClock.SetTime_Exit();

   rtcData[0] = 0x00;
   rtcData[1] = (((secondsToSet / 10) << 4) | (secondsToSet % 10));
   rtcData[2] = (((minutesToSet / 10) << 4) | (minutesToSet % 10));
   rtcData[3] = (((hoursToSet   / 10) << 4) | (hoursToSet   % 10));
   pcf8583.write(0x03, rtcData, 4);
}

//--State SetTimeIR------------------------------------------------------------
uint8_t digitToSet;

void stateSetTimeIR_Enter(void)
{
	nixieClock.display.SetDisplayIntoNormal();
	digitToSet = 0;
	nixieClock.display.SetDigitIntoSetup(digitToSet);
}

void stateSetTimeIR_Update(void)
{	
	IRrmntSmallChina00::KeyCodes keyCode = nixieClock.iRrmntSmallChina00.GetKeyCode();

	if(keyCode == IRrmntSmallChina00::KeyCode_SpeakerOff)
	{
		nixieClock.second = secondsToSet;
   		nixieClock.minute = minutesToSet;
   		nixieClock.hour   = hoursToSet;

		rtcData[0] = 0x00;
		rtcData[1] = (((secondsToSet / 10) << 4) | (secondsToSet % 10));
		rtcData[2] = (((minutesToSet / 10) << 4) | (minutesToSet % 10));
		rtcData[3] = (((hoursToSet   / 10) << 4) | (hoursToSet   % 10));
		pcf8583.write(0x03, rtcData, 4);

		stateMachine.immediateTransitionTo(stateDisplayTime);
	}
	else if(keyCode == IRrmntSmallChina00::KeyCode_VolMinus)
	{
		nixieClock.display.SetDisplayIntoNormal();
		digitToSet = ((digitToSet == 0) ? 5 : (digitToSet - 1));
		nixieClock.display.SetDigitIntoSetup(digitToSet);
	}
	else if(keyCode == IRrmntSmallChina00::KeyCode_VolPlus)
	{
		nixieClock.display.SetDisplayIntoNormal();
		digitToSet = ((digitToSet == 5) ? 0 : (digitToSet + 1));
		nixieClock.display.SetDigitIntoSetup(digitToSet);
	}
	else if(keyCode == IRrmntSmallChina00::KeyCode_CHPlus)
	{
		switch(digitToSet)
		{
			case 0:
				hoursToSet = ((hoursToSet >= 20) ? (hoursToSet % 10) : (hoursToSet + 10));
				break;
			case 1:
				hoursToSet = ((hoursToSet == 23) ? 0 : (hoursToSet + 1));
				break;
			case 2:
				minutesToSet = ((minutesToSet >= 50) ? (minutesToSet % 10) : (minutesToSet + 10));
				break;			
			case 3:
				minutesToSet = ((minutesToSet == 59) ? 0 : (minutesToSet + 1));
				break;
			case 4:
				secondsToSet = ((secondsToSet >= 50) ? (secondsToSet % 10) : (secondsToSet + 10));
				break;
			case 5:
				secondsToSet = ((secondsToSet == 59) ? 0 : (secondsToSet + 1));
				break;
			default:
				break;
		}
	}
	else if(keyCode == IRrmntSmallChina00::KeyCode_CHMinus)
	{
		switch(digitToSet)
		{
			case 0:
				hoursToSet = ((hoursToSet <= 10) ? 23 : (hoursToSet - 10));
				break;
			case 1:
				hoursToSet = ((hoursToSet == 0) ? 23 : (hoursToSet - 1));
				break;
			case 2:
				minutesToSet = ((minutesToSet <= 10) ? 59 : (minutesToSet - 1));
				break;
			case 3:
				minutesToSet = ((minutesToSet == 0) ? 59 : (minutesToSet - 1));
				break;
			case 4:
				secondsToSet = ((secondsToSet <= 10) ? 59 : (secondsToSet - 1));
				break;
			case 5:
				secondsToSet = ((secondsToSet == 0) ? 59 : (secondsToSet - 1));
				break;
			default:
				break;
		}
	}
	else if(   keyCode == IRrmntSmallChina00::KeyCode_Button0
			|| keyCode == IRrmntSmallChina00::KeyCode_Button1
			|| keyCode == IRrmntSmallChina00::KeyCode_Button2
			|| keyCode == IRrmntSmallChina00::KeyCode_Button3
			|| keyCode == IRrmntSmallChina00::KeyCode_Button4
			|| keyCode == IRrmntSmallChina00::KeyCode_Button5
			|| keyCode == IRrmntSmallChina00::KeyCode_Button6
			|| keyCode == IRrmntSmallChina00::KeyCode_Button7
			|| keyCode == IRrmntSmallChina00::KeyCode_Button8
			|| keyCode == IRrmntSmallChina00::KeyCode_Button9
			)
	{
		uint8_t buttonValue;

		switch(keyCode)
		{
			case IRrmntSmallChina00::KeyCode_Button0:
				buttonValue = 0;
				break;
			case IRrmntSmallChina00::KeyCode_Button1:
				buttonValue = 1;
				break;
			case IRrmntSmallChina00::KeyCode_Button2:
				buttonValue = 2;
				break;
			case IRrmntSmallChina00::KeyCode_Button3:
				buttonValue = 3;
				break;
			case IRrmntSmallChina00::KeyCode_Button4:
				buttonValue = 4;
				break;
			case IRrmntSmallChina00::KeyCode_Button5:
				buttonValue = 5;
				break;
			case IRrmntSmallChina00::KeyCode_Button6:
				buttonValue = 6;
				break;
			case IRrmntSmallChina00::KeyCode_Button7:
				buttonValue = 7;
				break;
			case IRrmntSmallChina00::KeyCode_Button8:
				buttonValue = 8;
				break;
			case IRrmntSmallChina00::KeyCode_Button9:
				buttonValue = 9;
				break;
			default:
				break;
		}

		switch(digitToSet)
		{
			case 0:
				if(buttonValue <= 2)
				{
					hoursToSet = ((buttonValue * 10) + (hoursToSet % 10));
				}
				break;
			case 1:
				if(hoursToSet >= 20)
				{
					if(buttonValue <= 3)
					{
						hoursToSet = (((hoursToSet / 10) * 10) + buttonValue);
					}
				}
				else if(hoursToSet < 20)
				{
					hoursToSet = (((hoursToSet / 10) * 10) + buttonValue);
				}
				break;
			case 2:
				if(buttonValue <= 5)
				{			
					minutesToSet = ((buttonValue * 10) + (minutesToSet % 10));
				}
				break;
			case 3:
				minutesToSet = (((minutesToSet / 10) * 10) + buttonValue);
				break;
			case 4:
				if(buttonValue <= 5)
				{			
					secondsToSet = ((buttonValue * 10) + (secondsToSet % 10));
				}
				break;
			case 5:
				secondsToSet = (((secondsToSet / 10) * 10) + buttonValue);
				break;
			default:
				break;
		}
	}
	nixieClock.display.DisplayData(hoursToSet, minutesToSet, secondsToSet);	
}

void stateSetTimeIR_Exit(void)
{
	nixieClock.SetTime_Exit();
}
State stateSetTimeIR = State(stateSetTimeIR_Enter, stateSetTimeIR_Update, stateSetTimeIR_Exit);


State /*Nixie::Fsm::*/stateSetTime = State(stateSetTime_Enter, stateSetTime_Update, stateSetTime_Exit);

//--Common---------------------------------------------------------------------
void stateMachine_Process(void)
{
	stateMachine.update();

	DcfReceiver::Process();
	if(DcfReceiver::StartBit == BIT_1)
	{
		if(	 ((DcfTime::getMinutes() != 0xFF) && (DcfTime::getHours() != 0xFF))
		  && ((DcfTime::getMinutes() != 0x00) && (DcfTime::getHours() != 0x00))
		  )
		{
			rtcData[0] = 0x00;
			rtcData[1] = (((0 / 10) << 4) | (0 % 10));
			rtcData[2] = (((DcfTime::getMinutes() / 10) << 4) | (DcfTime::getMinutes() % 10));
			rtcData[3] = (((DcfTime::getHours()   / 10) << 4) | (DcfTime::getHours()   % 10));
			pcf8583.write(0x03, rtcData, 4);

			nixieClock.hour   = DcfTime::getHours();
			nixieClock.minute = DcfTime::getMinutes();
			nixieClock.second = 0;
			nixieClock.timestampTimeUpdated = xpcc::Clock::now();
		}
	}
}

//--Fsm------------------------------------------------------------------------
FSM /*Nixie::Fsm::*/stateMachine = FSM(/*Nixie::Fsm::*/stateStartup);	
