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
    BufferedSerial &pc;

    MissionControllerState state;

    constexpr static const float PI = 3.1415926535897932f;
    constexpr static const float TIRE_RADIUS = 0.030; // tire radius [m]

public:
    MissionController(DCMotor &motor, Localizer &localizer, DigitalIn &start, DigitalOut &motorsEnable, BufferedSerial &pc) : 
        thread(osPriorityNormal, 2048),
        motor(motor),
        localizer(localizer),
        start(start),
        motorsEnable(motorsEnable),
        pc(pc)
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
            char c = 0;
            if (pc.readable()) pc.read(&c, 1);

            MissionControllerData d;
            LocalizerData localizerData = localizer.getData();            

            switch(state){
                case Idle: {
                    setVelocity(0.0);

                    // transition
                    if(!start | (c == 's')) {
                        toState(FineScan);
                        localizer.reset();
                        printf("[MissionController] Starting Scan \n");
                    };
                    break;
                };
                case CoarseScan: {
                    // not implemented
                    break;
                };
                case FineScan: {
                    setVelocity(-0.010);

                    // transition
                    if((localizerData.entryDepth > 0.5) | (c == 's')) {
                        toState(Return);
                        printf("[MissionController] Returning \n");
                    };
                    break;
                };
                case NoScan: {

                    break;
                };
                case Return: {
                    setVelocity(+0.200);

                    // transition
                    if(localizerData.entryDepth <= 0.0) {
                        toState(Idle);
                        printf("[MissionController] Returned \n");
                    };
                    break;
                }
            };

            // update data
            d.state = state;
            mutex.lock();
            data = d;
            mutex.unlock();

            ThisThread::sleep_for(50ms);
        }
    }
};