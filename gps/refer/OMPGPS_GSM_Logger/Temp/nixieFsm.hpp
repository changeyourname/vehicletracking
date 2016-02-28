#ifndef NIXIE_FSM_HPP
#define NIXIE_FSM_HPP

#include <xpcc/processing/timer.hpp>

#include "nixieDcf.hpp"
#include "nixieClock.hpp"
#include "Fsm/FiniteStateMachine.h"

extern    Nixie::Clock nixieClock;

//namespace Nixie
// {
//	namespace Fsm
	// {

	//--State Startup--------------------------------------------------------------
	void stateStartup_Enter(void);
	void stateStartup_Update(void);
	void stateStartup_Exit(void);
	extern State stateStartup;

	//--State SetTime--------------------------------------------------------------
	void statePreSetTime_Enter(void);
	void statePreSetTime_Update(void);
	void statePreSetTime_Exit(void);
	extern State statePreSetTime;

	void stateSetTime_Enter(void);
	void stateSetTime_Update(void);
	void stateSetTime_Exit(void);
	extern State stateSetTime;

	//--State SetTimeIR------------------------------------------------------------
	void stateSetTimeIR_Enter(void);
	void stateSetTimeIR_Update(void);
	void stateSetTimeIR_Exit(void);
	extern State stateSetTimeIR;

	//--State DisplayTime----------------------------------------------------------
	void stateDisplayTime_Enter(void);
	void stateDisplayTime_Update(void);
	void stateDisplayTime_Exit(void);
	extern State stateDisplayTime;

	//--State stateDisplayUnschrTime-----------------------------------------------
	void stateDisplayUnschrTime_Enter(void);
	void stateDisplayUnschrTime_Update(void);
	void stateDisplayUnschrTime_Exit(void);
	extern State stateDisplayUnschrTime;

	//--State DisplayDcf----------------------------------------------------------
	void stateDisplayDcf_Enter(void);
	void stateDisplayDcf_Update(void);
	void stateDisplayDcf_Exit(void);
	extern State stateDisplayDcf;

	//--Common---------------------------------------------------------------------
	void stateMachine_Process(void);

	//--Fsm------------------------------------------------------------------------
	extern FSM stateMachine;
// 	}
// }

#endif
