#include <stdio.h>
#include <pigpiod_if2.h>
#include <stdlib.h>
#include <string.h>
//starting to work on receiving data
void my_callback(int pi, unsigned user_gpio, unsigned level, uint32_t tick){
	printf("GPIO %u changed state to %u at tick %u\n", user_gpio, level, tick);
}

int main(){
	int recv = 26;
	int PI = pigpio_start(NULL, NULL);
	
	while(1)
	{
		//callback(PI, recv, 1, my_callback);
		printf("%d\n", gpio_read(PI, recv));
	}
		
	pigpio_stop(PI);
	return 0;
}
