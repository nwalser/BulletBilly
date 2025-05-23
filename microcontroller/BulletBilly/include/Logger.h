#pragma once

#include "mbed.h"
#include <SDBlockDevice.h>
#include <FATFileSystem.h>
#include <array>
#include <cstdio>

#include "AnomalyDetector.h"
#include "Localizer.h"
#include "MissionController.h"

class Logger {
private:
    Thread thread;
    SDBlockDevice &sd;
    FATFileSystem &fs;
    FILE *logFile;

    AnomalyDetector &detector;
    Localizer &localizer;
    MissionController &controller;

public:
    Logger(SDBlockDevice &sd, FATFileSystem &fs, AnomalyDetector &detector, Localizer &localizer, MissionController &controller) : 
        thread(osPriorityNormal, 4096*4),
        sd(sd),
        fs(fs),
        detector(detector),
        localizer(localizer),
        controller(controller)
    {
        printf("[Logger] Initializing SD card \n");
        int status = sd.init();
        if (status != 0) {
            printf("[Logger] SD card initialization failed with status %d\n", status);
            return;
        }

        if (sd.frequency(10000000) != 0) {
            printf("[Logger] SDWriter: set frequency failed (not fatal)\n");
        }
        if (fs.mount(&sd) != 0) {
            printf("[Logger] SDWriter: FS mount failed\n");
            sd.deinit();
        }

        printf("[Logger] Initializing SD card done \n");
        thread.start(callback(this, &Logger::run));
    }

    ~Logger(){
        thread.terminate();
    }

private:
    void logArray(array<float, 360> &arr){
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

    void openFile(){
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
            printf("[Logger] Error opening log file.\n");
            return;
        }
        printf("[Logger] %s \n", newLogFile);
    }

    void closeFile(){
        fclose(logFile);
        printf("[Logger] Closed log file.\n");
    }

    void run(){
        bool logging = false;

        while(true){
            ThisThread::sleep_for(50ms);

            // get data for log
            AnomalyDetectorData anomalyData = detector.getData();
            LocalizerData localizerData = localizer.getData();
            MissionControllerData controllerData = controller.getData();

            if((logging == true) & (controllerData.state != MissionControllerState::FineScan)){
                closeFile();
                logging = false;
            }

            if((logging == false) & (controllerData.state == MissionControllerState::FineScan)){
                openFile();
                logging = true;
            }

            if(!logging)
                continue;

            // START ROW
            fprintf(logFile, "{");

            // measurement data
            fprintf(logFile, "0");
            fprintf(logFile, ",");
            fprintf(logFile, "0");
            fprintf(logFile, ",");
            logArray(anomalyData.offset);
            fprintf(logFile, ",");
            fprintf(logFile, "0");

            // detection data
            fprintf(logFile, ",");
            fprintf(logFile, "%.3f", anomalyData.centerX);
            fprintf(logFile, ",");
            fprintf(logFile, "%.3f", anomalyData.centerY);
            fprintf(logFile, ",");
            fprintf(logFile, "%.3f", anomalyData.radius);
            fprintf(logFile, ",");
            fprintf(logFile, "%d", anomalyData.any_anomaly != 0);

            // localization data
            fprintf(logFile, ",");
            fprintf(logFile, "%.3f", localizerData.entryDepth);
            fprintf(logFile, ",");
            fprintf(logFile, "%.3f", localizerData.tilt);
            fprintf(logFile, ",");
            fprintf(logFile, "%d", localizerData.isOvertilt);
            fprintf(logFile, ",");
            fprintf(logFile, "%d", localizerData.isSlipping);

            // mission controller data
            fprintf(logFile, ",");
            fprintf(logFile, "%d", controllerData.state);

            fprintf(logFile, "}\n");
            // END ROW
        }
    }
};