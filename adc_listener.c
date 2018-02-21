#include "adc_listener.h"

//Timer event structures
struct sigaction sa, selfdrive;
struct sigevent se;

//Timer metadata
struct timespec ts; 
struct itimerspec itimer;
timer_t timerid;

//Running total among samples
//int tot1,tot2,tot3,tot4;
int tot [4];
//Sample number
int s;

//Process ID of the tank process
pid_t pid_tank;

//Process ID of the bt_listener
pid_t pid_bt;

//Self-Driving Flag
int selfdriving_flag;

int main() {
    // initialize averaging to 0
    tot[0] = 0;
    tot[1] = 0;
    tot[2] = 0;
    tot[3] = 0;
    s = 0;

    //self driving initially off
    selfdriving_flag = 0;

    //get PID of tank process
    //from http://stackoverflow.com/questions/8166415/how-to-get-the-pid-of-a-process-in-linux-in-c
    char line[LEN];
    FILE *cmd = popen("pgrep -f tank.exe", "r");
    fgets(line, LEN, cmd);
    pid_tank = strtoul(line, NULL, 10);
    printf("I FOUND TANK");

    //get PID of bt_listener
    cmd = popen("pgrep -f bt_listener.exe", "r");
    fgets(line, LEN, cmd);
    pid_bt = strtoul(line, NULL, 10);
    pclose(cmd);
    printf("I FOUND BT_LISTENER");

    //Set up sigevent for timer 
    memset(&sa, 0, sizeof(sa));

    sa.sa_handler = &timer_handler;
    sigaction (SIGVTALRM, &sa, NULL);

    se.sigev_notify = SIGEV_SIGNAL;
    se.sigev_signo = SIGVTALRM;
    timer_create(CLOCK_REALTIME, &se, &timerid);

    ts.tv_sec = 0;
    ts.tv_nsec = REPORT_T/FREQUENCY;

    itimer.it_interval = ts;
    itimer.it_value = ts;
    timer_settime(timerid, 0, &itimer, NULL);  

    // set up a sigaction for self drive
    selfdrive.sa_flags = SA_SIGINFO;
    selfdrive.sa_sigaction = &selfdriving_handler;
    sigemptyset(&selfdrive.sa_mask);
    sigaction(SIGUSR1, &selfdrive, NULL);

    //Loop infinitely until killed
    while(1);
     
       
    return 0;
}

//Timer interrupt handler
void timer_handler(int signum){
    int send_data;               //adc data stored as a 3 byte int

    //Increment running total and sample number for each analog input
    tot[0] += read_adc(AIN1); // Front
    tot[1] += read_adc(AIN2); // Right
    tot[2] += read_adc(AIN3); // Left
    tot[3] += read_adc(AIN4); // Rear
    s++;

    //If we've taken FREQUENCY samples, take the average, and store data as an int.
    //Send data to bt_listener, if in selfdrive mode send data to tank
    if (s == FREQUENCY -1){
        send_data = (tot[0] / s / 29);
        send_data *= 64;
        send_data += (tot[1] / s / 29);
        send_data *= 64;
        send_data += (tot[2] / s / 29);
        send_data *= 64;
        send_data += (tot[3] / s / 29);

        //Send data to the bt_listener
        union sigval adc_state;
        adc_state.sival_int = send_data;
        sigqueue(pid_bt, SIGUSR1, adc_state); 

        //Send data to tank in self drive mode
        if(selfdriving_flag){
            sigqueue(pid_tank, SIGUSR2, adc_state);
        }

        //Reset Sampling totals
        send_data = 0;
        tot[0] = 0;
        tot[1] = 0;
        tot[2] = 0;
        tot[3] = 0;
    s = 0;

    }
}

//Turns on/off self driving mode
void selfdriving_handler(int signum, siginfo_t * siginfo, void * extra){
    selfdriving_flag = siginfo->si_value.sival_int;
}


//Reads from the adc pin
int read_adc(char *path) {
    //Attempt to open the file; loop until file is found
    FILE *f = NULL;
    while (!!access(path, W_OK));
    f = fopen(path, "r");

    //Buffer to store contents of ADC pin
    char buf[101];

    int b_len = fread(buf, 1, 100, f);
    buf[100] = 0;

    fclose(f);

    return atoi(buf);
}
