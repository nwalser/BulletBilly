#pragma once

#include "mbed.h"
#include "DCMotor.h"
#include "Localizer.h"

enum MissionControllerState {
    Idle = 0,
    NoScan = 1,
    CoarseScan = 2,
    FineScan = 3,
    Return = 10,
};

struct MissionControllerData {
    MissionControllerState state;
};

class MissionController {
private:
    Thread thread;
    MissionControllerData data;
    Mutex mutex;
    
    DCMotor &motor;
    Localizer &localizer;
    DigitalIn &start;
    DigitalOut &motorsEnable;

    MissionControllerState state;

    constexpr static const float PI = 3.1415926535897932f;
    constexpr static const float TIRE_RADIUS = 0.030; // tire radius [m]

public:
    MissionController(DCMotor &motor, Localizer &localizer, DigitalIn &start, DigitalOut &motorsEnable) : 
        thread(osPriorityNormal, 2048),
        motor(motor),
        localizer(localizer),
        start(start),
        motorsEnable(motorsEnable)
    {
        printf("[MissionController] Initializing \n");

        thread.start(callback(this, &MissionController::run));

        printf("[MissionController] Initializing done \n");
    }

    MissionControllerData getData(){
        mutex.lock();
        MissionControllerData d = data;
        mutex.unlock();
        return d;
    };

private:
    void setVelocity(float velocity){
        float circumference = TIRE_RADIUS * 2 * PI;
        float rps = velocity / circumference;

        motor.setVelocity(rps);
    }

    void toState(MissionControllerState nextState){
        state = nextState;
    }

    void run(){
        motorsEnable = true;
        state = MissionControllerState::Idle;

        while(true){
            MissionControllerData d;
            LocalizerData localizerData = localizer.getData();

            switch(state){
                case Idle: {
                    setVelocity(0.0);

                    // transition
                    if(!start) toState(FineScan);
                    break;
                };
                case CoarseScan: {
                    // not implemented
                    break;
                };
                case FineScan: {
                    setVelocity(0.02);

                    // transition
                    if(localizerData.entryDepth > 0.3) toState(Return);
                    break;
                };
                case NoScan: {

                    break;
                };
                case Return: {
                    setVelocity(-0.02);

                    // transition
                    if(localizerData.entryDepth <= 0.0) toState(Idle);
                    break;
                }
            };

            // update data
            mutex.lock();
            data = d;
            mutex.unlock();

            ThisThread::sleep_for(100ms);
        }
    }
};