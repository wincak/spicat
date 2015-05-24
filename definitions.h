/**********************************************/
/*	definitions.h							  */
/*	author: W.								  */
/*	SPICAT definitions						  */
/**********************************************/


#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define NB_ENABLE 0
#define NB_DISABLE 1

#define CLOCKID CLOCK_REALTIME
#define SIG SIGUSR1

// addresses of controllers
#define PIC1	0x31	// ASCII '1'
#define	PIC2	0x32	// ASCII '2'

// Motor commands
#define FREE_RUN	0b000
#define MOTOR_CW	0b001
#define MOTOR_CCW	0b010
#define REGEN		0b100	// direction checked by controller
#define REGEN_CW	0b101
#define REGEN_CCW	0b110
#define BRAKE		0b111

// Current set
#define CURRENT_MIN		0
#define CURRENT_STEP	10
#define CURRENT_MAX		0xFF
