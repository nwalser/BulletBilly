#pragma once

#include <array>
#include <cstdlib>
#include <deque>
#include <mbed.h>
#include <vector>

class Lidar {
private:
    static const unsigned short DATA_SIZE = 5;
    
    static const char START_FLAG = 0xA5;
    static const char SCAN = 0x20;
    static const char STOP = 0x25;
    static const char RESET = 0x40;
    
    static const char QUALITY_THRESHOLD = 10;
    constexpr static const float PI = 3.1415926535897932f;
    constexpr static const float DISTANCE_THRESHOLD = 0.01f;
    constexpr static const float DEFAULT_DISTANCE = 0.0f;

    UnbufferedSerial&       serial;
    char                    dataCounter = 0;
    char                    data[DATA_SIZE];
    std::array<float, 360>  distances;
    bool                    skipNext = false;

public:
    Lidar(UnbufferedSerial& serial) : serial(serial){
        serial.format(8, SerialBase::None, 1);
        serial.baud(115200);
        serial.attach(callback(this, &Lidar::receive));

        char bytes[] = {START_FLAG, SCAN};
        serial.write(&bytes, 2);

        printf("Lidar Start \n");
    }
    
    virtual ~Lidar(){
        char bytes[] = {START_FLAG, STOP};
        serial.write(&bytes, 2);

        printf("Lidar Stop \n");
    }
    

    std::array<float, 360> getScan(){
        return distances;
    }
    
private:
    void receive(){
        if (!serial.readable())
            return;

        // read byte
        char byte = 0;
        serial.read(&byte, 1);
        
        if(skipNext){
            skipNext = false;
            return;
        }

        // add byte to buffer
        if(dataCounter < DATA_SIZE)
            data[dataCounter++] = byte;

        // skip evaluation if buffer is not full
        if(dataCounter < DATA_SIZE)
            return;

        // check if package is valid
        bool invalid1 = (data[1] & 0b1) != 1;
        bool invalid2 = ((data[0] >> 1) & 0b1) == ((data[0] >> 0) & 0b1);
        if(invalid1 || invalid2){
            skipNext = true;
            dataCounter = 0;
            return;
        }

        // parse package
        char quality = data[0] >> 2;
        short angle = 360-(((unsigned short)data[1] | ((unsigned short)data[2] << 8)) >> 1)/64;
        float distance = (float)((unsigned short)data[3] | ((unsigned short)data[4] << 8))/4000.0f;
        
        if ((quality < QUALITY_THRESHOLD) || (distance < DISTANCE_THRESHOLD)) 
            distance = DEFAULT_DISTANCE;

        // store values
        angle = (angle % 360 + 360) % 360;
        distances[angle] = distance;

        // reset counter
        dataCounter = 0;
    }
};