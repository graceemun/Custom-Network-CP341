#include <stdio.h>
#include <pigpiod_if2.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_BIT 1600 //max data is 200 characters = 1600 bytes

uint32_t current_tick = 0;
uint32_t previous_tick;
unsigned current_level = 0;
unsigned prev_level;
uint32_t time_diff;
int result_counter = 0;
int pair = 0;
char *binary_res; //changing it to 12 for now for formatting purposes but should be at max_bit


//starting to work on receiving data
void my_callback(int pi, unsigned user_gpio, unsigned level, uint32_t tick){
	if (previous_tick == 0) {	
		// solution: set level to zero in receive, do not collect the first bit sent!!!!!!!!!
		// in sender, add a 1 to the beginning of ALL messages
		//hardcoded so first value isn't a crazy high number and break our if statement (detect first level change)
		previous_tick = tick - 1;
		current_tick = tick;
		//hardcoded previous level to be low at 0 for the very first time the system runs
		prev_level = 0;
		current_level = level;
	} else {
		previous_tick = current_tick;
		current_tick = tick;
		prev_level = current_level;
		current_level = level;
	}
	time_diff = current_tick - previous_tick;
	if (time_diff >= 190000 && time_diff <= 205000) {
		binary_res[result_counter] =current_level;
		result_counter++;
	} else if (time_diff >= 90000 && time_diff <= 103000) {
		pair++;
		if (pair == 2){
			binary_res[result_counter] =current_level;
			result_counter++;
			pair = 0;
		}
	}

}

void temp_data_convertor(char *data){
	printf("Starting Conversion...\n");
	unsigned char binary = 0;
	int bits = 0;
	for(int i = 0; i < 8; i++){
		binary <<=1;
		if (data[i] == 1){
			binary |= 1;
			printf("Current binary1: %u\n", binary);
		}
			bits++;
		}
	
	if(bits == 8){
		printf("Binary: %c\n", binary);
	}else{
		printf("Incomplete byte\n");

	}
}


int main() {
	binary_res = (char*)malloc(MAX_BIT * sizeof(char));
	int recv = 26;
	int PI = pigpio_start(NULL, NULL);
	int rec_counter = 0;
	int first;
	int second;
	int third;
	int tracker;
	int long_tracker;
	bool run = true;
	char binary_result[1600];
	
	while(run) {
		time_sleep(0.1);
		
		//setting up the callback
		//timestamps the GPIO state change
		callback(PI, recv, EITHER_EDGE, my_callback);
		
		if (tracker == 20 || long_tracker == 20) {
			run = false;
		}

		if (time_diff >= 190000 && time_diff <= 205000){
			tracker = 0;
			long_tracker += 1;
		} else if (time_diff >= 90000 && time_diff <= 103000){
			tracker++;
			long_tracker = 0;
		}
	}


	//	every 8 bits transmitted, it stores them as a string or something and translates them from binary to text (EX: i is a character and i has 8 bits, it reads it, but then if i sent it, I'll read the i first (8 bits) then I'll read the t next (8 bits) and translate those

	printf("Result Counter: %d\n", result_counter);
	// DATA STORED IN binary_result AS INTS
	memcpy(binary_result, binary_res, result_counter);
	binary_result[result_counter] = '\0';
	

//CONVERTING TO TEXT HERE	
	temp_data_convertor(binary_result);
	free(binary_res);

	pigpio_stop(PI);
	return 0;
}




