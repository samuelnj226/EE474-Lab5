#include "tank_entry.h"

struct sigaction onoff;

int on;

int main() {
    pid_t pid1, pid2;

    //Set up appropriate interrupt handlers
    onoff.sa_flags = SA_SIGINFO;
    onoff.sa_sigaction = &switchhandler;
    sigemptyset(&onoff.sa_mask);
    sigaction(SIGUSR1, &onoff, NULL);

    //Initialize PWM slots and ADC slots
    initializePWMSlots();
    activatePWM(A_SLOT);
    activatePWM(B_SLOT);
    enable_adc();
    activatePWM(BUZZER_SLOT);

    //Activate switch
    activateGPIO(SWITCH_GPIO_NUM);
    setPin(SWITCH_DIR, "in");

    //Turn on green LED to let user know the program's ready
    activateGPIO(LED_GPIO_NUM);
    setPin(LED_DIR, "out");
    isetPin(LED_VAL, 1);

    //Loop infinitely
    while (1) {
        struct timespec t, t2;

        t.tv_sec = 1;

        on = 0;

        while (!on) {
            usleep(100000);
        }

        //Create a child to run the tank program.
        pid1 = fork();
        if (pid1 == 0) { //Tank process
            static char *argv[] = {NULL};
            execv("/root/lab5/tank.exe", argv);
            perror("First fork");
            exit(127);
        } else {
            //Create a child to run the acd_listener program
            pid2 = fork();

            if (pid2 == 0) { //adc_listener process
                static char *argv[]={NULL};
                execv("/root/lab5/adc_listener.exe", argv);
                perror("Second fork");
                exit(127);
            }
        }

        printf("created children\n");

        while (on) {
            usleep(100000);
        }

        //Kill the 2 children
        kill(pid1, SIGTERM);
        kill(pid2, SIGKILL);

        int status;

        //Wait on the 2 children
        waitpid(pid1, &status, 0);
        waitpid(pid2, &status, 0);

        printf("killed children\n");
    }
    return 0;
}

// signal handler for switch to create children
void switchhandler(int signum, siginfo_t * siginfo, void * extra) {
    int on_signal;

    on_signal = siginfo->si_value.sival_int;

    on = on_signal;
}
  
