#include <stdio.h>
#include <pigpiod_if2.h>
#include <string.h>
#include <stdlib.h>

//converts strings to binary
char* string_to_binary(char*s){
        if(s==NULL) return 0;
        size_t len = strlen(s);
        char *binary = malloc(len*8 + 1);
        binary[0] = '\0';
        for(size_t i = 0; i < len; i++){
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

int main(){
        int GPIO;
        int port1 = 27;
        int port2 = 25;
        int port3 = 23;
        int port4 = 21;

        int PI = pigpio_start(NULL, NULL);
        set_mode(PI, GPIO, PI_OUTPUT);

//get a user input and convert it to binary numbers
        char* user_input[200];
        int counter[200];
        printf("Type your message (200 characters max): ");
        scanf("%[^\n]", &user_input);
	if(strlen(user_input) > 200) {
		printf("Too long!\n");
		exit(-1);

	}

        char* result = string_to_binary(user_input);
        printf("Binary User RESULT: %s\n", result);


//store result into array
	char result_binary[1600];

	for (int m = 0; m < 1600; m++) {
		result_binary[m] = 9;
	}

	for (int k = 0; k < strlen(result); k++){
		result_binary[k] = result[k] - '0';
		printf("Stored binary is: %d\n", result_binary[k]);
	}

//parse through array with pointer
	int pointer = 0;

	while(result_binary[pointer] !=9){
		printf("Sending message ... %d\n", result_binary[pointer]);
		time_sleep(0.5);
		pointer+=1;
	}
	pigpio_stop(PI);
	return 0;
}

