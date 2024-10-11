#include <stdio.h>
#include <pigpiod_if2.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

#define MAX_BIT 1600 //max data is 200 characters = 1600 bytes
#define ADDRESS_SIZE 5
#define QUEUE_SIZE 500

uint32_t current_tick = 0;
uint32_t previous_tick;
unsigned current_level = 0;
unsigned prev_level;
uint32_t time_diff;
int result_counter = 0;
int pair = 0;
char *binary_res;
int *receive_address;
int *sender_address;
bool run = true;
char *total_response;
//source counter
int src_counter = 0;
//destination counter
int dest_counter = 0;
int PI;
bool what_destination = false;

// For our queue checkers
bool sending = false;
bool received_raspberry = false;
bool received_froot = false;

// SECOND THREAD VARIABLES
uint32_t r_current_tick = 0;
uint32_t r_previous_tick;
unsigned r_current_level = 0;
unsigned r_prev_level;
uint32_t r_time_diff;
int r_result_counter = 0;
int r_pair = 0;
char *r_binary_res;
int *r_receive_address;
int *r_sender_address;
bool r_run = true;
char *r_total_response;
int r_src_counter = 0;
int r_dest_counter = 0;
bool r_what_destination = false;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

//------------STORED PI INFORMATION-----------//
//THIS PI
char my_username[50] = "@pp1e_pi69";
int my_address[5] = {1,0,1,0,1};

// refactor second_pi_username to froot_pi
//
//SECOND PI
char froot_pi[50] = "fr00t_pi";
int froot_pi_address[5] = {1,0,0,0,1};


// refactor third_pi_username to raspberry_pi
// and third pi address to raspberry_pi_address
//THIRD PI
char raspberry_pi[50] = "r4sp_b3rry_pi";
int raspberry_pi_address[5] = {1,1,0,1,1};
//-------------------------------------------//
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

//------------QUEUE---------------
typedef struct{
	char *items[QUEUE_SIZE];
	int front;
	int rear;
} Queue;

void initializeQueue(Queue* q){
	q->front = -1; //access elements in struct
	q->rear = 0;
}
bool isEmpty(Queue* q){
	return (q->front == q->rear - 1);
}
bool isFull(Queue* q){
	return (q->rear == QUEUE_SIZE);
}
//-----------------------------------

//prompt user to type username and store it....
char* create_user(){
	printf("This is your username: %s\n", my_username);
	printf("This is your unique address: ");
	for(int i = 0; i < 5; i++){
	printf("%d", my_address[i]);
	}
	printf("\n");

}

// if the destination 
bool check_destination(int *destination_address){
	if(memcmp(destination_address, my_address, sizeof(my_address))==0){
		//printf("Matching Addresses\n");
		return true;
	}else if(memcmp(destination_address, my_address, sizeof(my_address)) != 0){ //&& memcmp(destination_address, my_address, sizeof(destination_address)) != 0){
		//printf("Dont know who this is, send to the next one\n");
		return false;
	}
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
		//total_response[total_response_counter] = current_level;
		//total_response_counter++;
		//printf("Start: %d\n",current_level);
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
			dest_counter++;
		} else if (dest_counter == 5) {
			what_destination = check_destination(receive_address);
			if (src_counter < 5) {
				sender_address[src_counter] = current_level;
				src_counter++;
			} else if (src_counter == 5) {
				binary_res[result_counter] = current_level;
				result_counter++;
			}
		}

	} else if (time_diff >= 1000 && time_diff <= 8000) {
		pair++;
		if (pair == 2) {
			if (dest_counter < 5){
				receive_address[dest_counter] = current_level;
				dest_counter++;
				pair = 0;	
			} else if (dest_counter == 5){
				what_destination = check_destination(receive_address);
				if(src_counter < 5) {
					sender_address[src_counter] = current_level;
					src_counter++;
					pair = 0;
				} else if (src_counter == 5){
					//printf("adding from short...\n");
					binary_res[result_counter] = current_level;
					result_counter++;
					pair = 0;
				}
			}
		}			
	}
}

void my_callback2(int pi, unsigned user_gpio, unsigned level, uint32_t tick){
//	printf("Recieving signals\n");

	if (r_previous_tick == 0) {	
		// solution: set level to zero in receive, do not collect the first bit sent!!!!!!!!!
		// in sender, add a 1 to the beginning of ALL messages
		//hardcoded so first value isn't a crazy high number and break our if statement (detect first level change)
		r_previous_tick = tick - 1;
		r_current_tick = tick;
		//hardcoded previous level to be low at 0 for the very first time the system runs
		r_prev_level = 0;
		r_current_level = level;
		//total_response[total_response_counter] = current_level;
		//total_response_counter++;
		//printf("Start: %d\n",current_level);
	} else {
		r_previous_tick = r_current_tick;
		r_current_tick = tick;
		r_prev_level = r_current_level;
		r_current_level = level;
	}
	r_time_diff = r_current_tick - r_previous_tick;

	if (r_time_diff >= 10000 && r_time_diff <= 20000) {
		// want to read and store the first 5 bits as address from source
		if (r_dest_counter < 5) { 
			r_receive_address[r_dest_counter] = r_current_level;
			r_dest_counter++;
		} else if (r_dest_counter == 5) {
			r_what_destination = check_destination(r_receive_address);
			if (r_src_counter < 5) {
				r_sender_address[r_src_counter] = r_current_level;
				r_src_counter++;
			} else if (r_src_counter == 5) {
				r_binary_res[r_result_counter] = r_current_level;
				r_result_counter++;
			}
		}

	} else if (r_time_diff >= 1000 && r_time_diff <= 8000) {
		r_pair++;
		if (r_pair == 2) {
			if (r_dest_counter < 5){
				r_receive_address[r_dest_counter] = r_current_level;
				r_dest_counter++;
				r_pair = 0;	
			} else if (r_dest_counter == 5){
				r_what_destination = check_destination(r_receive_address);
				if(r_src_counter < 5) {
					r_sender_address[r_src_counter] = r_current_level;
					r_src_counter++;
					r_pair = 0;
				} else if (r_src_counter == 5){
					//printf("adding from short...\n");
					r_binary_res[r_result_counter] = r_current_level;
					r_result_counter++;
					r_pair = 0;
				}
			}
		}			
	}
}


void binary_convertor(char *data, int data_length) {
	unsigned char binary = 0;
	int bits = 0;
	
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
			printf("%c", binary);
		}

	}
	printf("\n");
}

//------------------QUEUE----------------------
void enqueue(Queue* q, char *message){
	if(isFull(q)){
		return;
	}
	q->items[q->rear] = message;
	q->rear;
}

void dequeue(Queue* q){
	if(isEmpty(q)){
		return;
	}
	q->front++;
}
//------------------------------------------

void *send_thread(){
	printf("Starting send thread...\n");
	/*
	 * !CAUTION! Make sure to change 'tx' to appropriate transmitter port
	 */
	int tx;
	int second_pi = port1;
	int third_pi = port3;

	int PI = pigpio_start(NULL, NULL);
	Queue q;
	initializeQueue(&q);

	//prompt user to type
	char user_input[sizeof(char) * 500];
	bool running = true;
	int recipient_address[5];
	printf("Who do you want to send your message to? Type 'g*' for r4spb3rry_pi or 'w*' for fr00t_pi. You do not need to type 'g' or 'w' continue chatting with the user you were previously chatting with. To switch users to chat with, press 'g' or 'w'. Type 'yy' to exit the program. Your message cannot exceed 200 characters max\n");

	while(running) {
	//char user_input[200];	
		printf("\nSend from %s", my_username);
		printf("\n");
		scanf("%[^\n]%*c", &user_input);
		//enqueue(&q, user_input);
	
		if(strlen(user_input) > 199) {
			printf("Message limit exceeded... Please send shorter message!\n");
			printf("Size: %d\n", strlen(user_input));
		} else if (strlen(user_input) <= 199){	
			//choose recipient
			if(user_input[0] == 'w' && user_input[1] == '*') {
				printf("Sending to fr00t_pi\n");
				memcpy(recipient_address,froot_pi_address,sizeof(froot_pi_address));
				tx = second_pi;
				result_counter = 0;
				current_tick = 0;
				previous_tick = 0;
				current_level = 0;
				prev_level = 0;
				time_diff = 0;
				binary_res = realloc(binary_res, MAX_BIT);
				receive_address = realloc(receive_address, ADDRESS_SIZE * sizeof(int));
				sender_address = realloc(sender_address, ADDRESS_SIZE * sizeof(int));
				total_response = (char*)malloc(MAX_BIT+12 * sizeof(char));
			} else if(user_input[0] == 'g' && user_input[1] == '*') { 
				printf("Sending to r4spb3rry_pi\n");
				memcpy(recipient_address,raspberry_pi_address,sizeof(raspberry_pi_address));
				tx = third_pi;
				result_counter = 0;
				current_tick = 0;
				previous_tick = 0;
				current_level = 0;
				prev_level = 0;
				time_diff = 0;
				binary_res = realloc(binary_res, MAX_BIT);
				receive_address = realloc(receive_address, ADDRESS_SIZE * sizeof(int));
				sender_address = realloc(sender_address, ADDRESS_SIZE * sizeof(int));
				total_response = (char*)malloc(MAX_BIT+12 * sizeof(char));
			}else if(user_input[0] == 'y' && user_input[1] == 'y') {
				printf("Exiting...\n");
				running = false;
			}
			set_mode(PI, tx, PI_OUTPUT);
	
			//user exit code
			
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
			// store my address
			for(int j = 0; j < 5; j++) {
				result_binary[result_counter] = my_address[j];
				result_counter += 1;
			}
			
			//message
			for (int k = 0; k < strlen(result); k++){
				result_binary[result_counter] = result[k] - '0'; //change to binary so it dont default to ascii
				result_counter += 1;
			}

			//Setting data trailer as '0'
			result_binary[strlen(result)+1+10] = 0; 
			result_counter += 1;
			send_manchester(result_binary, result_counter, PI, tx);
			free(result);
		}
	}
}

// Receiving from Froot Pi
void *receive_froot(){
	printf("Starting receive thread...\n");
	/*
	 * !CAUTION! Make sure to change 'rx' to appropriate receiver port
	 */
	int froot_port = recv4;
	int transfer_to_raspberry = port3;
	Queue q;
	initializeQueue(&q);
	char binary_result[1600];
	int tracker = 0;
	int long_tracker = 0;
	char user[50];
	// ----------------------

	while(run) {
		time_sleep(0.005);
		//setting up the callback
		//timestamps the GPIO state change
		pthread_mutex_lock(&mutex);	
		//if the user id array is not empty, execute the following, else don't do anything
		//recieve from second pi
		callback(PI, froot_port, EITHER_EDGE, my_callback);
		pthread_mutex_unlock(&mutex);

		
		if (tracker == 20 || long_tracker == 20) {
			memcpy(binary_result, binary_res, result_counter);
			binary_result[result_counter] = '\0';

			if(memcmp(sender_address, froot_pi_address,sizeof(froot_pi_address)) == 0) {
				memcpy(user, froot_pi, sizeof(froot_pi));
			} else if (memcmp(sender_address,raspberry_pi_address,sizeof(raspberry_pi_address)) == 0) {
				memcpy(user, raspberry_pi,sizeof(raspberry_pi));
			} else {
				strcpy(user,"unknown");
			}

			//printf("\nFrom user %s: ", user);
			
			//NEW KALIE CODE RAHHHHH
			//when I recieve the message from the user (either raspberry_pi or froot_pi, I need to know if it is meant for me or meant for the other person
			//if the address is froot_pi's address, forward to froot_pi
			if(memcmp(receive_address, raspberry_pi_address, sizeof(raspberry_pi_address))==0){
			//	printf("DO NOT DISPLAY IN FINAL: Forward to froot_pi\n");
				total_response[0] = 1;
				int total_res_size = 1;

				for(int m = 0; m < 5; m++) {
					total_response[total_res_size] = receive_address[m];
					total_res_size++;
				}
				for(int n = 0; n < 5; n++) {
					total_response[total_res_size] = sender_address[n];
					total_res_size++;
				}
				for(int o = 0; o < result_counter - 1; o++) {
					total_response[total_res_size] = binary_result[o];
					total_res_size++;
				}

				total_response[total_res_size] = 0;
				total_res_size++;
				send_manchester(total_response, total_res_size, PI, transfer_to_raspberry);
			//else forward to raspberry_pi
			}else {
				printf("\nFrom user %s: ",user);
				binary_convertor(binary_result, result_counter-1);
			}
				
			enqueue(&q, total_response);
			
			pthread_mutex_lock(&mutex);
			dequeue(&q);
			pthread_mutex_unlock(&mutex);

			//------------------------------------//
			total_response = realloc(total_response, MAX_BIT+12);
			binary_res = realloc(binary_res, MAX_BIT);
			receive_address = realloc(receive_address, ADDRESS_SIZE * sizeof(int));
			sender_address = realloc(sender_address, ADDRESS_SIZE * sizeof(int));
	
			result_counter = 0;
			dest_counter = 0;
			src_counter = 0;
			what_destination = false;

		}
		if (time_diff >= 10000 && time_diff <= 20000){
			tracker = 0;
			long_tracker += 1;
		} else if (time_diff >= 1000 && time_diff <= 8000){
			tracker += 1;
			long_tracker = 0;
		}

	}

}

// Receiving from Raspberry Pi
void *receive_raspberry(){
	printf("Starting receive thread...\n");
	/*
	 * !CAUTION! Make sure to change 'rx' to appropriate receiver port
	 */
	int raspberry_port = recv3;
	int transfer_froot = port1;
	
	Queue q;
	initializeQueue(&q);
	char r_binary_result[1600];
	int r_tracker = 0;
	int r_long_tracker = 0;
	char r_user[50];
	// ----------------------

	while(run) {
		time_sleep(0.005);
		//setting up the callback
		//timestamps the GPIO state change
		pthread_mutex_lock(&mutex);	
		//recieve from third pi
		callback(PI, raspberry_port, EITHER_EDGE, my_callback2);
		pthread_mutex_unlock(&mutex);
		
		if (r_tracker == 20 || r_long_tracker == 20) {
			memcpy(r_binary_result, r_binary_res, r_result_counter);
			r_binary_result[r_result_counter] = '\0';

			if(memcmp(r_sender_address, froot_pi_address,sizeof(froot_pi_address)) == 0) {
				memcpy(r_user, froot_pi, sizeof(froot_pi));
			} else if (memcmp(r_sender_address,raspberry_pi_address,sizeof(raspberry_pi_address)) == 0) {
				memcpy(r_user, raspberry_pi,sizeof(raspberry_pi));
			} else {
				strcpy(r_user,"unknown");
			}

			
			if(memcmp(r_receive_address, froot_pi_address, sizeof(froot_pi_address))==0){
				r_total_response[0] = 1;
				int r_total_res_size = 1;

				for(int m = 0; m < 5; m++) {
					r_total_response[r_total_res_size] = r_receive_address[m];
					r_total_res_size++;
				}
				for(int n = 0; n < 5; n++) {
					r_total_response[r_current_tick] = r_sender_address[n];
					r_total_res_size++;
				}
				for(int o = 0; o < r_result_counter - 1; o++) {
					r_total_response[r_total_res_size] = r_binary_result[o];
					r_total_res_size++;
				}

				r_total_response[r_total_res_size] = 0;
				r_total_res_size++;
				send_manchester(r_total_response, r_total_res_size, PI, transfer_froot);
			}else {
				printf("\nFrom user %s: ",r_user);
				binary_convertor(r_binary_result, r_result_counter-1);
			}
			
			enqueue(&q, r_total_response);
			
			pthread_mutex_lock(&mutex);
			dequeue(&q);
			pthread_mutex_unlock(&mutex);
			//------------------------------------//
			r_total_response = realloc(r_total_response, MAX_BIT+12);
			r_binary_res = realloc(r_binary_res, MAX_BIT);
			r_receive_address = realloc(r_receive_address, ADDRESS_SIZE * sizeof(int));
			r_sender_address = realloc(r_sender_address, ADDRESS_SIZE * sizeof(int));
	
			r_result_counter = 0;
			r_dest_counter = 0;
			r_src_counter = 0;
			r_what_destination = false;

		}
		if (r_time_diff >= 10000 && r_time_diff <= 20000){
			r_tracker = 0;
			r_long_tracker += 1;
		} else if (r_time_diff >= 1000 && r_time_diff <= 8000){
			r_tracker += 1;
			r_long_tracker = 0;
		}

	}

}


//void *check_queues(){
/* Refactor send thread so that when we have our data to be sent, it gets store into a queue
 * -- This thread will now deal with calling manchester or binary_convertor whenever 
 *    a message is added to the queue --
 * 
 * We want to be able to check each queue one at a time so that we don't get clashing data when recceiving/transferring data
 * 
 * This thread should also be handling forwarding 
*/

//}

int main(){
	// Storing Received Data ...
	binary_res = (char*)malloc(MAX_BIT * sizeof(char));
	total_response = (char*)malloc(MAX_BIT+12 * sizeof(char));
	receive_address = (int*)malloc(ADDRESS_SIZE * sizeof(int));
	sender_address = (int*)malloc(ADDRESS_SIZE * sizeof(int));
	r_binary_res = (char*)malloc(MAX_BIT * sizeof(char));
	r_total_response = (char*)malloc(MAX_BIT+12 * sizeof(char));
	r_receive_address = (int*)malloc(ADDRESS_SIZE * sizeof(int));
	r_sender_address = (int*)malloc(ADDRESS_SIZE * sizeof(int));

	PI = pigpio_start(NULL, NULL);
	pthread_t thread[3];

	create_user();

	pthread_create(&thread[0], NULL, send_thread, NULL);
	pthread_create(&thread[1], NULL, receive_froot, NULL);
	pthread_create(&thread[2], NULL, receive_raspberry, NULL);

	pthread_join(thread[0], NULL);
	pthread_join(thread[1], NULL);
	pthread_join(thread[2], NULL);
	
	free(binary_res);
	free(total_response);
	free(receive_address);	
	free(sender_address);

	free(r_binary_res);
	free(r_total_response);
	free(r_receive_address);	
	free(r_sender_address);
	pigpio_stop(PI);
	return 0;
}


