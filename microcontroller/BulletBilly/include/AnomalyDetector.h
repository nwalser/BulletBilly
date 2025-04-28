#pragma once

#include "mbed.h"
#include "Lidar.h"
#include <array>
#include <chrono>
#include <Eigen/Dense>

struct AnomalyDetectorData {
    float centerX;
    float centerY;
    float radius;

    array<float, 360> scan;
    array<float, 360> fit;
    array<float, 360> offset;
    array<bool, 360> anomaly;

    bool any_anomaly;
};

class AnomalyDetector {
private:
    constexpr static const Kernel::Clock::duration_u32 CYCLE = 1000ms;
    constexpr static const float PI = 3.1415926535897932f;
    constexpr static const float ANOMALY_THRESHOLD = 0.005f;

    Thread thread;
    Lidar &lidar;

    AnomalyDetectorData data;
    Mutex mutex;

public:
    AnomalyDetector(Lidar &lidar) : lidar(lidar), thread(osPriorityNormal, 8192 * 2) {
        thread.start(callback(this, &AnomalyDetector::run));
    }

    ~AnomalyDetector() {
        thread.terminate();
    }

    AnomalyDetectorData getData() {
        mutex.lock();
        AnomalyDetectorData d = data;
        mutex.unlock();
        return d;
    }

private:
    void fitCircleLeastSquares(AnomalyDetectorData &d) {
        // TODO: skip zero values for fitting to not change result


        int N = d.scan.size();
        Eigen::MatrixXd A(N, 3);  // Matrix A (n x 3)
        Eigen::VectorXd B(N);     // Vector B (n x 1)

        // Step 1: Fill matrices A and B
        for (int i = 0; i < N; ++i) {
            float angle = i * PI / 180.0;
            float r = d.scan[i];
            float x = r * cos(angle);
            float y = r * sin(angle);

            A(i, 0) = x;
            A(i, 1) = y;
            A(i, 2) = 1;
            B(i) = -(x * x + y * y);
        }

        // Step 2: Solve the system A * X = B using least squares
        Eigen::Vector3d solution = A.colPivHouseholderQr().solve(B);

        // Step 3: Extract circle parameters
        float centerX = solution(0) / -2.0f;
        float centerY = solution(1) / -2.0f;
        float radius = std::sqrt(centerX * centerX + centerY * centerY - solution(2));

        // Set the circle parameters
        d.centerX = centerX;
        d.centerY = centerY;
        d.radius = radius;
    }


    void run() {
        while(true){
            AnomalyDetectorData d;

            // scan with 360 point from 0 to 360 degrees
            d.scan = lidar.getScan();

            fitCircleLeastSquares(d);

            // evaluate each point on circle
            for (int i = 0; i < 360; i++) {
                // TODO: angle is currently given in coordinates of circle, 
                // but this should be given in coordinates of origin center
                float angle = i * PI / 180.0;
                float x = d.centerX + d.radius * cos(angle);
                float y = d.centerY + d.radius * sin(angle);
                
                float distance = sqrt(x*x + y*y);

                d.fit[i] = distance;
                d.offset[i] = abs(d.fit[i] - d.scan[i]);
                d.anomaly[i] = d.offset[i] > ANOMALY_THRESHOLD;

                d.any_anomaly |= d.anomaly[i];
            }

            // update data
            mutex.lock();
            data = d;
            mutex.unlock();

            printf("%.3f %.3f %.3f \n", data.centerX, data.centerY, data.radius);

            // wait for next cycle
            ThisThread::sleep_for(CYCLE);
        }
    }
};