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
bool run = true;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


//changes user input string as binary
//Part of SEND logic
char* string_to_binary(char*s, size_t len) {
	if(s==NULL) return 0;
	//size_t len = strlen(s);
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
//Part of SEND logic
void send_manchester(char* data, int size, int PI, int gpio){
	//printf("size of data: %d\n", size);
	for (int i = 0; i < size; i++){
		if (data[i] == 1){
			//printf("1");
			gpio_write(PI, gpio, 0);
			time_sleep(0.005);
			gpio_write(PI, gpio, 1);
			time_sleep(0.005);				
		} else if (data[i] == 0){
			//printf("0");
			gpio_write(PI, gpio, 1);
			time_sleep(0.005);
			gpio_write(PI, gpio, 0);
			time_sleep(0.005);
		} 
	}	
}

void my_callback(int pi, unsigned user_gpio, unsigned level, uint32_t tick){
//	printf("Recieving signals\n");
	

	if (previous_tick == 0) {	
		// solution: set level to zero in receive, do not collect the first bit sent!!!!!!!!!
		// in sender, add a 1 to the beginning of ALL messages
		//hardcoded so first value isn't a crazy high number and break our if statement (detect first level change)
		previous_tick = tick - 1;
		current_tick = tick;
		//hardcoded previous level to be low at 0 for the very first time the system runs
		prev_level = 0;
		current_level = level;
	//	printf("In the if\n");
	} else {
		previous_tick = current_tick;
		current_tick = tick;
		prev_level = current_level;
		current_level = level;
	//	printf("in the else\n");
	}
	time_diff = current_tick - previous_tick;

	if (time_diff >= 10000 && time_diff <= 20000) {
		binary_res[result_counter] = current_level;
		result_counter++;
	//	printf("curr level %d\n", current_level);
	} else if (time_diff >= 1000 && time_diff <= 8000) {
		pair++;

		if (pair == 2) {
			//printf("time diff %d\n", time_diff);
			binary_res[result_counter] =current_level;
			result_counter++;
			pair = 0;
	//	printf("curr level pair 2 %d\n", current_level);
		}
	}
}

void binary_convertor(char *data, int data_length){
	unsigned char binary = 0;
	int bits = 0;
	printf("\nFrom user: ");

	//process 8 bit at a time
	for(int i = 0; i < data_length; i+=8){
		//reset binary for each byte of data (each character/letter)
		binary = 0;
		bits = 0;
	

		//iterate through each bit in a byte	
		for(int j = 0; j < 8; j++){
		binary <<=1;
		if (data[i+j] == 1){
			binary |= 1;
		} 
		bits++;
	}

		if(bits == 8) {
		//	printf("Your character is in binary_converter print: %c\n", binary);
			printf("%c", binary);
		}

	}
	printf("\nSend: ");
	printf("\n");
}


void *send_thread(){
	printf("Starting send thread...\n");
	// Set to the Send Port being used on corresponding pi
	int port1 = 27;
    	int port2 = 25;
    	int port3 = 23;
    	int port4 = 21;
	/*
	 * !CAUTION! Make sure to change 'tx' to appropriate transmitter port
	 */
	int tx = port1;
	int PI = pigpio_start(NULL, NULL);
    	set_mode(PI, tx, PI_OUTPUT);
	
	//prompt user to type
	char user_input[200];
	bool running = true;
	printf("Type your message. Type 'yy' to exit the program. (200 characters max)\n");

	while(running) {
	//char user_input[200];	
		printf("\nSend: ");
		scanf("%200[^\n]%*c", &user_input);

	//user exit code
		if(user_input[0] == 'y' && user_input[1] == 'y') {
			printf("Exiting...\n");
			running = false;
			exit(-1);
		}
	
	//quits if over 200 characters
		if(strlen(user_input) > 200){
			printf("Too long!\n");
			break;
		}

	// turn user input to binary
		char* result = string_to_binary(user_input, strlen(user_input));

	//store result into array
		char result_binary[strlen(result)+2];

	//Setting data header as '1'
		int result_counter = 1;
		result_binary[0] = '1' - '0';
		for (int k = 0; k < strlen(result); k++){
			result_binary[result_counter] = result[k] - '0'; //change to binary so it dont default to ascii
			result_counter += 1;
		}

	//Setting data trailer as '0'
		result_binary[strlen(result)+1] = 0; 
		int size_of = sizeof(result_binary) + 2;

		send_manchester(result_binary, size_of, PI, tx);
		free(result);

	}
}


//HAVE AN ISSUE WITH SPACES! ANY TEXT WITH SPACES WONT PRINT CORRECTLY


void *receive_thread(){
	printf("Starting receive thread...\n");
	// RECEIVE VARIABLES
	int recv1 = 26;
	int recv2 = 24;
	int recv3 = 22;
	int recv4 = 20;
	/*
	 * !CAUTION! Make sure to change 'rx' to appropriate receiver port
	 */
	int rx = recv4;
	int tracker = 0;
	int long_tracker = 0;
	char binary_result[1600];
	int PI = pigpio_start(NULL, NULL);
//	binary_res = malloc(MAX_BIT * sizeof(char));
	// ----------------------

	//run = true;
	while(run) {
		time_sleep(0.005);
		//setting up the callback
		//timestamps the GPIO state change
		pthread_mutex_lock(&mutex);
		callback(PI, rx, EITHER_EDGE, my_callback);
		pthread_mutex_unlock(&mutex);
		

		
		//pthread_mutex_lock(&mutex);
		if (tracker ==20 || long_tracker == 20) {
			//run = false;
			
			memcpy(binary_result, binary_res, result_counter);
			binary_result[result_counter] = '\0';
			
			binary_convertor(binary_result, result_counter - 1);
		//	for (int i = 0; i < result_counter; i++){
		//		printf("BINARY RES: %d\n", binary_result[i]);
		//	}
			//free(binary_res);
			binary_res = realloc(binary_res, MAX_BIT);
			result_counter = 0;
			//tracker = 0;
			//long_tracker = 0;
			//binary_res = realloc(binary_res, MAX_BIT * sizeof(char));

		}
		if (time_diff >= 10000 && time_diff <= 20000){
			tracker = 0;
			long_tracker += 1;
		} else if (time_diff >= 1000 && time_diff <= 8000){
			tracker++;
			long_tracker = 0;
		}

	}
	//pthread_mutex_unlock(&mutex);
	

		/*	
		binary_res = realloc(binary_res, MAX_BIT * sizeof(char));
			printf("Stopping receive after 10 events\n");
			memcpy(binary_result, binary_res, result_counter);
			binary_result[result_counter] = '\0';
			
//			printf("Binary result counter: %d\n", result_counter);
			binary_convertor(binary_result, result_counter);

			if(result_counter>10){
				run = true;
			}
*/
}


int main(){
	// Storing Received Data ...
	binary_res = (char*)malloc(MAX_BIT * sizeof(char));
//	free(binary_res);

	int PI = pigpio_start(NULL, NULL);
	pthread_t thread[2];
	bool chatting = true;

	pthread_create(&thread[0], NULL, send_thread, NULL);
	pthread_create(&thread[1], NULL, receive_thread, NULL);

	pthread_join(thread[0], NULL);
	pthread_join(thread[1], NULL);
	
	free(binary_res);	
	pigpio_stop(PI);
	return 0;
}

