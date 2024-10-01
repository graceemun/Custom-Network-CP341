#include <stdio.h>
#include <pigpiod_if2.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

uint32_t current_tick = 0;
uint32_t previous_tick;
unsigned current_level = 0;
unsigned prev_level;

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
		previous_tick = tick - 500001;
		//printf("%u\n", previous_tick);
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
	printf("GPIO %u changed state to %u at tick %u\n", user_gpio, level, tick);
}

int main() {
	int recv = 26;
	int PI = pigpio_start(NULL, NULL);
	uint32_t time = 0;
	char received[3200];
	int rec_counter = 0;
	int first;
	int second;
	int third;
	bool run = true;
	//char received_data[];
	while(run) {
		time_sleep(0.5);
		//setting up the callback
		//timestamps the GPIO state change
		time = current_tick - previous_tick;
		callback(PI, recv, EITHER_EDGE, my_callback);
		//printf("Time Passed: %u\n", current_tick - previous_tick);
		//printf("Prev level: %u\n", prev_level);
		//printf("Current level: %u\n", current_level);
		//printf("%d\n", gpio_read(PI, recv));

		
		if (time >= 500000 && time <= 505000) {
			if (prev_level == 0 && current_level == 1) {
				third = second;
				second = first;
				first = 1;
				printf("1 RECEIVED\n");
			} else if (prev_level == 1 && current_level == 0) {
				third = second;
				second = first;
				first = 0;
				printf("0 RECEIVED\n");
			}

			if (first == second && second == third && third == first) {
				run = false;
			} else {
				received[rec_counter] += third;
				rec_counter += 1;
				received[rec_counter] += second;
				rec_counter += 1;
			}

		} else {
			printf("-\n");
		}

	}

	int rec_leng = strlen(received);

	for (int m = 0; m < rec_leng; m++) {
		printf("%d", &received[m]);
	}
	printf("\n");
		
	pigpio_stop(PI);
	return 0;
}


