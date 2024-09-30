#include <pigpiod_if2.h>

// just a tester, do not actually use
// trying to get reciever to properly read input

int main() {
	int GPIO;
	int port1 = 27;
	
	int PI = pigpio_start(NULL,NULL);

	while(1) {
		gpio_write(PI, port1, 1);
		time_sleep(0.5);
		gpio_write(PI, port1, 0);
		time_sleep(0.5);
	}
}
