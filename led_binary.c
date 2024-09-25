#include <stdio.h>
#include <pigpiod_if2.h>
#include <string.h>

int main() {
	// initialize all port numbers and gpio
	int GPIO;
	int port1 = 27;
	int port2 = 25;
	int port3 = 23;
	int port4 = 21;

	// change this to accept longer inputs
	int int_size = 4;

	// start accepting stuff
	int PI = pigpio_start(NULL,NULL);

	// set the mode to output
	set_mode(PI,GPIO,PI_OUTPUT);

	// char res is what the user input will be stored in
	// int array will be used to parse through the input
	char res[int_size];
	int array[int_size];

	// user input
	printf("Type a 4-bit binary number: ");
	scanf("%s", res);
	printf("\n");

	// loop through user input
	for(int i = 0; i < int_size; i++) {
		array[i] = res[i] - '0';
		// this lights up the port
		gpio_write(PI,port4,array[i]);
		printf("%d\n",array[i]);
		// must have time sleep for visual effect
		time_sleep(0.5);
	}

	return 0;
}
