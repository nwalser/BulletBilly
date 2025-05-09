#include "mbed.h"
#include "PESBoardPinMap.h"
#include <SDBlockDevice.h>
#include <FATFileSystem.h>

#include <array>

#include "Lidar.h"
#include "DCMotor.h"
#include "IMU.h"
#include "AnomalyDetector.h"
#include "Logger.h"
#include "Localizer.h"
#include "MissionController.h"

int main() {
    ThisThread::sleep_for(1s);

    // motor
    const float voltage_max = 12.0f;
    const float gear_ratio = 150.0f;
    const float kn = 200.0f / 12.0f;
    const float counts_per_turn = 64;
    DCMotor motor(PB_PWM_M1, PB_ENC_A_M1, PB_ENC_B_M1, gear_ratio, kn, voltage_max, counts_per_turn);

    // KP = 4.2 * 50 / 78.125
    // KI = 140 * 50 / 78.125
    // KD = 0.0192 * 50 / 78.125
    //motor.setVelocityCntrl(4.2 * 50 / 78.125 * 0, 140 * 50 / 78.125 * 0.1, 0);
    //motor.disableMotionPlanner();

    DigitalOut motorsEnable(PB_ENABLE_DCMOTORS);

    // encoder
    EncoderCounter encoder(PB_ENC_A_M2, PB_ENC_B_M2);

    // lidar
    UnbufferedSerial serial(PA_9, PA_10);
    Lidar lidar(serial);
    AnomalyDetector detector(lidar);

    // imu
    IMU imu(PB_IMU_SDA, PB_IMU_SCL);

    // button
    DigitalIn button(BUTTON1);

    // logging
    SDBlockDevice sd(PB_SD_MOSI, PB_SD_MISO, PB_SD_SCK, PB_SD_CS);
    FATFileSystem fs("sd");

    Localizer localizer(motor, encoder, imu);
    MissionController controller(motor, localizer, button, motorsEnable);


    Logger logger(sd, fs, detector, localizer);


    while(true){
        ThisThread::sleep_for(100ms);
    }
}