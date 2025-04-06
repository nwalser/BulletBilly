#include "mbed.h"


// pin mappings
#include "PESBoardPinMap.h"

// imu
#include "IMU.h"

ImuData imu_data;
IMU imu(PB_IMU_SDA, PB_IMU_SCL);

// motor 1
#include "DCMotor.h"

DigitalOut enable_motors(PB_ENABLE_DCMOTORS);

const float voltage_max = 12.0f;
const float gear_ratio = 50.0f;
const float kn = 200.0f / 12.0f;
const float counts_per_turn = 64;
DCMotor motor(PB_PWM_M1, PB_ENC_A_M1, PB_ENC_B_M1, gear_ratio, kn, voltage_max, counts_per_turn);

DigitalIn button(BUTTON1);

int main()
{
    while(button) { };

    enable_motors = 1;
    motor.setVelocity(motor.getMaxVelocity() * 0.03f);
    motor.enableMotionPlanner();

    while(true){
        float rotation = motor.getRotation();
        printf("%.2f \n", rotation);
    }
}