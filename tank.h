#ifndef TANK_H
#define TANK_H

#define _BSD_SOURCE
#define _POSIX_C_SOURCE 199309L
#include "lab5_common.h"
#include <stdlib.h>
#include <signal.h>
#include <time.h>

//GPIOS
#define AIN1_VAL "/sys/class/gpio/gpio27/value"
#define AIN2_VAL "/sys/class/gpio/gpio47/value"
#define BIN1_VAL "/sys/class/gpio/gpio46/value"
#define BIN2_VAL "/sys/class/gpio/gpio65/value"
#define STBY_VAL "/sys/class/gpio/gpio60/value"

#define AIN1_DIR "/sys/class/gpio/gpio27/direction"
#define AIN2_DIR "/sys/class/gpio/gpio47/direction"
#define BIN1_DIR "/sys/class/gpio/gpio46/direction"
#define BIN2_DIR "/sys/class/gpio/gpio65/direction"
#define STBY_DIR "/sys/class/gpio/gpio60/direction"

#define BL1_VAL "/sys/class/gpio/gpio49/value"
#define BL2_VAL "/sys/class/gpio/gpio115/value"

#define BL1_DIR "/sys/class/gpio/gpio49/direction"
#define BL2_DIR "/sys/class/gpio/gpio115/direction"

#define NUM_DB 5
#define NUM_PWM 2

//PWMs
#define A_PPATH "/sys/devices/ocp.3/pwm_test_P9_14.15/period"
#define A_DPATH "/sys/devices/ocp.3/pwm_test_P9_14.15/duty"
#define A_RPATH "/sys/devices/ocp.3/pwm_test_P9_14.15/run"

#define B_PPATH "/sys/devices/ocp.3/pwm_test_P9_16.16/period"
#define B_DPATH "/sys/devices/ocp.3/pwm_test_P9_16.16/duty"
#define B_RPATH "/sys/devices/ocp.3/pwm_test_P9_16.16/run"

#define BUZZER_PPATH "/sys/devices/ocp.3/pwm_test_P8_13.18/period" // originally .18 on all to get it to run with boot
#define BUZZER_DPATH "/sys/devices/ocp.3/pwm_test_P8_13.18/duty"
#define BUZZER_RPATH "/sys/devices/ocp.3/pwm_test_P8_13.18/run"

#define A_SLOT "bone_pwm_P9_14"
#define B_SLOT "bone_pwm_P9_16"
#define BUZZER_SLOT "bone_pwm_P8_13"

#define PERIOD 500000
#define START_DUTY 100000

#define T_BEEPS 250000000

#define LEN 12

struct Pwm {
    char *period_p;
    char *duty_p;
    char *run_p;

    char *slot;
    int duty; 
};

struct Gpio {
    int num;

    char *value_p;
    char *direction_p;
};


void sighandler(int, siginfo_t *, void *);
void exithandler(int);
void signal_handler_ADC(int, siginfo_t *, void *);
void setDuty(struct Pwm *, int);
void drive(int);
void setPins(int);

#endif // TANK_H
