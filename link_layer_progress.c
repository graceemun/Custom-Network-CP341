#include <stdio.h>
#include <pigpiod_if2.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

#define MAX_BIT 1600 //max data is 200 characters = 1600 bytes

uint32_t current_tick = 0;
uint32_t previous_tick;
unsigned current_level = 0;
unsigned prev_level;
uint32_t time_diff;
int result_counter = 0;
int pair = 0;
char *binary_res; //changing it to 12 for now for formatting purposes but should be at max_bit

//changes user input string as binary
char* string_to_binary(char*s) {
	if(s==NULL) return 0;
	size_t len = strlen(s);
	char *binary = malloc(len*8 + 1);
	binary[0] = '\0';
	for(size_t i = 0; i < len; i++) {
		char ch = s[i];
		for(int j = 7; j >= 0; j--) {
			if(ch & (1 << j)) {
				strcat(binary, "1");
			} else {
				strcat(binary, "0");
			}
		}
	}
	return binary;
}

//bit rate defined
//send manchester logic
void send_manchester(char* data, int size, int PI, int gpio){
	//printf("size of data: %d\n", size);
	for (int i = 0; i < size; i++){
		if (data[i] == 1){
			printf("1");
			gpio_write(PI, gpio, 0);
			time_sleep(0.1);
			gpio_write(PI, gpio, 1);
			time_sleep(0.1);				
		} else if (data[i] == 0){
			printf("0");
			gpio_write(PI, gpio, 1);
			time_sleep(0.1);
			gpio_write(PI, gpio, 0);
			time_sleep(0.1);
		} 
	}	
}

void my_callback(int pi, unsigned user_gpio, unsigned level, uint32_t tick){
	printf("Recieving signals\n");
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

	if (time_diff >= 18000 && time_diff <= 23000) {
		binary_res[result_counter] =current_level;
		result_counter++;
	} else if (time_diff >= 8000 && time_diff <= 10600) {
		pair++;
		if (pair == 2) {
			binary_res[result_counter] =current_level;
			result_counter++;
			pair = 0;
		}
	}

}

void binary_convertor(char *data, int data_length){
	unsigned char binary = 0;
	int bits = 0;

	//process 8 bit at a time
	for(int i = 0; i < data_length; i+=8){
		//reset binary for each byte of data (each character/letter)
		binary = 0;
		bits = 0;
		
		for(int j = 0; j < 8 && (i+j)<data_length;j++){
		binary <<=1;
		if (data[i+j] == 1){
			binary |= 1;
		}
			bits++;
		}
	
		if(bits == 8){
			printf("%c", binary);
		}
	}
}


//WILLA LOOK HERE!!!!!!
//W
//I
//L
//L
//A
//current issue is with send thread. When you run it and press any character 
//(yes including only temporary exit character "e", which doesn't exit as intended unfortunately) 
//it spits out the correct binary, but with a bunch of extra stuff attached to the end.
// EX: "i" in binary is 01101001 and now its 0110100100001010
//  so each thing has an extra 00001010 at the end of it, which messes up the binary... 
//  thats what I plan on fixing unless you get to it first.
//  Recieve can be tested on both ends if you make an exact copy of this code and put it on the other computer
//  I am available to call if you need any explanation!!!!
void *send_thread(){
	printf("Starting send thread...\n");
	// Set to the Send Port being used on corresponding pi
	int port1 = 27;
    	int port2 = 25;
    	int port3 = 23;
    	int port4 = 21;

	int PI = pigpio_start(NULL, NULL);

	// MAKE SURE CORRECT PORT IS BEING SET
    	set_mode(PI, port1, PI_OUTPUT);
	
	int message = 0;

	
	//prompt user to type
	char user_input[200];

	while(1){
	//char user_input[200];	
	printf("Type your message. Type 'e' to exit the program. (200 characters max): \n");
//	scanf("%[^\n]", &user_input);

	fgets(user_input, sizeof(user_input), stdin);
	//error detection for reads user input
	//if (fgets(user_input, sizeof(user_input), stdin)==NULL){
	//	printf("Error reading input\n");
//		continue;
//	}


	//user exit code
	if(strcmp(user_input, "e")==0){
		printf("Exiting...\n");
		exit(-1);
		}
	
	//quits if over 200 characters
	if(strlen(user_input) > 200){
		printf("Too long!\n");
		break;
	}
	// turn user input to binary
	char* result = string_to_binary(user_input);
	printf("Binary user result: %s\n", result);

	//store result into array
	char result_binary[strlen(result)+2];

	//Setting data header as '1'
	int result_counter = 1;
	result_binary[0] = '1' - '0';
	for (int k = 0; k < strlen(result); k++){
		result_binary[result_counter] = result[k] - '0'; //change to binary so it dont default to ascii
		//printf("Stored binary is: %d\n", result_binary[result_counter]);
		result_counter += 1;
	}

	//Setting data trailer as '0'
	result_binary[strlen(result)+1] = 0; 
	int size_of = sizeof(result_binary) + 2;

	//printf("size of result_binary: %d\n", sizeof(result_binary));
	send_manchester(result_binary, size_of, PI, port1);
	free(result);

	}
	printf("Thread finished??????\n");
}

void *receive_thread(){
	printf("Starting receive thread...\n");
	// RECEIVE VARIABLES
	int recv = 20;
	int rec_counter = 0;
	int tracker;
	int long_tracker;
	bool run = true;
	char binary_result[1600];
	int PI = pigpio_start(NULL, NULL);
	// ----------------------

	while(run) {
		time_sleep(0.01);
		
		//setting up the callback
		//timestamps the GPIO state change
		callback(PI, recv, EITHER_EDGE, my_callback);
		if (tracker == 20 || long_tracker == 20) {
			run = false;
		}
		if (time_diff >= 18000 && time_diff <= 23000){
			tracker = 0;
			long_tracker += 1;
		} else if (time_diff >= 8000 && time_diff <= 10600){
			tracker++;
			long_tracker = 0;
		}
	}

	// Storing Received Data ...
	binary_res = (char*)malloc(MAX_BIT * sizeof(char));
	// DATA STORED IN binary_result AS INTS
	memcpy(binary_result, binary_res, result_counter);
	binary_result[result_counter] = '\0';
	
	//CONVERTING TO TEXT HERE	
	binary_convertor(binary_result, result_counter);
	free(binary_res);
}


int main(){
	int PI = pigpio_start(NULL, NULL);
	pthread_t thread[2];
	pthread_create(&thread[1], NULL, receive_thread, NULL);
	pthread_create(&thread[0], NULL, send_thread, NULL);
	pthread_join(thread[0], NULL);
	pthread_join(thread[1], NULL);
	
	pigpio_stop(PI);
	return 0;
}




