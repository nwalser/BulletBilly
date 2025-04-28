#include "mbed.h"
#include "PESBoardPinMap.h"

#include <array>

#include "Lidar.h"
#include "DCMotor.h"
#include "IMU.h"
#include "AnomalyDetector.h"

// motor
const float voltage_max = 12.0f;
const float gear_ratio = 50.0f;
const float kn = 200.0f / 12.0f;
const float counts_per_turn = 64;
DCMotor motor(PB_PWM_M1, PB_ENC_A_M1, PB_ENC_B_M1, gear_ratio, kn, voltage_max, counts_per_turn);
DigitalOut motors_enable(PB_ENABLE_DCMOTORS);

// encoder
EncoderCounter encoder(PB_ENC_A_M2, PB_ENC_B_M2);

// lidar
UnbufferedSerial serial(PC_10, PC_11);
Lidar lidar(serial);
AnomalyDetector anomalyDetector(lidar);

// imu
IMU imu(PB_IMU_SDA, PB_IMU_SCL);

// button
DigitalIn button(BUTTON1);


#include <array>
#include <numeric>
template<size_t N>
float average(const std::array<float, N>& arr) {
    if (N == 0) return 0.0f;
    return std::accumulate(arr.begin(), arr.end(), 0.0f) / N;
}


int main() {
    // while(button){}

    // motors_enable.write(true);

    // motor.setVelocity(motor.getMaxVelocity() * 0.03f);
    // motor.enableMotionPlanner();

    while(true){
        //deque<Point> points = lidar.getScan();
        ImuData imu_data = imu.getImuData();
        float rotation = motor.getRotation();
        std::array<float, 360> single = lidar.getScan();

        float aver = average(single);

        AnomalyDetectorData ad_data = anomalyDetector.getData();

        printf("%.2f %.2d %.2f %.2f %d \n", rotation, encoder.read(), imu_data.tilt, aver, ad_data.any_anomaly);
    }
}