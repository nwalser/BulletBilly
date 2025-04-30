#pragma once

#include "mbed.h"
#include "DCMotor.h"
#include "Localizer.h"

struct MissionControllerData {

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
    void run(){
        motorsEnable = true;

        while(start){
            ThisThread::sleep_for(100ms);
        }

        while(!start){
            ThisThread::sleep_for(100ms);
        }

        printf("[MissionController] Drive \n");

        float velocity = 0.05; // [m/s]
        float circumference = TIRE_RADIUS * 2 * PI;
        float rps = velocity / circumference;

        motor.setVelocity(rps);

        while(start){
            ThisThread::sleep_for(100ms);
        }

        motor.setVelocity(0);

        printf("[MissionController] Stop \n");

        return;


        while(true){
            MissionControllerData d;



            // update data
            mutex.lock();
            data = d;
            mutex.unlock();


            ThisThread::sleep_for(100ms);
        }
    }
};