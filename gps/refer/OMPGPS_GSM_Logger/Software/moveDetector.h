#ifndef MOVE_DETECTOR_H
#define MOVE_DETECTOR_H

#include "mbed.h"
#include "sdLogger.h"
#include "MPU6050.h"

//extern I2C     mdi2c;
extern MPU6050 mdmpu;

typedef enum DebounceSignalSate
{
    DebounceSignalSate_Unknown = 0,
    DebounceSignalSate_On      = 1,
    DebounceSignalSate_Off     = 2
}DebounceSignalSate;

class DebounceSignal
{
	private:
		uint8_t MaxVal;
		uint8_t RaiseVal;
		uint8_t FallVal;

		uint8_t debVal;
	protected:
	public:
		DebounceSignalSate debounceSignalSate;

		DebounceSignal(uint8_t maxVal, uint8_t raiseVal, uint8_t fallVal);
		DebounceSignalSate Process(bool signal);
		void Reset(void);
};

class MoveDetector: SDLogger
{
private:
//	void Tick(void);
protected:
public:
	DebounceSignal notActive;
	DebounceSignal acive;
	DebounceSignal sleep;
	DebounceSignal awake;

	MoveDetector(void);
	
	bool	InitDetector(void);
	bool	NoMoveDetected(void);
	float	GetInternTemperature(void);
	bool    MotionStatus(void);
	void	SleepIn(void);
	void	SleepOut(void);
	void    Process(void);
};

//extern MoveDetector moveDetector;

#endif
