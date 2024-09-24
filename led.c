#include <stdio.h>
#include <pigpiod_if2.h>
// gcc led.c -lpigpiod_if2
// ./a.out
int main(){
	int PI = pigpio_start(NULL,NULL);
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
