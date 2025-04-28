#include "mbed.h"
#include "Lidar.h"
#include <array>
#include <chrono>

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
    constexpr static const std::chrono::microseconds CYCLE = 100ms;
    constexpr static const float PI = 3.1415926535897932f;
    constexpr static const float ANOMALY_THRESHOLD = 0.005f;

    Ticker ticker;
    Lidar &lidar;

    AnomalyDetectorData data;

public:
    AnomalyDetector(Lidar &lidar) : lidar(lidar) {
        ticker.attach(callback(this, &AnomalyDetector::run), CYCLE);
    }

    ~AnomalyDetector() {
        ticker.detach();
    }

    AnomalyDetectorData getData() {
        return data;
    }

private:
    void run() {
        AnomalyDetectorData d;

        // scan with 360 point from 0 to 360 degrees
        d.scan = lidar.getScan();

        // FIT CIRCLE
        float sum_x = 0, sum_y = 0, sum_xx = 0, sum_yy = 0, sum_xy = 0;
        int N = d.scan.size();

        // create system
        for (int i = 0; i < N; ++i) {
            float angle = i * PI / 180.0;
            float r = d.scan[i];

            float x = r * cos(angle);
            float y = r * sin(angle);

            sum_x += x;
            sum_y += y;
            sum_xx += x * x;
            sum_yy += y * y;
            sum_xy += x * y;
        }

        // solve
        float A = N * sum_xx - sum_x * sum_x;
        float B = N * sum_xy - sum_x * sum_y;
        float C = N * sum_yy - sum_y * sum_y;
        float D = (sum_xx * sum_y - sum_x * sum_xy);
        float E = (sum_xy * sum_y - sum_x * sum_yy);

        d.centerX = D / A;
        d.centerY = E / A;
        d.radius = sqrt((sum_xx + sum_yy - 2 * (d.centerX * sum_x + d.centerY * sum_y)) / N);


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

        data = d;
    }
};