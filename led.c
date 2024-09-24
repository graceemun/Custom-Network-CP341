#include <stdio.h>
#include <pigpiod_if2.h>
// gcc led.c -lpigpiod_if2
// ./a.out

int main(){
	int GPIO
	int port1 = 27;
	int port2 = 25;
	int port3 = 23;
	int port4 = 21;

	int PI = pigpio_start(NULL,NULL);
	
	//iterates through all GPIO to see which ports are connected to which GPIO numbers
	
	//for(GPIO = 1; GPIO<=39; GPIO++){
	set_mode(PI, 27, PI_OUTPUT);
	
	for(int i = 0; i <= 10; i++){
		gpio_write(PI,27,1);
		time_sleep(1);
		gpio_write(PI, 27,0);
		time_sleep(1);
		printf("BLINKING: %d\n", i);
	}
	
	return 0;

}
