#include <stdio.h>
#include <pigpiod_if2.h>
#include <string.h>
#include <stdlib.h>

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
//
//GOT RID OF TIME SLEEP
//int bit_rate = 0.75;

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

int main(){
        int port1 = 27;
        int port2 = 25;
        int port3 = 23;
        int port4 = 21;

	//prompt user to type
	char user_input[200];
	printf("Type your message (200 characters max): \n");
	scanf("%[^\n]", &user_input);
	
	//quits if over 200 characters
	if(strlen(user_input) > 200){
		printf("Too long!\n");
		exit(-1);
	}

	// turn user input to binary
	char* result = string_to_binary(user_input);
	printf("Binary user result: %s\n", result);

	//store result into array
	char result_binary[strlen(result)+2];

	//HARDCODED VALUES ARE NOT STORED, ONLY SENT
	//hardcoding first value as 1
	int result_counter = 1;
	result_binary[0] = '1' - '0';
	for (int k = 0; k < strlen(result); k++){
		result_binary[result_counter] = result[k] - '0'; //change to binary so it dont default to ascii
		//printf("Stored binary is: %d\n", result_binary[result_counter]);
		result_counter += 1;
	}
	//hardcode last value as 0
	result_binary[strlen(result)+1] = 0; 
        
	int PI = pigpio_start(NULL, NULL);
        set_mode(PI, port1, PI_OUTPUT);

	int size_of = sizeof(result_binary) + 2;

//	printf("size of reuslt_binary: %d\n", sizeof(result_binary));

	send_manchester(result_binary, size_of, PI, port1);
	pigpio_stop(PI);
	return 0;
}


