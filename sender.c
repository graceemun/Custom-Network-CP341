#include <stdio.h>
#include <pigpiod_if2.h>
#include <string.h>
#include <stdlib.h>


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

void send_manchester(char *data, int PI, int gpio){
	int size = strlen(data);
	
	for (int i = 0; i < size; i++){
		if (data[i] == '1'){
			printf("Sending 1...\n");
			gpio_write(PI, gpio, 0);
			time_sleep(0.5);
			gpio_write(PI, gpio, 1);
			time_sleep(0.5);				
		} else if (data[i] == '0'){
			printf("Sending 0...\n");
			gpio_write(PI, gpio, 1);
			time_sleep(0.5);
			gpio_write(PI, gpio, 0);
			time_sleep(0.5);
		} 
	}	
}

int main(){
        int port1 = 27;
        int port2 = 25;
        int port3 = 23;
        int port4 = 21;

//	char* user_response[200];

//	printf("Type your message (200 characters max): \n");
	//scanf("%[^\n]", &user_response);

	//int res_length = strlen(user_response);

	//res_length = res_length * 8;

	//char* binary_input[1616] = "10101010" + 

	// scan f to get user input
	// turn user input to binary
	// get length of binary input and initialize char data_to_send[length_of_binary] = "binary input" PLUS 0000000
	//char data_to_send[16] = "1111";
	char data_to_send[16] = "0000";

	printf("%c\n", data_to_send[2]);
	char data_buffer[sizeof(data_to_send)];

	memcpy(data_buffer, data_to_send, sizeof(data_to_send));
	
	/*	
	printf("Unpacking binary string:\n");
    	for (int i = 0; i < strlen(data_buffer); i++) {
        	printf("Bit %d: %c\n", i + 1, data_buffer[i]);
    	}
	*/	

        
	int PI = pigpio_start(NULL, NULL);
        set_mode(PI, port1, PI_OUTPUT);
	
	send_manchester(data_buffer, PI, port1);

	pigpio_stop(PI);
	
	return 0;
}


