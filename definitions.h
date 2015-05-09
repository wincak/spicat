/**********************************************/
/*	definitions.h							  */
/*	author: W.								  */
/*	SPICAT definitions						  */
/**********************************************/


#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define NB_ENABLE 0
#define NB_DISABLE 1

// addresses of controllers
#define PIC1	0x31	// ASCII '1'
#define	PIC2	0x32	// ASCII '2'

// Motor commands
#define free_run	0b000
#define motor_CW	0b001
#define motor_CCW	0b010
#define regen_CW	0b101
#define regen_CCW	0b110
#define brake		0b111
