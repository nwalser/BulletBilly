#include "mbed.h"

/*
#include "LIDAR.h"

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

// lidar
//UnbufferedSerial serial(PA_9, PA_10);
//LIDAR* lidar = new LIDAR(*serial);
*/


DigitalIn button(BUTTON1);

UnbufferedSerial serial1(USBTX, USBRX, 115200);
UnbufferedSerial serial2(PC_10, PC_11, 115200);

void forward_A_to_B() {
    char c;
    while (serial1.read(&c, 1) == 1) {
        serial2.write(&c, 1);
    }
}

void forward_B_to_A() {
    char c;
    while (serial2.read(&c, 1) == 1) {
        serial1.write(&c, 1);
    }
}

int main() {
    serial1.format(8, SerialBase::None, 1);
    serial2.format(8, SerialBase::None, 1);

    serial1.attach(&forward_A_to_B);
    serial2.attach(&forward_B_to_A);

    while (button) {
        char c = 'd';
        serial1.write(&c, 1);

        ThisThread::sleep_for(1s);
    }

    // write start
    char start[] = { 165, 32 };
    serial2.write(&start, 2);

    while (1) {
        char c = 'h';
        serial1.write(&c, 1);

        ThisThread::sleep_for(1s);
    }
}

/*
int main()
{
    // initialize serial interface
    //serial.baud(115200);
    //serial.format(8, SerialBase::None, 1);
    
    // start scanning
    //char start[] = {165, 32};
    //serial.write(&start, 2);


    while(button){

    }


    //char stop[] = {165, 37};
    //serial.write(&stop, 2);


    while(true) {
        deque<Point> points = lidar->getScan();
        float count = 0;

        for(int i = 0; i < points.size(); i++){
            count += points[i].distance();
        }

        printf("%.2f \n", count);
    }
    */

    /*
    enable_motors = 1;
    motor.setVelocity(motor.getMaxVelocity() * 0.03f);
    motor.enableMotionPlanner();

    while(true){
        float rotation = motor.getRotation();
        printf("%.2f \n", rotation);
    }
}
*/