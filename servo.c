// Servo control functions for GPIO 18
// Using rpi-pwm HW PWM driver
// Author: Wincak

#include <stdio.h>
#include <stdint.h>
#include <math.h>

// TODO: rewrite using defines
#define SERVO_MAX	23	
#define SERVO_MIN	9
#define	SERVO_MID	16

FILE *factive, *fservo, *fmode;

void servo_init(void){
        factive=fopen("/sys/class/rpi-pwm/pwm0/active", "w");
        fservo=fopen("/sys/class/rpi-pwm/pwm0/servo", "w");
        fmode=fopen("/sys/class/rpi-pwm/pwm0/mode", "w");

        fputs("servo", fmode);
        fputs("1", factive);
        fputs("16", fservo);

        fclose(fmode);
        fclose(factive);
        fclose(fservo);

        fservo=fopen("/sys/class/rpi-pwm/pwm0/servo", "w");
        fputs("16", fservo);
        fclose(fservo);

}

// Steer front wheel angle by servo
// angle defined as -100..0..100
void set_servo(signed char angle){
	fservo=fopen("/sys/class/rpi-pwm/pwm0/servo", "w");
	int servo_step;
	float servo_max = SERVO_MAX;
	float servo_mid = SERVO_MID;
	float servo_min = SERVO_MIN;

	if(angle == 0)
	{
		fputs("16", fservo);
	}
	else if (angle > 0 && angle <=100)
	{
		//steer left
		servo_step = abs(angle);

		servo_step = (int)(servo_step*((servo_max-servo_mid)/100));

		servo_step = SERVO_MID + servo_step;

		fprintf(fservo,"%i",servo_step);
	}
	else if (angle < 0 && angle >=-100)
	{
		//steer right
		servo_step = abs(angle);

		servo_step = (int)(servo_step*((servo_max-servo_mid)/100));

		servo_step = SERVO_MID-servo_step;
		
		fprintf(fservo,"%i",servo_step);
	}
	else fputs("0", fservo);
	
	fflush(fservo);
	fclose(fservo);

	return;
}
