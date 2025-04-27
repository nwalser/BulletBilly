#pragma once

#include <cstdlib>
#include <deque>
#include <mbed.h>
#include "Point.h"

class LIDAR2 {
    private:
        static const unsigned short DATA_SIZE = 5;
        
        static const char   START_FLAG = 0xA5;
        static const char   SCAN = 0x20;
        static const char   STOP = 0x25;
        static const char   RESET = 0x40;
        
        static const char QUALITY_THRESHOLD = 10;
        const float PI = 3.1415926535897932f;
        static const float DISTANCE_THRESHOLD;
        static const float DEFAULT_DISTANCE;

        UnbufferedSerial&   serial;
        char                dataCounter = 0;
        char                data[DATA_SIZE];
        float               distances[360];
        bool                skipNext = false;

    public:
        LIDAR2(UnbufferedSerial& serial) : serial(serial){
            serial.format(8, SerialBase::None, 1);
            serial.baud(115200);
            serial.attach(callback(this, &LIDAR2::receive));

            char bytes[] = {START_FLAG, SCAN};
            serial.write(&bytes, 2);

            printf("LIDAR Start \n");
        }
        
        virtual ~LIDAR2(){
            char bytes[] = {START_FLAG, STOP};
            serial.write(&bytes, 2);

            printf("LIDAR Stop \n");
        }
        

        deque<Point> getScan(){
            deque<Point> scan;
            
            for (unsigned short i = 0; i < 360; i++) {
                scan.push_back(Point(distances[i], (float)i*PI/180.0f));
            }
            
            return scan;
        }

        float getSingle(){
            return distances[0];
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
            int quality = data[0] >> 2;
            float angle = ((unsigned short)data[1] >> 1) | ((unsigned short)data[2] << 7) / 64;
            float distance = (float)((unsigned short)data[3] | ((unsigned short)data[4] << 8))/4000.0f;
            
            distances[0] = distance;

            // reset counter
            dataCounter = 0;
        }
};