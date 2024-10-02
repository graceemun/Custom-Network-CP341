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
char *binary_res[12]; //changing it to 12 for now for formatting purposes but should be at max_bit

void binary_to_string(char *input, size_t length){
	//size_t length = strlen(input + 7) / 8;

	char* result = malloc(length + 1);
	//copy 8 bit from input string
	for(size_t i = 0; i < length; i++){
		result[i] = (char)strtol(input, NULL, 2);
	}
	//return *result;
}

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
	if (time_diff >= 900 && time_diff <= 2000) {
		binary_res[result_counter] = (char)current_level;
		result_counter++;
	} else if (time_diff >= 100 && time_diff <= 500) {
		result_counter--;
		binary_res[result_counter] = (char)current_level;
		result_counter++;
	}

	//printf("Prev tick: %u Cur tick: %u Difference: %u\n",previous_tick, current_tick, dif);
	//printf("GPIO %u changed state to %u at tick %u\n", user_gpio, level, tick);
	//printf("Prev lvl: %u Cur lvl: %u\n",prev_level, current_level);
}


int main() {
	int recv = 26;
	int PI = pigpio_start(NULL, NULL);
	int rec_counter = 0;
	int first;
	int second;
	int third;
	int tracker;
	int long_tracker;
	bool run = true;

	while(run) {
		time_sleep(0.1);
		
		//setting up the callback
		//timestamps the GPIO state change
		callback(PI, recv, EITHER_EDGE, my_callback);
		
		
		//printf("Time Passed: %u\n", current_tick - previous_tick);
		//printf("Prev level: %u\n", prev_level);
		//printf("Current level: %u\n", current_level);
		//printf("%d\n", gpio_read(PI, recv));

		if (tracker == 10 || long_tracker == 10) {
			run = false;
		}

		

		if (time_diff >= 900 && time_diff <= 2000){
			tracker = 0;
			long_tracker += 1;
		} else if (time_diff >= 100 && time_diff <= 600){
			tracker++;
			long_tracker = 0;
		}
		//printf("Tracker: %d Long Tracker: %d\n-\n", tracker, long_tracker);
	}


	//	every 8 bits transmitted, it stores them as a string or something and translates them from binary to text (EX: i is a character and i has 8 bits, it reads it, but then if i sent it, I'll read the i first (8 bits) then I'll read the t next (8 bits) and translate those
	
	
	char text_translated[200];
	//size_t iterations = (result_counter) / 8;
	//printf("iterations: %d\n", iterations);
	
	//int text_counter = 0;
	
	//convert each byte to ascii value
	//for(int i = 0; i < iterations; i++){
	//	//text_translated[i] = (char*)malloc(iterations/8);
	//	text_translated[i] = binary_res[i];
	//	//binary_to_string(text_translated);
	//}

	printf("len of binary_res: %c\n", strlen(binary_res));

	for(int w = 0; w < 8; w++) {
		printf("binary: %d\n",&binary_res[w]);
	}

	printf("\n");

	size_t len = strlen(text_translated + 7) / 8;
	binary_to_string(text_translated,len);

	printf("MY OUTPUT: %c\n", text_translated);

		//char *translated_bits[8];
//		for(int h = 0; h < 8; h++) {
//			translated_bits[h] = binary_res[k];
//		}
//		text_translated[text_counter] = binary_to_char(translated_bits);
//		text_counter+=1;
//	}
	
	
	/*	
	for(int i = 0; i <= 8 ; i++) {
		translated_bits[i] = binary_res[i];
		//TRANSLATING LOGIC
		char* translated = binary_to_text(translated_bits);
		printf("Translated text: %d\n", translated);
	}
	*/

	//printf("LENGTH: %d\n", sizeof(binary_res));
/*
	for (int n = 0; n < sizeof(binary_res); n++) {
		printf("DATA: %d\n",binary_res[n]);
	}
	printf("\n");
*/
		
	pigpio_stop(PI);
	return 0;
}




