#include "mbed.h"
#include "fsm.h"

#include "moveDetector.h"
#include "barometer.h"
#include "compass.h"

Barometer baro;
Compass compass;
MoveDetector moveDetector;

//DigitalOut myled(LED1);
Serial pc(USBTX, USBRX);


int main()
{    
    while(1) 
    {
        pc.printf("--------------------------------------------------\n\r");
//        myled = !myled;
        stateMachine_Process();
        wait(1);        
    }
}
