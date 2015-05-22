// spicat header file

#include <stdint.h>
#include <signal.h>

void nonblock(int state);
int kbhit(void);

typedef struct {
	uint8_t address;
	uint8_t command;
	uint8_t current;
}tx_struct;

typedef struct {
	uint8_t dutycycle;
	uint8_t current_req;
	uint8_t H_current;
	uint8_t L_current;
	uint8_t trans_temp;
	uint8_t motor_temp;
	uint8_t batt_voltage;
	uint8_t status_byte;
}rx_struct;

/* SPI */
int openSPI(void);
rx_struct SPI_exchange_data(tx_struct tx_data);

/* Current control */
//void current_plus (void);
//void current_minus (void);

/* Input decode */
void load_tab(void);
void read_tab(void);

/* Motion control */
void forward(void);
void backward(void);
void motor_stop(void);

/* Signals */
static void handler(int sig, siginfo_t *si, void *uc);
int SPI_timer_init(void);

// Servo
void servo_init(void);
void set_servo(signed char angle);
