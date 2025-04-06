/*
// sd card
#include <SDBlockDevice.h>
#include <FATFileSystem.h>

#define PB_SD_MOSI PC_12
#define PB_SD_MISO PC_11
#define PB_SD_SCK PC_10
#define PB_SD_CS PD_2

SDBlockDevice sd(PB_SD_MOSI, PB_SD_MISO, PB_SD_SCK, PB_SD_CS);
FATFileSystem fs("sd");
FILE *logFile;


int main()
{
    // mount sd card
    printf("Initializing SD card...\n");
    int status = sd.init();
    if (status != 0) {
        printf("SD card initialization failed with status %d\n", status);
        return -1;
    }

    if (sd.frequency(10000000) != 0) {
        printf("SDWriter: set frequency failed (not fatal)\n");
    }
    if (fs.mount(&sd) != 0) {
        printf("SDWriter: FS mount failed\n");
        sd.deinit();
    }

    logFile = fopen("/sd/log.txt", "a");
    if (logFile == NULL) {
        printf("Error opening log file.\n");
        return -1;
    }


    while (true) {
        imu_data = imu.getImuData();
        float roll = imu_data.rpy(0);
        float pitch = imu_data.rpy(1);
        float yaw = imu_data.rpy(2);

        printf("%.2f, %.2f, %.2f \n", roll, pitch, yaw);

        fprintf(logFile, "%.2f, %.2f, %.2f \n", roll, pitch, yaw);
    }
}
*/