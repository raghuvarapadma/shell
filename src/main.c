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
int fetch_line();
void parse_line(char *stdin_input);
void exec_command(char* argument, char **arguments);

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
		status = fetch_line();
	}

}

int fetch_line() {

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
			parse_line(stdin_string);
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

void parse_line(char *stdin_input) {
	int COMMAND_SIZE = 20; 
	char *command;
	char **arguments = malloc(COMMAND_SIZE * sizeof(char*));
	char *argument;
	int argument_index = 0;

	argument = strtok(stdin_input, " ");

	while (argument != NULL) {
		if (argument_index >= COMMAND_SIZE) {
			COMMAND_SIZE = COMMAND_SIZE * 2;
			char** arguments_copy = realloc(command_arr, COMMAND_SIZE * sizeof(char*));
			if (arguments_copy == NULL) {
				printf("%s\n", "Memory allocation error!");
				free(arguments_copy);
				free(arguments);
				return;
			}

			arguments = command_arr_copy;
		}

		int argument_length = strlen(argument);
		if (argument_index == 0) {
			command = malloc((argument_length+1) * sizeof(char));
			strcpy(command, argument);
		}
		else {
			arguments[argument_index] = malloc((argument_length + 1) * sizeof(char));
			strcpy(arguments[argument_index], argument);
			argument_index++;
		}

		argument = strtok(NULL, " ");
	}

	// free all the space here used by arguments including the memory allocated by command_arr

	
}

// exec logic down here
void exec_command(char *command, char **arguments) {
	// need to use PATH to search through valid directories and use access() to see if directory is valid
	// then i can do execv command with command and arguments	
}

	
