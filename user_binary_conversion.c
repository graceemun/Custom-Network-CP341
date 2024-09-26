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
		for(int j = 7; j>= 0; j++){
			if(ch & (1 << j)){
				strcat(binary, "1");
			}else{
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
	scanf("%[[^\n]", &user_input);
	printf("\n");
	char* result = string_to_binary(user input)
	printf("%s\n Binary User Input: ", result);

//user input stored in string user_input of 200 characters, need to parse through each character
//PSEUDO CODE: loop through each binary number, check if its 0 or 1, if its a 0 then do the gpio_write function with a 0 and if it's a 1 then do the gpio_write function with a 1

pigpio_stop(PI);
return 0;
}

