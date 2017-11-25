#include <wiringPiI2C.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

int fd;
int acclX, acclY, acclZ;

double acclX_scaled, acclY_scaled, acclZ_scaled;
double total=0;
double n=0;
double last=1;
double magnitude=0;
double change=0;
long mymagnitude,mymagnitude_y;
int mode;
char state='A';
char *throwCommand(char *message) {
	char *command = malloc(300);
	sprintf(command, "echo '%s'", message);
	return command;
}
int read_word_2c(int addr)
{
	int val;
	val = wiringPiI2CReadReg8(fd, addr);
	val = val << 8;
	val += wiringPiI2CReadReg8(fd, addr+1);
	if (val >= 0x8000)
	val = -(65536 - val);
	return val;
}

double dist(double a, double b)
{
	return sqrt((a*a) + (b*b));
}

double get_y_rotation(double x, double y, double z)
{
	double radians;
	radians = atan2(x, dist(y, z));
	return -(radians * (180.0 / 3.141592));
}

double get_x_rotation(double x, double y, double z)
{
	double radians;
	radians = atan2(y, dist(x, z));
	return (radians * (180.0 / 3.141592));
}


int readSensor()
{
	fd = wiringPiI2CSetup (0x68);
	wiringPiI2CWriteReg8 (fd,0x6B,0x00);//disable sleep mode
	//printf("set 0x6B=%X\n",wiringPiI2CReadReg8 (fd,0x6B));
	int n=0;
	int test_on=0;
	int test_off = 0;
	while(n<20){
		acclX = read_word_2c(0x3B);
		acclY = read_word_2c(0x3D);
		acclX_scaled = acclX / 16384.0;
		acclY_scaled = acclY / 16384.0;
		//printf("My acclX_scaled: %f\n", acclX_scaled);
		mymagnitude=(acclX_scaled*1000000);
		mymagnitude_y = (acclY_scaled *1000000);
		printf("Magnitude_x=%d Magnitude_y = %d\n",mymagnitude, mymagnitude_y);
		if(mymagnitude > -15000 && mymagnitude< 16000)
		{
			test_off++;
			//printf("Device off\n");
			//mode = 0;
		}
		else
		{
			test_on++;
			//printf("Device on\n");
			//mode=1;
		}
		n++;
	}
	if(test_on >= test_off){
		printf("test_on: %d	test_off: %d\n", test_on, test_off);
		printf("Device on\n");
		mode=1;
	}
	else{
		printf("test_on: %d	test_off: %d\n", test_on, test_off);
		printf("Device off\n");
		mode = 0;
	}
	delay(200);
}



int main()
{

	printf("Reading mode from main=%d\n",mode);
	int check =1;
	while(check)
	{
		//readSensor();
		 switch(state)
		{
		case 'A': //off
			{


			printf("CASE A, OFF\n");
			readSensor();
			if (mode==1) //if it detects that washing machine is vibrating, mode=1. Go to State B.
				{
				state='B';
				}
			else{ 	//continue the same loop thinking it is off
				state='A';
			}
			break;
			}
		case 'B':			// ON MODE
			{
			printf("CASE B, ON\n");
			readSensor();
			if(mode==0)		// checks if its off temporarily, then checks for a pause and goes to pause mode.
				{
				state='C';
				}
			else{state='B';} // continues in the same loop thinking it is on
			break;
			}
		case 'C':			// PAUSE MODE
			{

			printf("CASE C, PAUSE\n");


			//readSensor();
			delay(10000);		//Runs the timer for 10 seconds
			readSensor();
			if(mode==1)		//if after 10 seconds the mode =1, then it means the machine has turned on. so go to state B, which is ON mode.
				{

				state='B';
				}
			else{state='D';}	//if after 10 seconds , no vibration is detected then the mode=0, and it means the machine has stopped. so go to step D.
			break;
			}
		case 'D': 	//END MODE, send email.
			{
                        printf("CASE D: END\n");
                        system("python sendingemail.py");

            }
            check=1;
            break;

		}
	}
	return 0;
}



























