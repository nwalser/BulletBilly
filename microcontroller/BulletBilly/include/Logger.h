#pragma once

#include "mbed.h"
#include <SDBlockDevice.h>
#include <FATFileSystem.h>
#include <array>
#include <cstdio>
#include "AnomalyDetector.h"

class Logger {
private:
    Thread thread;
    SDBlockDevice &sd;
    FATFileSystem &fs;
    FILE *logFile;

    AnomalyDetector &detector;

public:
    Logger(SDBlockDevice &sd, FATFileSystem &fs, AnomalyDetector &detector) : 
        thread(osPriorityNormal, 4096*4),
        sd(sd),
        fs(fs),
        detector(detector)
    {
        printf("Initializing SD card \n");
        int status = sd.init();
        if (status != 0) {
            printf("SD card initialization failed with status %d\n", status);
            return;
        }

        if (sd.frequency(10000000) != 0) {
            printf("SDWriter: set frequency failed (not fatal)\n");
        }
        if (fs.mount(&sd) != 0) {
            printf("SDWriter: FS mount failed\n");
            sd.deinit();
        }

        // find unused log file slot
        int counter = 0;
        for(int i = 0; i < 999; i++){
            char filename[32];
            snprintf(filename, sizeof(filename), "/sd/log%d.txt", i);
            
            FILE *file = fopen(filename, "r");
            
            if (!file) {
                counter = i;
                break;
            }

            fclose(file);
        }

        // create new log file
        char newLogFile[32];
        snprintf(newLogFile, sizeof(newLogFile), "/sd/log%d.txt", counter);

        logFile = fopen(newLogFile, "a");
        if (logFile == NULL) {
            printf("Error opening log file.\n");
            return;
        }
        printf("%s", newLogFile);
        printf("\n");

        printf("Initializing SD card done \n");
        thread.start(callback(this, &Logger::run));
    }

    ~Logger(){
        thread.terminate();
    }

private:
    void logArray(array<float, 360> arr){
        fprintf(logFile, "{");
        for(int i = 0; i < arr.size(); i++){
            fprintf(logFile, "%.3f", arr[i]);

            if(i < arr.size()-1)
                fprintf(logFile, ",");
        }
        fprintf(logFile, "}");
    }

    void logArray(array<bool, 360> arr){
        fprintf(logFile, "{");
        for(int i = 0; i < arr.size(); i++){
            fprintf(logFile, "%d", arr[i]);

            if(i < arr.size()-1)
                fprintf(logFile, ",");
        }
        fprintf(logFile, "}");
    }


    void run(){
        while(true){
            // get data for log
            AnomalyDetectorData ad_data = detector.getData();

            // START ROW
            fprintf(logFile, "{");

            logArray(ad_data.scan);
            logArray(ad_data.fit);
            logArray(ad_data.offset);
            logArray(ad_data.anomaly);

            fprintf(logFile, "}");
            // END ROW

            ThisThread::sleep_for(1000ms);
        }
    }

};