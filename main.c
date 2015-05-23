/********************************************************************/
/*	SPICAT							    */
/*  Author: Wincak						    */
/*								    */
/*	Reads input from stdin and exchanges data over SPI	    */
/*	according to commands. 					    */
/*	Purpose: control of SPI peripherals in regular intervals    */
/*								    */
/********************************************************************/

// If using timers, compile with -lrt linker option

// Usage with netcat (nc)
// HOST:	$ mkfifo fifo
//			$ cat fifo | ./a.out -i 2>&1 | nc -l 1234 -k > fifo
// USER:	$ nc <host_ip> 1234


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <termios.h>
#include <signal.h>
#include <time.h>

#include "functions.h"
#include "definitions.h"

// SPI variables
static const char *device = "/dev/spidev0.0";
static uint8_t mode;
static uint8_t bits = 8;
//static uint32_t speed = 245000;	// 40 MHz ok
static uint32_t speed = 50000;		// for 10 MHz PIC clock
static uint16_t delay;

// Signal timer variables
timer_t timerid;

struct sigevent sev;
struct itimerspec its;
long long freq_nanosecs;
sigset_t mask;
struct sigaction sa;

int SPI_timer;     // timer overflow notification


// SPI transfer structures
tx_struct tx_left, tx_right;
rx_struct rx_left, rx_right;

// STDIN input
signed int rx_tab[4];

char ch;
int fd;

// Load char data from stdin (netcat)
void load_tab (void)
{
	char position;
	signed char ret;

	for (position = 0; position <=3; position++)
	{
		ret = getchar();
		rx_tab[position] = ret;
	}
	fprintf(stderr,"\n");
}

// Read input data and calculate control values
void read_tab (void)
{
	// IGNORING X,Z axes and Button!
	signed char req_power;

	req_power = rx_tab[1];	// Y axis
	if(req_power == 0) motor_stop();
	else if (req_power > 0 && req_power <=100)
	{
		forward();
	}
	else if (req_power < 0 && req_power >=-100)
	{
		backward();
	}
	else {
		motor_stop();	// Error
		return;
	}

	// Steer
	signed char steer_angle;
	steer_angle = rx_tab[0]; // X axis
	set_servo(steer_angle);
	printf("STEER: %i\n",steer_angle);

	tx_right.current = abs(req_power)*(CURRENT_MAX/100);
	tx_left.current = abs(req_power)*(CURRENT_MAX/100);
	fflush(stdout);

}


int main (void) {
	// Init SPI TX values
	tx_right.address = PIC1;
	tx_right.command = 0;
	tx_right.current= 0;

	tx_left.address = PIC2;
	tx_right.command = 0;
	tx_right.current= 0;

	// Variables to convert/calculate measured current
	short right_current_meas, left_current_meas;

	fprintf(stdout,"Hello from raspberry!!\n");

	openSPI();

	nonblock(NB_ENABLE);
	
	//SPI_timer_init();	// Signals cause instant connection loss!! but why?
	//SPI_timer = 0;	// clear timer flag
	//sigprocmask(SIG_BLOCK, &mask, NULL);

	servo_init();

	printf("Start...\n");
	fflush(stdout);	

/* main loop */

  	while(1){

		// Read input
		ch = getchar();
		if (ch == 255) {
			printf("Connection closed\n"); return(EXIT_SUCCESS);
		}

		if (ch == 250){
			load_tab();
			read_tab();
		}

		// Clear data from input buffer
		int c;
		while ((c = getchar()) != '\n' && c != EOF) fprintf(stderr, "clr");

		rx_right = SPI_exchange_data(tx_right);
		rx_left = SPI_exchange_data(tx_left);

		// 2x char -> short
		right_current_meas = (rx_right.H_current<<8) + rx_right.L_current;
		left_current_meas = (rx_left.H_current<<8) + rx_left.L_current;

		//printf("trans-temp: %i \n",rx_right.trans_temp); fflush(stdout);
		if(rx_right.status_byte){
			fprintf(stdout,"\rRGHT-> ONLINE  ");
		}
		else fprintf(stdout,"\rRGHT-> ERROR!  ");

		printf("MODE: %i REQ: %.1f MEAS: %.1f DTC: %i TEMP: %i BATT: %.1f            \n",
			tx_right.command, ((float)rx_right.current_req)/10, ((float)right_current_meas)/10, \
			 rx_right.dutycycle, rx_right.trans_temp, ((float)rx_right.batt_voltage)/10);

		if(rx_left.status_byte){
			fprintf(stdout,"\rLEFT-> ONLINE  ");
		}
		else fprintf(stdout,"\rLEFT-> ERROR!  ");

		printf("MODE: %i REQ: %.1f MEAS: %.1f DTC: %i TEMP: %i BATT: %.1f           \n",
			tx_left.command, ((float)rx_left.current_req)/10, ((float)left_current_meas)/10, \
			 rx_left.dutycycle, rx_left.trans_temp, ((float)rx_left.batt_voltage)/10);

		fflush(stdout);

		//usleep(100000);	// original value
		usleep(10000);	// just testing signals

  	}

	close(fd);

   	nonblock(NB_DISABLE);

	printf("\nExiting...\n\n");

	return 0;
}

void forward(void){
	tx_right.command = MOTOR_CW;
	tx_left.command = MOTOR_CCW;

	tx_right.current = CURRENT_MIN;
	tx_left.current = CURRENT_MIN;

	return;
}

void backward(void){
	tx_right.command = MOTOR_CCW;
	tx_left.command = MOTOR_CW;

	tx_right.current = CURRENT_MIN;
	tx_left.current = CURRENT_MIN;

	return;
}

void motor_stop(void){
	tx_right.command = FREE_RUN;
	tx_left.command = FREE_RUN;

	tx_right.current = 0;
	tx_left.current = 0;

	return;
}


rx_struct SPI_exchange_data(tx_struct tx_data){
	int ret;
	rx_struct received;
	uint8_t tx[] = {
		0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};
	uint8_t rx[ARRAY_SIZE(tx)] = {0, };

	tx[0] = tx_data.address;
	tx[1] = tx_data.current;
	tx[2] = tx_data.command;


	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = ARRAY_SIZE(tx),
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	// Transfer and receive data
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1){
		printf("can't send spi message\n");
		fflush(stdout);
	}

	// Evaluate
	received.dutycycle = rx[3];
	received.current_req = rx[4];
	received.H_current = rx[5];
	received.L_current = rx[6];
	received.trans_temp = rx[7];
	received.motor_temp = rx[8];
	received.batt_voltage = rx[9];
	received.status_byte = rx[10];

	return received;
}



// Make stdin reading non-blocking
void nonblock(int state)
{
    struct termios ttystate;
    //get the terminal state
    tcgetattr(STDIN_FILENO, &ttystate);

    if (state==NB_ENABLE)
    {
        //turn off canonical mode
        ttystate.c_lflag &= ~ICANON;
        //minimum of number input read.
        ttystate.c_cc[VMIN] = 1;
    }
    else if (state==NB_DISABLE)
    {
        //turn on canonical mode
        ttystate.c_lflag |= ICANON;
    }
    //set the terminal attributes.
    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);

}

// Check for input
int kbhit()
{
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;  tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds); //STDIN_FILENO is 0
    select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &fds);
}


// Well.... open SPI?
int openSPI(void){
	int ret;
	fd = open(device, O_RDWR);
	if (fd < 0)
		printf("can't open device");

	/*
	 * spi mode
	 */
	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
		printf("can't set spi mode");

	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
		printf("can't get spi mode");

	/*
	 * bits per word
	 */
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		printf("can't set bits per word");

	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		printf("can't get bits per word");

	/*
	 * max speed hz
	 */
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		printf("can't set max speed hz");

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		printf("can't get max speed hz");

	printf("spi mode: %d\n", mode);
	printf("bits per word: %d\n", bits);
	printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);

	return;
}

// Signal timer handler
static void handler(int sig, siginfo_t *si, void *uc)
{
	if(si->si_value.sival_ptr != &timerid){
		fprintf(stderr, "Stray signal\n");	// not safe to use
	} else {
		SPI_timer = 1;
		timer_settime(timerid, 0, &its, NULL);
		fprintf(stderr, "tmr overflow\n");
	}

}

int SPI_timer_init(void)
{
        sa.sa_flags = SA_SIGINFO;
        sa.sa_sigaction = handler;
        sigemptyset(&sa.sa_mask);
        sigaction(SIG, &sa, NULL);

        sev.sigev_notify = SIGEV_SIGNAL;
        sev.sigev_signo = SIG;
        sev.sigev_value.sival_ptr = &timerid;
        timer_create(CLOCKID, &sev, &timerid);

        // Start the timer
        its.it_value.tv_sec = 1;
        its.it_value.tv_nsec = 100000000; // 100ms
        its.it_interval.tv_sec = its.it_value.tv_sec;
        its.it_interval.tv_nsec = its.it_value.tv_nsec;

        // Activate timer
        timer_settime(timerid, 0, &its, NULL);

        return(0);
}
