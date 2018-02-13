#include "bt_listener.h"

// Terminal interface structure.
struct termios ti;

// Signal handlers for bluetooth drive commands and adc data
struct sigaction saio, adc_receive;

// Wait flag for receiving a bluetooth command
int wait_flag = 1;

// Wait flag for adc data
int wait_adc_flag = 1;

// Flag for self drive mode
int self_drive_flag = 0;

// Stores adc values in 3 bytes to send to controller.
char adc_string[4] = "000\n"; //null terminated

// Process ID of the tank_entry and tank processes.
pid_t pid_tank_entry, pid_tank;
 
// Much of the serial input code taken from http://www.tldp.org/HOWTO/Serial-Programming-HOWTO/x115.html
int main() {
  int f;                     // serial file descriptor
  int len, command;          // stores the length of a command and the command
  unsigned char buffer[100]; // buffer to read from serial port

  // Initialize PIDs to 0
  pid_tank = 0;
  pid_tank_entry = 0;
  
  // signal handler for bluetooth commands (asynchronous retreival)
  saio.sa_handler = signal_handler_IO;
  saio.sa_flags = 0;
  saio.sa_restorer = NULL;
  sigaction(SIGIO,&saio,NULL);

  // set up signal handler for adc data retrieval
  adc_receive.sa_flags = SA_SIGINFO;
  adc_receive.sa_sigaction = &signal_handler_ADC;
  sigemptyset(&adc_receive.sa_mask);
  sigaction(SIGUSR1, &adc_receive, NULL);

  //set up serial port for bluetooth
  enable_UART1();

  f = open(UART1, O_RDWR | O_NOCTTY | O_NONBLOCK);

  if (f < 0) {
    printf("file failed to open\n");
  }
  
  memset (&ti, 0, sizeof(ti));

  if (tcgetattr(f, &ti) != 0) {
    printf("tcgetattr failed\n");
  }

  // set file control options
  fcntl(f, F_SETOWN, getpid()); // enable this process to receive signal
  fcntl(f, F_SETFL, O_ASYNC);   // enable asynchronous execution

  cfsetospeed(&ti, BAUDRATE);
  cfsetispeed(&ti, BAUDRATE);

  // control flags 
  ti.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;

  // input flag
  ti.c_iflag = IGNPAR;

  // local flag
  ti.c_lflag = ICANON;

  // output flag
  ti.c_oflag = 0;

  ti.c_cc[VMIN] = 2;
  ti.c_cc[VTIME] = 0;
  tcflush(f, TCIFLUSH);

  // define all the attributes from termios struct into the file
  if (tcsetattr(f, TCSANOW, &ti) != 0) {
    printf("tcsetattr failed\n");
  }
  printf("finished initializing %d\n",f);
  
  // get PID of tank_entry process and check whether it's valid or not
  // modified from http://stackoverflow.com/questions/8166415/how-to-get-the-pid-of-a-process-in-linux-in-c
  while (!pid_tank_entry) {
    char line[LEN];
    FILE *cmd = popen("pgrep -f tank_entry.exe", "r");
    fgets(line, LEN, cmd);
    pid_tank_entry = strtoul(line, NULL, 10);
    pclose(cmd);

    // check if pid is valid
    int valid = kill(pid_tank_entry, 0);

    if (!!valid) {
      pid_tank_entry = 0;
    }
  }
  
  // Continue to loop, only executes a read when interrupted by SIGIO
  while (1) {
    usleep(100000);  
    
    if (!wait_flag) {    // If new data is available
      // Read from serial port
      len = read(f, buffer, 100);
      buffer[len] = 0;
      
      // Send adc information across bluetooth if new info has been received
      if(!wait_adc_flag) {
        write(f, adc_string, 4);
      }

      // Convert buffer information to 2 byte command
      command = buffer[0] * 256 + buffer[1];

      // Analyze which command to execute.
      if (command == 0xFFFF) {                 // on command
	union sigval tank_state_on;
	tank_state_on.sival_int = 1;                          
	sigqueue(pid_tank_entry, SIGUSR1, tank_state_on);

	// get PID of tank process each time, because it is a child of tank_entry
	// from http://stackoverflow.com/questions/8166415/how-to-get-the-pid-of-a-process-in-linux-in-c
	while (pid_tank == 0){
	  char line[LEN];
	  FILE *cmd = popen("pgrep -f tank.exe", "r");
	  fgets(line, LEN, cmd);
	  pid_tank = strtoul(line, NULL, 10);
	  pclose(cmd);

	  // check if pid is valid
	  int valid = kill(pid_tank, 0);

	  if (!!valid) {
	    pid_tank = 0;
	  }
	}

	//reset bufffer
	memset(buffer, 0, 100);
	
      } else if (command == 0xFF00){         // off command
	union sigval tank_state_off;
	tank_state_off.sival_int = 0;
	sigqueue(pid_tank_entry, SIGUSR1, tank_state_off);

	// ensure that new tank pid is obtained on call of 0xFFFF
	pid_tank = 0;

	//reset bufffer
	memset(buffer, 0, 100);
	
      } else if ((command == 0xFF01 && !self_drive_flag) || (command == 0xFF02)) {   // self drive on/off
	union sigval tank_command;
	tank_command.sival_int = command;
	sigqueue(pid_tank, SIGUSR1, tank_command);

	// set self drive flag
	if (command == 0xFF01) {
	  self_drive_flag = 1;
	} else {
	  self_drive_flag = 0;
	}

	//reset bufffer
	memset(buffer, 0, 100);
	
      } else if (!self_drive_flag) {         // drive command
	union sigval tank_drive;
	tank_drive.sival_int = command;
	sigqueue(pid_tank, SIGUSR1, tank_drive);
	
      } else {                               // disable certain commands in self drive mode
	memset(buffer, 0, 100);
      }

      wait_flag = 1;
      tcflush(f, TCIOFLUSH);
    }
  }
  close(f);
}

// enable serial port
void enable_UART1() {
  // Attempt to open the file; loop until file is found
  FILE *f = NULL;
  while (!!access(DEVPATH, W_OK));
  f =  fopen(DEVPATH, "w");

  fprintf(f, "BB-UART2");
  
  fclose(f);
}

// signal handler for SIGIO from serial port (for asynchronous acquisition)
void signal_handler_IO (int status) {
  wait_flag = 0;
}

// signal handler of the ADC Data retrieval
void signal_handler_ADC(int signum, siginfo_t * siginfo, void * extra ) {
  int adc_data;
  adc_data = siginfo->si_value.sival_int;

  // store data to send over serial port
  adc_string[2] = adc_data % 256;
  adc_data /= 256;
  adc_string[1] = adc_data % 256;
  adc_data /= 256;
  adc_string[0] = adc_data;

  // set wait_adc_flag to 0
  wait_adc_flag = 0;

}
