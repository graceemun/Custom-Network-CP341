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
int repeat_counter = 0;
char binary_res[12]; //changing it to 12 for now for formatting purposes but should be at max_bit
//char *daita_storer = malloc (MAX_BIT + 2);
/*
char* man_to_binary(char* result) {
	char* binary[1600];
	int result_counter = 0;
	int len = strlen(result);
	char cur;
	char next;

	for(int i = 0; i < len; i++) {
		if(i % 2 == 0) {
			cur = result[i];
			next = result[i+1];
			if (cur == 1 && next == 0) {
				result[result_counter] = 0;
			}  else if (cur == 0 && next == 1) {
				result[result_counter] = 1;
			} else {
				i = len; // break the loop entirely
			}

			result_counter += 1;
			i += 1;
		}
	}

	return binary;
}

char* binary_to_text(char* answer) {
	char* text[200];
	int ans_len = strlen(answer);
	//int sym_count = answer / 8 + 1;
}
*/

//starting to work on receiving data
void my_callback(int pi, unsigned user_gpio, unsigned level, uint32_t tick){
	if (previous_tick == 0) {	
		// solution: set level to zero in receive, do not collect the first bit sent!!!!!!!!!
		// in sender, add a 1 to the beginning of ALL messages
		//hardcoded so first value isn't a crazy high number and break our if statement (detect first level change)
		previous_tick = tick - 1;
		//printf("Prev tick: %u Cur tick: %u\n", previous_tick, current_tick);
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
	uint32_t dif = current_tick - previous_tick;
	time_diff = current_tick - previous_tick;
	if (time_diff >= 1000000 && time_diff <= 1003000) {
		binary_res[result_counter] = (char)current_level;
		result_counter += 1;

	} else if (time_diff >= 500000 && time_diff <= 502000) {
		repeat_counter += 1;
		if (repeat_counter % 2 == 0 && repeat_counter != 0) {
			binary_res[result_counter] = (char)current_level;
			result_counter += 1;
			repeat_counter = 0;
		}
	}




	printf("Prev tick: %u Cur tick: %u Difference: %u\n",previous_tick, current_tick, dif);
	//printf("GPIO %u changed state to %u at tick %u\n", user_gpio, level, tick);
	printf("Prev lvl: %u Cur lvl: %u\n",prev_level, current_level);
}


int main() {
	int recv = 26;
	int PI = pigpio_start(NULL, NULL);
	//uint32_t time = 0;
	char received[3200];
	int rec_counter = 0;
	int first;
	int second;
	int third;
	int tracker;
	int long_tracker;
	bool run = true;
	//int data_counter =0;
	//char received_data[12];
	//char received_data[];
	while(run) {
		time_sleep(0.5);
		//setting up the callback
		//timestamps the GPIO state change
		//time = current_tick - previous_tick;
		callback(PI, recv, EITHER_EDGE, my_callback);
		//time = current_tick - previous_tick;
		//printf("Time Passed: %u\n", current_tick - previous_tick);
		//printf("Prev level: %u\n", prev_level);
		//printf("Current level: %u\n", current_level);
		//printf("%d\n", gpio_read(PI, recv));

		if (tracker == 9 || long_tracker == 9) {
			run = false;
		}

		if (time_diff <= 1003000 && time_diff >= 1000000){
			tracker = 0;
			long_tracker += 1;
			//received_data[data_counter] = 4;
			//data_counter += 1;
		}else if (time_diff <= 502000 && time_diff >= 500000){
			tracker++;
			long_tracker = 0;
			if (tracker == 2) {
				tracker = 0;
				//received_data[data_counter] = 6;
				//data_counter += 1;
			}
		}
		
		//printf("Tracker: %d Long Tracker: %d\n-\n", tracker, long_tracker);

			//if (first == second && second == third && third == first) {
			//	run = false;
			//} else {
			/*	received[rec_counter] += third;
				rec_counter += 1;
				received[rec_counter] += second;
				rec_counter += 1;
			} */

	//	}

	}
//	for(int i = 0; i < sizeof(binary_res); i +=8){


	//printf("LENGTH: %d\n", sizeof(binary_res));
	for (int n = 0; n < sizeof(binary_res); n++) {
		printf("DATA: %d\n",binary_res[n]);
	}
	printf("\n");

	//int rec_leng = strlen(received);

	//for (int m = 0; m < rec_leng; m++) {
	//	printf("%d", &received[m]);
	//}
	//printf("\n");
		
	pigpio_stop(PI);
	return 0;
}


