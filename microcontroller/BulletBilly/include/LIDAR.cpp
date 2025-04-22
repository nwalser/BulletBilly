#include <cmath>
#include "LIDAR.h"

using namespace std;
 
const float LIDAR::DISTANCE_THRESHOLD = 0.01f;  // threshold for measured distance, given in [m]
const float LIDAR::DEFAULT_DISTANCE = 10.0f;    // default distance > range of sensor, given in [m]
const float LIDAR::M_PI = 3.1415926535897932f;  // the mathematical constant PI
const float LIDAR::DISTANCES[360] = {0};

/**
 * Creates a LIDAR object.
 * @param serial a reference to a serial interface to communicate with the laser scanner.
 */
LIDAR::LIDAR(UnbufferedSerial& serial) : serial(serial) {
    
    // initialize serial interface
    
    serial.baud(115200);
    serial.format(8, SerialBase::None, 1);
    
    // initialize local values
    
    headerCounter = 0;
    dataCounter = 0;
    
    for (unsigned short i = 0; i < 360; i++) distances[i] = DEFAULT_DISTANCE;
    
    simulation = true;
    
    // start serial interrupt
    
    serial.attach(callback(this, &LIDAR::receive), UnbufferedSerial::RxIrq);
    
    // start the continuous operation of the LIDAR
    
    char bytes[] = {START_FLAG, SCAN};
    serial.write(&bytes, 2);
}

/**
 * Stops the lidar and deletes this object.
 */
LIDAR::~LIDAR() {
    
    // stop the LIDAR
    
    char bytes[] = {START_FLAG, STOP};
    serial.write(&bytes, 2);
}

/**
 * Get a list of points of a full 360 degree scan.
 * @return a deque vector of 360 point objects.
 */
deque<Point> LIDAR::getScan() {
    
    deque<Point> scan;
    
    for (unsigned short i = 0; i < 360; i++) {
        
        if (simulation) {
            
            // use simulated distances, because LIDAR is not available
            
            scan.push_back(Point(DISTANCES[i]-0.002f*(rand()%10), (float)i*M_PI/180.0f));
            
        } else {
            
            // use latest measurements from actual LIDAR
            
            scan.push_back(Point(distances[i], (float)i*M_PI/180.0f));
        }
    }
    
    return scan;
}

/**
 * Get a list of points which are part of beacons.
 * @return a deque vector of points that are beacons.
 */
deque<Point> LIDAR::getBeacons() {
    
    // get a list of all points of a scan
    
    deque<Point> scan = getScan();
    
    // create a list of points of beacons
    
    deque<Point> beacons;
    
    // max distance of 3 meter

    for(int i = 0; i < scan.size(); i++){
        Point p1 = scan[i];

        // skip if distance is larger than 3m
        if(p1.distance() > 3.0)
            continue;
        
        //beacons.push_back(p1);

        // at least one point closer than 0.1m
        bool hasClosePoints = false;
        bool hasMiddlePoints = false;

        for(int j = 0; j < scan.size(); j++){
            if(j == i) 
                continue;

            float distance = p1.distance(scan[j]);
            hasClosePoints |= (distance <= 0.1); 
            hasMiddlePoints |= (distance > 0.1 & distance < 0.5); 
        }

        if(!hasClosePoints)
            continue;

        if(hasMiddlePoints)
            continue;

        beacons.push_back(p1);
    }
    
    return beacons;
}

/**
 * This method is called by the serial interrupt service routine.
 * It handles the reception of measurements from the LIDAR.
 */
void LIDAR::receive() {
    
    // read received characters while input buffer is full
    
    if (serial.readable()) {
        
        // read single character from serial interface
        
        char byte = 0;
        serial.read(&byte, 1);
        
        // add this character to the header or to the data buffer
        
        if (headerCounter < HEADER_SIZE) {
            headerCounter++;
        } else {
            if (dataCounter < DATA_SIZE) {
                data[dataCounter] = byte;
                dataCounter++;
            }
            if (dataCounter >= DATA_SIZE) {
                
                // data buffer is full, process measurement
                
                char quality = data[0] >> 2;
                short angle = 360-(((unsigned short)data[1] | ((unsigned short)data[2] << 8)) >> 1)/64;
                float distance = (float)((unsigned short)data[3] | ((unsigned short)data[4] << 8))/4000.0f;
                
                if ((quality < QUALITY_THRESHOLD) || (distance < DISTANCE_THRESHOLD)) 
                    distance = DEFAULT_DISTANCE;
                
                // store distance in [m] into array of full scan
                
                while (angle < 0) angle += 360;
                while (angle >= 360) angle -= 360;
                distances[angle] = distance;
                
                // reset data counter and simulation flag
                
                dataCounter = 0;
                simulation = false;
            }
        }
    }
}
