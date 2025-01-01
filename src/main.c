#include <stdio.h>
#include <stdlib.h>

// 1. need to receive input continously - for now, this will be only be able to take in a line of commands 
// 2. take in the argument and parse it
// 	a. need to get character by character - how do i store this in string array
//		i. this should all be stored in one string
//		ii. we can use malloc to dynamically store this string (remember to free it)
//		iii. should be able to resize the malloc string if the space we reserved is too small (for now, we can use 1024) 
// 	b. use strtok to split up string into the command and arguments  
// 3. execute the argument using fork() and exec()
// 4. know the difference between built-in and executable commands

void run_program();
void parse_input();
int parse_line();

int main() {
	run_program();
}

void run_program() {
	int status = 0;

	printf("%s\n", "Welcome to the shell! Please enter your input below!");

	parse_input();
}

void parse_input() {

	int status = 1;

	while (status) {
		status = parse_line();
	}

}

int parse_line() {
	
	int ch;

	char *stdin_string = ( char* )malloc( 1024 *sizeof( char ) );
	int count_length_string = 0;

	printf("\n>");
	
	while ((ch = getchar()) != EOF) {
		if (ch == '\n') {
			stdin_string[count_length_string] = '\0';
			printf("%s", stdin_string);
			free(stdin_string);
			count_length_string = 0;
			break;
		} 
		else {
			stdin_string[count_length_string] = ch;	
			count_length_string++;
		}
	}

	if (ch == EOF) {
		return 0;
	} else {
		return 1;
	}

}
	
