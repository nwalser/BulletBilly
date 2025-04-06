





class PositionEstimator {
private:



public:
    double getCurrentPosition(){
        return 0;
    };

    // get the current slipage of the motor
    double getCurrentSlip(){
        return 0;
    };



};



// PositionEstimator
// estimates the current position based on encoder values from motors

// PositionEstimator2
// estimates the current position based on separate wheel

// SlippageDetector
// detects slippage of the two position estimators

// PoseEstimator
// estimate the current pose based on gyro and accelerometer measurements

// MissionController
// controls the current roboter tasks based on the planned mission