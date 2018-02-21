#include "lab5_common.h"

// Write flag to the file corresponding to path
void setPin(char * path, char * flag) {
    // Attempt to open the file; loop until file is found 
    FILE *f = NULL;
    while (!!access(path, W_OK));
    f =  fopen(path, "w");

    fprintf(f, "%s", flag);
    fclose(f);
}

// Activate the GPIO corresponding to gnum
void activateGPIO(int gnum) {
    // Attempt to open the file; loop until file is found
    FILE *f = NULL;
    while (!!access(GPIO_EXPORT_PATH, W_OK));
    f =  fopen(GPIO_EXPORT_PATH, "w");

    // Create configuration files for GPIO gnum
    fprintf(f, "%d", gnum);
    fclose(f);
}

void enable_adc() {
    // Attempt to open the file; loop until file is found
    FILE *f = NULL;
    while (!!access(SLOTS_PATH, W_OK));
    f =  fopen(SLOTS_PATH, "w");

    fprintf(f, "cape-bone-iio");

    fclose(f);
}

void initializePWMSlots() {
    // Attempt to open the file; loop until file is found
    FILE *f = NULL;
    while (!!access(SLOTS_PATH, W_OK));
    f =  fopen(SLOTS_PATH, "w");

    fprintf(f, "am33xx_pwm");
    fclose(f);
}

// Create configuration folders for PWM EHRPWM1A
void activatePWM(char * pwm) {
    // Attempt to open the file; loop until file is found
    FILE *f = NULL;
    while (!!access(SLOTS_PATH, W_OK));
    f =  fopen(SLOTS_PATH, "w");

    // Set up configuration folders for EHRPWM1A
    fprintf(f, "%s", pwm);
    fclose(f);
}

// Write flag to the file corresponding to path (integer version)
void isetPin(char * path, int flag) {
    // Attempt to open the file; loop until file is found
    FILE *f = NULL;
    while (!!access(path, W_OK));
    f =  fopen(path, "w");

    fprintf(f, "%d", flag);
    fclose(f);
}
