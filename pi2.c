#include <stdio.h>
#include <pigpio_if2.h>

int main(){
	int recv = 26;
	int PI = pigpio_start(NULL, NULL);

	while(1){
		printf("%d\n", gpio_read(PI, recv));
	}
	return 0;
}
