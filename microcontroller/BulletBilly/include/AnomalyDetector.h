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

struct Point {
    double x, y;
};

class AnomalyDetector {
private:
    constexpr static const Kernel::Clock::duration_u32 CYCLE = 200ms;
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
        std::vector<Point> pts;
        for (int i = 0; i < d.scan.size(); i+=4) {
            float angle = i * PI / 180.0;
            float r = d.scan[i];
            
            // skip measurements outside of range
            if (r <= 0 || r > 0.25) continue;

            float x = r * cos(angle);
            float y = r * sin(angle);
            pts.push_back({x, y});
        }

        int n = pts.size();
        double sum_x = 0, sum_y = 0;
        double sum_x2 = 0, sum_y2 = 0;
        double sum_x3 = 0, sum_y3 = 0;
        double sum_xy = 0, sum_x1y2 = 0, sum_x2y1 = 0;

        for (const auto& p : pts) {
            double x = p.x, y = p.y;
            double x2 = x * x, y2 = y * y;
            sum_x += x;
            sum_y += y;
            sum_x2 += x2;
            sum_y2 += y2;
            sum_x3 += x2 * x;
            sum_y3 += y2 * y;
            sum_xy += x * y;
            sum_x1y2 += x * y2;
            sum_x2y1 += x2 * y;
        }

        double C = n * sum_x2 - sum_x * sum_x;
        double D = n * sum_xy - sum_x * sum_y;
        double E = n * sum_y2 - sum_y * sum_y;
        double G = 0.5 * (n * sum_x3 + n * sum_x1y2 - (sum_x2 + sum_y2) * sum_x);
        double H = 0.5 * (n * sum_y3 + n * sum_x2y1 - (sum_x2 + sum_y2) * sum_y);
        double denom = C * E - D * D;

        if (std::abs(denom) < 1e-10) return;  // avoid division by zero

        double a = (G * E - D * H) / denom;
        double b = (C * H - D * G) / denom;
        double r = std::sqrt((sum_x2 + sum_y2 - 2 * a * sum_x - 2 * b * sum_y + n * (a * a + b * b)) / n);

        d.centerX = a;
        d.centerY = b;
        d.radius = r;
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