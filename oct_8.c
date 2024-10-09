#include <stdio.h>
#include <pigpiod_if2.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

#define MAX_BIT 1600 //max data is 200 characters = 1600 bytes
//#define MY_ADDRESSS 00001; //make sure each pi has a unique address
#define ADDRESS_SIZE 5

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
char my_username[50];
int *receive_address;
int my_address[5] = {1,0,0,0,1};
int recipient_address[5] = {0,0,0,0,1};
int *sender_address;
//source counter
int src_counter = 0;
//destination counter
int dest_counter = 0;
bool what_destination = false;

//SEND PORTS
int port1 = 27;
int port2 = 25;
int port3 = 23;
int port4 = 21;
// RECEIVE PORTS
int recv1 = 26;
int recv2 = 24;
int recv3 = 22;
int recv4 = 20;

//prompt user to type username and store it....
char* create_user(){
	printf("Please type in your desired username. YOUR USERNAME CANNOT BE CHANGED OR EXCEED 50 CHARACTERS.\n");
	scanf("%200[^\n]%*c", &my_username);
	printf("This is your username: %s\n", my_username);
	char user_id[5];
	srand(time(0));
	for (int i = 0; i < 5; i++) {
		int r = (rand() % 2);
		printf("%d",r);
		user_id[i] = r;
	}
	printf("\n");
	//return &my_username;
}

// if the destination 
bool check_destination(int *destination_address){
	//I know who im connected to (FAKE CODE NOW AHHHHHH)
	
	if(memcmp(receive_address, my_address, sizeof(my_address))==0){
		//printf("My address\n");
		return true;
	}else if(memcmp(receive_address, my_address, sizeof(my_address)) != 0){ //&& memcmp(destination_address, my_address, sizeof(destination_address)) != 0){
		printf("Dont know who this is, send to the next one\n");
		return false;
	}
}


//create static addresses
char* create_address(){
	//if listening from port, address = 1001
}

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
		printf("Start: %d",current_level);
	} else {
		previous_tick = current_tick;
		current_tick = tick;
		prev_level = current_level;
		current_level = level;
	}
	time_diff = current_tick - previous_tick;

	if (time_diff >= 10000 && time_diff <= 20000) {
		// want to read and store the first 5 bits as address from source
		if (dest_counter < 5) { 
			receive_address[dest_counter] = current_level;
			printf("%d", receive_address[dest_counter]);
			dest_counter ++;
			//printf("%d", current_level);
		} else if (dest_counter == 5) {
			what_destination = check_destination(receive_address);
			if (what_destination){
				binary_res[dest_counter] = current_level;
				dest_counter++;
				printf("%d", current_level);
			}
			dest_counter++;
		//	printf("curr level %d\n", current_level);
		} else if(dest_counter == 5 && src_counter < 5) {
			sender_address[src_counter] = current_level;
			src_counter++;
			printf("%d", current_level);
		} else if(src_counter == 5) {
			what_destination = check_destination(receive_address);
			if(what_destination) {
				binary_res[result_counter] = current_level;
				printf("%d",binary_res[result_counter]);
				result_counter++;
			}

		}
	} else if (time_diff >= 1000 && time_diff <= 8000) {
		pair++;
		if (pair == 2) {
			if (dest_counter < 5){
				receive_address[dest_counter] = current_level;
				printf("%d", current_level);
				dest_counter ++;
				pair = 0;	
			}else if (dest_counter == 5){
				//printf("time diff %d\n", time_diff);
				what_destination = check_destination(receive_address);
				if (what_destination){
					binary_res[result_counter] = current_level;
					dest_counter++;
					pair = 0;
					printf("%d", current_level);
				}
				dest_counter++;
			} else if (dest_counter == 5 && src_counter < 5) {
				sender_address[src_counter] = current_level;
				src_counter++;
				printf("%d", current_level);
				pair = 0;
			} else if (src_counter == 5) {
				if(what_destination) {
					binary_res[result_counter] = current_level;
					result_counter++;
					printf("%d", current_level);
					pair = 0;
				}
			}
		}
	}
}


void binary_convertor(char *data, int data_length) {
	unsigned char binary = 0;
	int bits = 0;
	char *user;
	int grace_address[5] = {0,0,0,0,1};
	
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
	printf("\nSend from %s", my_username);
	printf("\n");
}


void *send_thread(){
	printf("Starting send thread...\n");
	/*
	 * !CAUTION! Make sure to change 'tx' to appropriate transmitter port
	 */
	int tx = port4;
	int PI = pigpio_start(NULL, NULL);
    	set_mode(PI, tx, PI_OUTPUT);
	
	//prompt user to type
	char user_input[200];
	bool running = true;
	printf("Type your message. Type 'yy' to exit the program. (200 characters max)\n");

	while(running) {
	//char user_input[200];	
		printf("\nSend from %s", my_username);
		printf("\n");
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

		//store recipient address first
		for(int i = 0; i < 5; i++){
			result_binary[result_counter] = recipient_address[i];
			result_counter +=1;
		}
		//DEBUG PRINT
		/*
		for(int i = 1; i < 6; i++){
			printf("Destination: %d\n", result_binary[i]);
		}
		*/

		// store my address
		for(int j = 0; j < 5; j++) {
			result_binary[result_counter] = my_address[j];
			result_counter += 1;
		}
		
		//DEBUGGING
		/*
		for(int i = 6; i < 11; i++){
			printf("Source: %d\n", result_binary[i]);
		}
		*/

		for (int k = 0; k < strlen(result); k++){
			result_binary[result_counter] = result[k] - '0'; //change to binary so it dont default to ascii
			result_counter += 1;
		}

		//DEBUGGING
		/*
		for(int i = 11; i < result_counter; i++){
			printf("Message: %d\n", result_binary[i]);
		}
		*/

	//Setting data trailer as '0'
		result_binary[strlen(result)+1+10] = 0; 
		result_counter += 1;
		//int size_of = sizeof(result_binary) + 2;

		
		for (int p = 0; p < result_counter; p++){
			printf("SENT BITS: %d\n", result_binary[p]);
		}
		


		send_manchester(result_binary, result_counter, PI, tx);
		free(result);

	}
}



void *receive_thread(){
	printf("Starting receive thread...\n");
	/*
	 * !CAUTION! Make sure to change 'rx' to appropriate receiver port
	 */
	int rx = recv1;
	int tracker = 0;
	int long_tracker = 0;
	char binary_result[1600];
	int PI = pigpio_start(NULL, NULL);
	// ----------------------

	while(run) {
		time_sleep(0.005);
		//setting up the callback
		//timestamps the GPIO state change
		pthread_mutex_lock(&mutex);
		
		//if the user id array is not empty, execute the following, else don't do anything
		callback(PI, rx, EITHER_EDGE, my_callback);
		pthread_mutex_unlock(&mutex);
		
			
		//pthread_mutex_lock(&mutex);
		if (tracker == 20 || long_tracker == 20) {
			//run = false;

			memcpy(binary_result, binary_res, result_counter);
			binary_result[result_counter] = '\0';
			
			printf("%d", binary_result[0]);
			
			/*
			 * To everyone:
			 * we are reading right, writing right, but CONVERTING isn't working for some reason
			 * i've already checked that it's reading the current_level and writing to binary_res
			 * nothing about binary_convertor has been changed but it's not translating the bits right
			 * also i think they're sending right but i haven't looked as deeply
			 */
			binary_convertor(binary_result, result_counter - 1);
			
			/*
			for (int i = 0; i < result_counter; i++){
				printf("BINARY RES: %d\n", binary_res[i]);
			} 
			*/

			binary_res = realloc(binary_res, MAX_BIT);
			receive_address = realloc(receive_address, ADDRESS_SIZE * sizeof(int));
			sender_address = realloc(receive_address, ADDRESS_SIZE * sizeof(int));
			
			result_counter = 0;
			dest_counter = 0;
			src_counter = 0;
			what_destination = false;

		}
		if (time_diff >= 10000 && time_diff <= 20000){
			tracker = 0;
			long_tracker += 1;
		} else if (time_diff >= 1000 && time_diff <= 8000){
			tracker++;
			long_tracker = 0;
		}

	}

}
int main(){
	// Storing Received Data ...
	binary_res = (char*)malloc(MAX_BIT * sizeof(char));
	receive_address = (int*)malloc(ADDRESS_SIZE * sizeof(int));
	sender_address = (int*)malloc(ADDRESS_SIZE * sizeof(int));
//	free(binary_res);

	int PI = pigpio_start(NULL, NULL);
	pthread_t thread[2];
	bool chatting = true;

	create_user();

	pthread_create(&thread[0], NULL, send_thread, NULL);
	pthread_create(&thread[1], NULL, receive_thread, NULL);

	pthread_join(thread[0], NULL);
	pthread_join(thread[1], NULL);
	
	free(binary_res);
	free(receive_address);	
	free(sender_address);
	pigpio_stop(PI);
	return 0;
}


