#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
void execute_command(char *stdin_input);

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

	int BUFFER_SIZE = 1024;
	int ch;
	char *stdin_string = ( char* )malloc( BUFFER_SIZE * sizeof( char ) );
	int count_length_string = 0;

	if (stdin_string == NULL) {
		printf("%s\n", "Memory allocation error, please restart the program!");
		free(stdin_string);
		return 0;
	}

	printf("\n>");
	
	while ((ch = getchar()) != EOF) {
		if (count_length_string >= BUFFER_SIZE) {
			BUFFER_SIZE = BUFFER_SIZE * 2;
			char *stdin_string_copy = (char*) realloc(stdin_string, BUFFER_SIZE * sizeof( char ) );
			if (stdin_string_copy == NULL) {
				printf("%s\n", "Memory allocation error, please restart the program!");
				free(stdin_string_copy);
				free(stdin_string);
				return 0;
			}
			
			stdin_string = stdin_string_copy;	
		} 


		if (ch == '\n') {
			stdin_string[count_length_string] = '\0';
			execute_command(stdin_string);
			free(stdin_string);
			count_length_string = 0;
			return 1;
		}
 
		stdin_string[count_length_string] = ch;	
		count_length_string++;
	}

	free(stdin_string);
	return 0;

}

void execute_command(char *stdin_input) {
	printf("%s\n", stdin_input);
}

	
