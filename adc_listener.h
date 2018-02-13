#ifndef ADC_LISTENER_H
#define ADC_LISTENER_H

#define _POSIX_C_SOURCE 199309L
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <time.h>
#include "lab5_common.h"

#define AIN1 "/sys/devices/ocp.3/helper.17/AIN1" // Front
#define AIN2 "/sys/devices/ocp.3/helper.17/AIN2" // Right
#define AIN3 "/sys/devices/ocp.3/helper.17/AIN3" // Left
#define AIN4 "/sys/devices/ocp.3/helper.17/AIN4" // Rear

#define FREQUENCY 20
#define SIG_USER 20

#define REPORT_T 10000000

#define LEN 12

int read_adc(char *);
void selfdriving_handler(int, siginfo_t *, void *);
void timer_handler(int);
 
#endif //ADC_LISTENER_H
