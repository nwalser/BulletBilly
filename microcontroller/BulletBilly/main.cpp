#include "mbed.h"
#include <Eigen/Dense>

#include "IMU.h"


// imu
#define PB_IMU_SDA PC_9
#define PB_IMU_SCL PA_8

ImuData imu_data;
IMU imu(PB_IMU_SDA, PB_IMU_SCL);

using Eigen::MatrixXd;

DigitalOut do1 = DigitalOut(LED1);


int main()
{
    while (true) {
        imu_data = imu.getImuData();
        float roll = imu_data.rpy(0);
        float pitch = imu_data.rpy(1);
        float yaw = imu_data.rpy(2);

        printf("%.2f, %.2f, %.2f \n", roll, pitch, yaw);
    }
}

