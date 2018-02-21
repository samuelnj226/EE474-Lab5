#include "tank.h"

// Information about the GPIOs we're using
struct Gpio GPIOS[] = {
    {27, AIN1_VAL, AIN1_DIR},
    {47, AIN2_VAL, AIN2_DIR},
    {46, BIN1_VAL, BIN1_DIR},
    {65, BIN2_VAL, BIN2_DIR},
    {60, STBY_VAL, STBY_DIR}
};

// Information about the PWMs we're using
struct Pwm PWMS[] = {
    {A_PPATH, A_DPATH, A_RPATH, A_SLOT, START_DUTY},
    {B_PPATH, B_DPATH, B_RPATH, B_SLOT, START_DUTY}
};

// signal action structures
struct sigaction sa, quit, adc_receive;

// self drive on or off
int selfdrive_flag;

// adc information
int front, right, left, rear;

// Process ID of adc_listener
pid_t pid_adc;

// uninterrupted sleep time
int sleep_time;


int main() {
    //activate GPIOs
    for(int i = 0; i < NUM_DB; i++) {
        activateGPIO(GPIOS[i].num);
        setPin(GPIOS[i].direction_p, "out");
    }

    //Activate PWMs
    initializePWMSlots();
    for (int i = 0; i < NUM_PWM; i++) {
        activatePWM(PWMS[i].slot);
    }

    //Set up the buzzer
    activatePWM(BUZZER_SLOT);

    isetPin(BUZZER_RPATH, 0);
    isetPin(BUZZER_PPATH, 1000000);
    isetPin(BUZZER_DPATH, 500000);

    // set up signal handler for handling commands
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = &sighandler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGUSR1, &sa, NULL);

    // set up signal handler for adc data retrieval
    adc_receive.sa_flags = SA_SIGINFO;
    adc_receive.sa_sigaction = &signal_handler_ADC;
    sigemptyset(&adc_receive.sa_mask);
    sigaction(SIGUSR2, &adc_receive, NULL);

    // set up signal handler for exit of process
    quit.sa_handler = &exithandler;
    sigaction(SIGINT, &quit, NULL);
    sigaction(SIGTERM, &quit, NULL);

    // get PID of ADC
    while (pid_adc == 0){
        char line[LEN];
        FILE *cmd = popen("pgrep -f adc_listener.ex", "r"); // b/c adc_listener.exe comes up as as .ex
        printf("adc_PID is %d\n", pid_adc);  
        fgets(line, LEN, cmd);
        pid_adc = strtoul(line, NULL, 10);
        pclose(cmd);

        // check if pid is valid
        int valid = kill(pid_adc, 0);
        if (!!valid) {
            pid_adc = 0;
        }
    } 

    // initialize self drive to off
    selfdrive_flag = 0;

    // initialize blocked set of signals for uninterruped execution
    sigset_t block_set;
    sigemptyset(&block_set);
    sigaddset(&block_set, SIGUSR2);

    // continue to loop
    while(1) {
        usleep(1000000);

        // self driving implementation
        while(selfdrive_flag){
            usleep(1000000);

            drive(0xFAFA); // forward

            if (front > 800) {
                if( left > right) {                             // right 
                    sigprocmask(SIG_BLOCK, &block_set, NULL);     // run uninterrupted from adc_listener
                    drive(0x4040);
                    usleep(1000000);
                    drive(0xFA7A);    //turn right;
                    usleep(1000000);
                    drive(0xFAFA);
                    sigprocmask(SIG_UNBLOCK, &block_set, NULL);
                } else {                                        // left 
                    sigprocmask(SIG_BLOCK, &block_set, NULL);     // run uninterrupted from adc_listener
                    drive(0x4040);
                    usleep(1000000);
                    drive(0x7AFA);    //turn left;
                    usleep(1000000);
                    drive(0xFAFA);
                    sigprocmask(SIG_UNBLOCK, &block_set, NULL);
                }
            } else if (rear > 800) {
                sigprocmask(SIG_BLOCK, &block_set, NULL);     // run uninterrupted from adc_listener
                drive(0xFDFD);
                usleep(1000000);
                sigprocmask(SIG_UNBLOCK, &block_set, NULL);
            }

        }
    }
}

// signal handler for tank motors to turn
void sighandler(int signum, siginfo_t * siginfo, void * extra ) {
    int command;
    union sigval self_drive;

    command = siginfo->si_value.sival_int;

    if(command == 0xFF01){      // self drive on
        selfdrive_flag = 1;
        self_drive.sival_int = 1;
        sigqueue(pid_adc, SIGUSR1, self_drive);
        printf("self drive: on\n");
    } else if(command == 0xFF02){ // self drive off
        selfdrive_flag = 0;
        self_drive.sival_int = 0;
        sigqueue(pid_adc, SIGUSR1, self_drive);

        drive(0x8080);
        isetPin(BUZZER_RPATH, 0);
        printf("self drive: off\n");
    } else {                    // manual drive
        drive(command);
    }
}

// signal handler for exit
void exithandler(int signum) {
    //Turn everything off
    isetPin(GPIOS[4].value_p, 0);
    isetPin(BUZZER_RPATH, 0);
    //isetPin(BL1_VAL, 0);
    //isetPin(BL2_VAL, 0);

    printf("ran exit process\n");
    exit(0);
}

// signal handler of the ADC Data retrieval in self drive
void signal_handler_ADC(int signum, siginfo_t * siginfo, void * extra ) {
    int adc_data;
    adc_data = siginfo->si_value.sival_int;

    // extract data
    rear = (adc_data % 64) * 29;
    adc_data /= 64;
    left = (adc_data % 64) * 29;
    adc_data /= 64;
    right = (adc_data % 64) * 29;
    adc_data /= 64;
    front = adc_data * 29;
}

// set duty of a pwm
void setDuty(struct Pwm *p, int d) {
    //Make sure duty isn't greater than period
    if (d > PERIOD) {
        d = PERIOD;
    }

    //Set the duty and update the current duty
    isetPin(p->duty_p, d);
    p->duty = d;
}

//Drive that takes 2 byte drive code from bluetooth.
void drive(int command) {
    int rightm, leftm, rightdir, leftdir, rightduty, leftduty;

    isetPin(GPIOS[4].value_p, 0);

    rightm = command % 256; // lower byte
    leftm = command / 256;  // upper byte

    rightdir = rightm / 128;
    leftdir = leftm / 128;

    // calculate duty, lower is faster.
    rightduty = PERIOD - (PERIOD/127)*(rightm % 128);
    leftduty = PERIOD - (PERIOD/127)*(leftm % 128);

    //set PWMS to run
    isetPin(PWMS[0].run_p, 1);
    isetPin(PWMS[1].run_p, 1);

    if ((rightdir == 1) && (leftdir == 1)) {
        setPins(0xa); // drive forward
    } else if ((rightdir == 0) && (leftdir == 0)) {
        setPins(0x5); // drive back
    } else if ((rightdir == 1) && (leftdir == 0)) {
        setPins(0x6); // turn left
    } else {
        setPins(0x9); // turn right
    }

    // set period
    isetPin(PWMS[0].period_p, PERIOD);
    isetPin(PWMS[1].period_p, PERIOD);

    // set duty
    setDuty(&PWMS[0], leftduty);
    setDuty(&PWMS[1], rightduty);

    // standy off
    isetPin(GPIOS[4].value_p, 1);
}

// Set pins to configuration specified by config
void setPins(int config) {
    for (int i = 0; i < NUM_DB - 1; i++) {
        isetPin(GPIOS[NUM_DB - 2 - i].value_p, config % 2);
        config /= 2;
    }
}
