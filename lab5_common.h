#ifndef LAB_5_COMMON_H
#define LAB_5_COMMON_H

#include <stdio.h>
#include <unistd.h>

#define GPIO_EXPORT_PATH "/sys/class/gpio/export"
#define SLOTS_PATH "/sys/devices/bone_capemgr.9/slots"

void setPin(char *, char *);
void isetPin(char *, int);
void activateGPIO(int);
void enable_adc();
void initializePWMSlots();
void activatePWM(char *);

#endif // LAB_5_COMMON_H
