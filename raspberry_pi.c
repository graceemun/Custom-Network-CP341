#include <stdio.h>
#include <pigpiod_if2.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

#define MAX_BIT 1600 //max data is 200 characters = 1600 bytes
#define ADDRESS_SIZE 5
#define QUEUE_SIZE 500 //QUEUE SIZE IS 213 BECAUSE THAT IS THE MAX 

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
int *receive_address;
int *sender_address;

int PI;
//------------STORED PI INFORMATION-----------//
//OTHER PI
char apple_pi[50] = "@pp1e_pi69";
int apple_pi_address[5] = {1,0,1,0,1};

//this PI
char froot_pi[50] = "fr00t_pi";
int froot_pi_address[5] = {1,0,0,0,1};

//THIRD PI
char my_username[50] = "r4sp_b3rry_pi";
int my_address[5] = {1,1,0,1,1};
//-------------------------------------------//

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

//--------queue-------
/*(char *incoming_queue[QUEUE_SIZE]; //array to hold incoming messages
char *outgoing_queue[QUEUE_SIZE]; //array to hold messages being read
int in_queue_size = 0;
int out_queue_size = 0;
*/
typedef struct{
	char *items[QUEUE_SIZE];
	int front;
	int rear;
} Queue;

void initializeQueue(Queue* q){
	q->front = -1;
	q->rear = 0;
}
bool isEmpty(Queue* q){
	return (q->front == q-> rear - 1);
}
bool isFull(Queue* q){
	return (q->rear == QUEUE_SIZE);
}

//---------------------------

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
		}
		if (what_destination){
		       	if(src_counter < 5) {
				sender_address[src_counter] = current_level;
				src_counter++;
			}else if(src_counter == 5) {
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
			}else if (dest_counter == 5){
				what_destination = check_destination(receive_address);
				
			}if (what_destination){	
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

//------------QUEUE-----------------
void enqueue(Queue* q, char *message){
	//pthread_mutex_lock(&mutex);
	if(isFull(q)){
		//printf("Queue is full\n");
		return;
	}
	q->items[q->rear] = message;
	q->rear++;
}

void dequeue(Queue* q){
	if(isEmpty(q)){
		//printf("Queue is empty\n");
		return;
	}
	q->front++;
}
/*
void printQueue(Queue* q){
	if (isEmpty(q)){
		printf("Queue is empty\n");
		return;
	}
	printf("current queue: ");
	for (int i = q->front + 1; i < q->rear; i++){
		printf("%d", q->items[i]);
	}
	printf("\n");
}
*/
//------------------------------------------

void *send_thread(){
	printf("Starting send thread...\n");
	/*
	 * !CAUTION! Make sure to change 'tx' to appropriate transmitter port
	 */
	int tx = port2;

	//prompt user to type
	bool running = true;
	int recipient_address[5];
	// FOR FUTURE: implement character size checking in a better way instead of having to allocate more space than required
	char user_input[sizeof(char) * 500];
	printf("Who do you want to send your message to? Type 'w*' for froot_pi or 'k*' for @pple_pi69. You do not need to press 'w*' or 'k*' continue chatting with the user you were previously chatting with. To switch users to chat with, press 'w*' or 'k*'. Type 'yy' to exit the program. Your message cannot exceed 200 characters max\n");

	while(running) {
		printf("\nSend from %s", my_username);
		printf("\n");
		//scanf("%200[^\n]%*c", &user_input);
		scanf("%[^\n]%*c", &user_input);
		
		if(strlen(user_input) > 199){
			printf("Message Limit Exceeded... Please send shorter message!\n");
			printf("Size: %d\n", strlen(user_input));	
		}else if (strlen(user_input) <= 199){
			printf("String length: %d\n", strlen(user_input));
			if(user_input[0] == 'w' && user_input[1] == '*'){
				//choose recipient
				printf("Sending to fr00t_pi\n");
				memcpy(recipient_address, froot_pi_address, sizeof(froot_pi_address));
				result_counter = 0;
				current_tick = 0;
				previous_tick = 0;
				current_level = 0;
				prev_level = 0;
				time_diff = 0;
				binary_res = realloc(binary_res, MAX_BIT);
				receive_address = realloc(receive_address, ADDRESS_SIZE * sizeof(int));
				sender_address = realloc(sender_address, ADDRESS_SIZE * sizeof(int));
			}else if(user_input[0] == 'k' && user_input[1] == '*'){
				printf("Sending to @pple_pi69\n");
				memcpy(recipient_address, apple_pi_address, sizeof(apple_pi_address));
				result_counter = 0;
				current_tick = 0;
				previous_tick = 0;
				prev_level = 0;
				time_diff = 0;
				binary_res = realloc(binary_res, MAX_BIT);
				receive_address = realloc(receive_address, ADDRESS_SIZE * sizeof(int));
				sender_address = realloc(sender_address, ADDRESS_SIZE * sizeof(int));
			}else if(user_input[0] == 'y' && user_input[1] == 'y') {
				printf("Exiting...\n");
				running = false;
				exit(-1);
			}
			set_mode(PI, tx, PI_OUTPUT);
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


void *receive_thread(){
	printf("Starting receive thread...\n");
	/*
	 * !CAUTION! Make sure to change 'rx' to appropriate receiver port
	 */
	int second_pi = recv2;
	Queue q;
	initializeQueue(&q);
	char binary_result[1600];
	int PI = pigpio_start(NULL, NULL);
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
		callback(PI, second_pi, EITHER_EDGE, my_callback);
		
		pthread_mutex_unlock(&mutex);
		
		if (tracker == 20 || long_tracker == 20) {
			memcpy(binary_result, binary_res, result_counter);
			binary_result[result_counter] = '\0';
		//------------------------------------------------------------------------	
			// Get correct username based off of user address
			if (memcmp(sender_address, apple_pi_address,sizeof(apple_pi_address)) == 0) {
				memcpy(user, apple_pi, sizeof(apple_pi));
			} else if(memcmp(sender_address, froot_pi_address,sizeof(froot_pi_address)) == 0) {
				memcpy(user, froot_pi, sizeof(froot_pi));
			} else {
				strcpy(user,"unknown");
			}
			printf("\nFrom user %s: ", user);
		//------------------------------------------------------------------------	
				
			//ENQUEUED BINARY RESULT
			//pthread_mutex_lock(&mutex);
			enqueue(&q, binary_result);
		//	pthread_mutex_unlock(&mutex);
			//printQueue(&q);

			//DEQUEUE
			pthread_mutex_lock(&mutex);
			dequeue(&q);
			pthread_mutex_unlock(&mutex);
			//printQueue(&q);
		
			binary_convertor(binary_result, result_counter - 1);
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
int main(){
	binary_res = (char*)malloc(MAX_BIT * sizeof(char));
	receive_address = (int*)malloc(ADDRESS_SIZE * sizeof(int));
	sender_address = (int*)malloc(ADDRESS_SIZE * sizeof(int));

	PI = pigpio_start(NULL, NULL);
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

