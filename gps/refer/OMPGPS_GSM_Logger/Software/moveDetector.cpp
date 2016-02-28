#include "moveDetector.h"
#include "I2cDataBus.h"
extern Serial pc;

//I2C     mdI2c(I2C_SDA, I2C_SCL);
//MPU6050 mdMpu(mdI2c);
MPU6050 mdMpu(i2cDataBus);

InterruptIn moveDetectPin(PC_9);

Timer zeroMovementTimer;

void moveDetectedCallBack(void) 
{
//    pc.printf("----------moveDetectedCallBack\n\r");
    zeroMovementTimer.reset();
}

MoveDetector::MoveDetector(void):SDLogger("mdetect", "log"),/*debounceSignal(50, 5, 10)*/notActive(50, 10, 25), acive(50, 10, 25), sleep(200, 1, 2), awake(120, 1, 2)
{};

bool MoveDetector::InitDetector(void)
{
    pc.printf("MPU6050 initialize \n");
 
    moveDetectPin.rise(&moveDetectedCallBack);
    zeroMovementTimer.start();

    mdMpu.initialize();
    mdMpu.setAccelerometerPowerOnDelay(3);
    mdMpu.setIntZeroMotionEnabled(false);
    mdMpu.setIntMotionEnabled(true);    
    mdMpu.setDHPFMode(1);
    mdMpu.setZeroMotionDetectionThreshold(2);
    mdMpu.setZeroMotionDetectionDuration(2);     
    mdMpu.setMotionDetectionThreshold(2);
    mdMpu.setMotionDetectionDuration(5);
    
    pc.printf("MPU6050 testConnection \n");
 
    bool mpu6050TestResult = mdMpu.testConnection();
    if(mpu6050TestResult) {
        pc.printf("MPU6050 test passed \n");
    } else {
        pc.printf("MPU6050 test failed \n");
    }	

    return mpu6050TestResult;
}

bool MoveDetector::NoMoveDetected(void)
{
    if(zeroMovementTimer.read() > 1)
    {
        pc.printf("No movement detected\n\r");
        return true;
    }

    return false;    
}

float MoveDetector::GetInternTemperature(void)
{
    return((mdMpu.getTemperature()/340.)+36.53);
}

bool MoveDetector::MotionStatus(void)
{
    return mdMpu.getIntMotionStatus();
}

void MoveDetector::SleepIn(void)
{
}

void MoveDetector::SleepOut(void)
{
}

void MoveDetector::Process(void)
{
    bool mSt = MotionStatus();
    printf("%d\n", mSt);
    log_printf("%d\n", mSt);

    printf("Temp: %f\n", (mdMpu.getTemperature()/340.)+36.53);
    log_printf("Temp: %f\n", (mdMpu.getTemperature()/340.)+36.53);

    notActive.Process(!mSt);
    acive.Process(mSt);
    sleep.Process(!mSt);
    awake.Process(mSt);
}

//--DebounceSignal-----------------------------------------------------------------------
DebounceSignal::DebounceSignal(uint8_t maxVal, uint8_t raiseVal, uint8_t fallVal)
{
    MaxVal      = maxVal;
    RaiseVal    = raiseVal;
    FallVal     = fallVal;
    debVal      = DebounceSignalSate_Unknown;
}

DebounceSignalSate DebounceSignal::Process(bool signal)
{
    if(signal)
    {
        if(debVal <= (MaxVal - RaiseVal))
        {
            debVal += RaiseVal;
        }
        else
        {
            debVal = MaxVal;
        }
    }
    else
    {
        if(debVal >= FallVal)
        {
            debVal -= FallVal;
        }
        else
        {
            debVal = 0x00;
        }
    }

    if(debVal >= MaxVal)
    {
        debounceSignalSate = DebounceSignalSate_On;
    }
    else if(debVal == 0x00)
    {
        debounceSignalSate = DebounceSignalSate_Off;
    }
    
    return debounceSignalSate;
}

void DebounceSignal::Reset(void)
{
    debVal = DebounceSignalSate_Unknown;
}

//--Common-------------------------------------------------------------------------------
//MoveDetector moveDetector;
