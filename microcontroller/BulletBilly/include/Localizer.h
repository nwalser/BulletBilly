#pragma once

#include "mbed.h"
#include "EncoderCounter.h"
#include "DCMotor.h"
#include "IMU.h"
#include "AvgFilter.h"

struct LocalizerData {
    float entryDepth;
    float tilt;

    bool isSlipping;
    bool isOvertilt;
};

class Localizer {
private:
    Thread thread;
    AvgFilter tiltFilter;
    LocalizerData data;
    Mutex mutex;

    EncoderCounter &encoder;
    DCMotor &motor;
    IMU &imu;

    constexpr static const float PI = 3.1415926535897932f;
    constexpr static const float COUNTS_PER_TURN = 2000; // [1]
    constexpr static const float TIRE_RADIUS = 0.030; // tire radius [m]
    constexpr static const float MAXIMAL_TILT = 10; // maximal tilt [degrees]

public:
    Localizer(DCMotor &motor, EncoderCounter &encoder, IMU &imu) : 
        thread(osPriorityNormal, 2048),
        tiltFilter(5),
        motor(motor),
        encoder(encoder),
        imu(imu)
    {
        printf("[Localizer] Initializing \n");

        thread.start(callback(this, &Localizer::run));

        printf("[Localizer] Initializing done \n");
    }

    LocalizerData getData(){
        mutex.lock();
        LocalizerData d = data;
        mutex.unlock();
        return d;
    };

private:
    void run(){
        long count = encoder.read();
        short previousCount = count;     

        while(true){
            LocalizerData d;


            // update encoder
            const short actualCount = encoder.read();
            const short countDelta = actualCount - previousCount; // avoid overflow
            previousCount = actualCount;
            count -= countDelta;

            // calculate entry depth
            float tireCircumference = 2 * PI * TIRE_RADIUS;
            float rotations = count / COUNTS_PER_TURN;
            d.entryDepth = rotations * tireCircumference;

            // calculate tilt
            ImuData imuData = imu.getImuData();
            Eigen::Vector3f acc = imuData.acc.normalized();
            d.tilt = tiltFilter.apply(std::acos(acc(1)) * 180.0f / M_PI - 90) ;
            d.isOvertilt = abs(d.tilt) > MAXIMAL_TILT;

            // calculate slippage
            // TODO
            d.isSlipping = false;


            // update data
            mutex.lock();
            data = d;
            mutex.unlock();


            ThisThread::sleep_for(100ms);
        }
    }
};