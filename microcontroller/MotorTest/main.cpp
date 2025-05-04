#include "mbed.h"
#include "FastPWM.h"

// main() runs in its own thread in the OS
int main()
{
    DigitalOut enable(PB_15);
    FastPWM pwm(PB_13);

    enable = true;

    while (true) {
        pwm.write(0.53);

        printf("Pwm \n");
        ThisThread::sleep_for(100ms);
    }
}

