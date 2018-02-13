#ifndef BT_LISTENER_H
#define BT_LISTENER_H

#define _BSD_SOURCE
#define _POSIX_C_SOURCE 199309L

#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/types.h>

#define DEVPATH "/sys/devices/bone_capemgr.9/slots"

#define BAUDRATE B115200

#define LEN 12

#define UART1 "/dev/ttyO2"

void enable_UART1();
void signal_handler_IO (int status);  
void signal_handler_ADC(int, siginfo_t *, void *);
#endif //BT_LISTENER_H
