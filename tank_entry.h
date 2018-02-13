#ifndef _TANK_ENTRY_H
#define _TANK_ENTRY_H

#define _BSD_SOURCE
#define _POSIX_C_SOURCE 199309L
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <sys/wait.h>
#include <signal.h>
#include "lab5_common.h"

#define SWITCH_GPIO_NUM 20
#define SWITCH_VAL "/sys/class/gpio/gpio20/value"
#define SWITCH_DIR "/sys/class/gpio/gpio20/direction"

#define LED_GPIO_NUM 61
#define LED_VAL "/sys/class/gpio/gpio61/value"
#define LED_DIR "/sys/class/gpio/gpio61/direction"

#define A_SLOT "bone_pwm_P9_14"
#define B_SLOT "bone_pwm_P9_16"
#define BUZZER_SLOT "bone_pwm_P8_13"

#define AIN1 "/sys/devices/ocp.3/helper.17/AIN1"

void switchhandler(int, siginfo_t *, void *);

#endif //_TANK_ENTRY_H
