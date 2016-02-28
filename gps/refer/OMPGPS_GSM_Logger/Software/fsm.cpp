#include "mbed.h"
#include "fsm.h"
#include "Fsm/FiniteStateMachine.h"

#include "sdLogger.h"
#include "compass.h"
#include "moveDetector.h"
#include "barometer.h"

extern Compass compass;
extern Barometer baro;
extern MoveDetector moveDetector;

extern Serial pc;

#define _hartBeatSlowBlinkPeriod   		1*1000*1000
#define _hartBeatRegularBlinkPeriod   	1* 500*1000
#define _hartBeatFastBlinkPeriod   		1* 100*1000

//--State Startup--------------------------------------------------------------
void stateStartup_Enter(void);
void stateStartup_Update(void);
void stateStartup_Exit(void);
extern State stateStartup;

//--State Active---------------------------------------------------------------
void stateActive_Enter(void);
void stateActive_Update(void);
void stateActive_Exit(void);
extern State stateActive;

//--State Not Active-----------------------------------------------------------
void stateNotActive_Enter(void);
void stateNotActive_Update(void);
void stateNotActive_Exit(void);
extern State stateNotActive;

//--State Sleep----------------------------------------------------------------
void stateSleep_Enter(void);
void stateSleep_Update(void);
void stateSleep_Exit(void);
extern State stateSleep;

//--State Send SMS-------------------------------------------------------------
void stateSendSMS_Enter(void);
void stateSendSMS_Update(void);
void stateSendSMS_Exit(void);
extern State stateSendSMS;

//--Fsm------------------------------------------------------------------------
FSM stateMachine = FSM(stateStartup);

//-----------------------------------------------------------------------------
//Implementation
//-----------------------------------------------------------------------------

//--State Startup--------------------------------------------------------------
void stateStartup_Enter(void)
{
	pc.printf("\n\r");
	pc.printf("stateStartup_Enter\n\r");	
}

void stateStartup_Update(void)
{
	moveDetector.InitDetector();
	stateMachine.transitionTo(stateActive);
}

void stateStartup_Exit(void)
{
}
State stateStartup = State(stateStartup_Enter, stateStartup_Update, stateStartup_Exit);

//--State Active---------------------------------------------------------------
void stateActive_Enter(void)
{
	pc.printf("stateActive_Enter\n\r");
	sdLogger.log((char*)"stateActive_Enter");
//    hartBeatBlinker.attach_us(&hartBeatBlink, 1000000); 
}

void stateActive_Update(void)
{
	wait(1);
	pc.printf("stateActive_Update\n\r");
	//sdLogger.log((char*)"stateActive_Update");


	if(moveDetector.notActive.debounceSignalSate == DebounceSignalSate_On)
	{
		stateMachine.transitionTo(stateNotActive);
	}
}

void stateActive_Exit(void)
{
	pc.printf("stateActive_Exit\n\r");
	sdLogger.log((char*)"stateActive_Exit");

//	hartBeatBlinker.detach();	
}
State stateActive = State(stateActive_Enter, stateActive_Update, stateActive_Exit);

//--State Not Active-----------------------------------------------------------
void stateNotActive_Enter(void)
{
	pc.printf("stateNotActive_Enter\n\r");
	sdLogger.log((char*)"stateNotActive_Enter");
//    hartBeatBlinker.attach_us(&hartBeatBlink, 500000);	
}

void stateNotActive_Update(void)
{
	pc.printf("stateNotActive_Update\n\r");
	//sdLogger.log((char*)"stateNotActive_Update");

	//pc.printf("noMoveDetected: %d\n\r", moveDetector.noMoveDetected.debounceSignalSate);
	if(moveDetector.acive.debounceSignalSate == DebounceSignalSate_On)
	{
		stateMachine.transitionTo(stateActive);
	}
	else if(moveDetector.sleep.debounceSignalSate == DebounceSignalSate_On)
	{
		stateMachine.transitionTo(stateSleep);
	}
}

void stateNotActive_Exit(void)
{
	pc.printf("stateNotActive_Exit\n\r");
	sdLogger.log((char*)"stateNotActive_Exit");

//	hartBeatBlinker.detach();
}
State stateNotActive = State(stateNotActive_Enter, stateNotActive_Update, stateNotActive_Exit);


//--State Sleep----------------------------------------------------------------
void stateSleep_Enter(void)
{
	pc.printf("stateSleep_Enter\n\r");
	sdLogger.log((char*)"stateSleep_Enter");

//    hartBeatBlinker.attach_us(&hartBeatBlink, 100000);	
}

void stateSleep_Update(void)
{
	pc.printf("stateSleep_Update\n\r");
	//sdLogger.log((char*)"stateSleep_Update");

	if(moveDetector.awake.debounceSignalSate == DebounceSignalSate_On)
	{
		stateMachine.transitionTo(stateActive);
	}
}

void stateSleep_Exit(void)
{
	pc.printf("stateSleep_Exit\n\r");
	sdLogger.log((char*)"stateSleep_Exit");

//	hartBeatBlinker.detach();
}
State stateSleep = State(stateSleep_Enter, stateSleep_Update, stateSleep_Exit);

//--State Send SMS-------------------------------------------------------------
void stateSendSMS_Enter(void)
{

}

void stateSendSMS_Update(void)
{
	stateMachine.transitionTo(stateActive);
}

void stateSendSMS_Exit(void)
{

}
State stateSendSMS = State(stateSendSMS_Enter, stateSendSMS_Update, stateSendSMS_Exit);

//--Common---------------------------------------------------------------------
void stateMachine_Process(void)
{
	moveDetector.Process();
	baro.Process();
	compass.Process();

	stateMachine.update();
}


