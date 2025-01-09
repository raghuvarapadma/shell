#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

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
void find_path(char* command, char **arguments);
void execute_command(char *command_path, char **arguments);

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
			char** arguments_copy = realloc(arguments, COMMAND_SIZE * sizeof(char*));
			if (arguments_copy == NULL) {
				printf("%s\n", "Memory allocation error!");
				free(arguments_copy);
				free(arguments);
				return;
			}

			arguments = arguments_copy;
		}

		int argument_length = strlen(argument);
		if (argument_index == 0) {
			command = malloc((argument_length+1) * sizeof(char));
			strcpy(command, argument);
		}
		
		arguments[argument_index] = malloc((argument_length + 1) * sizeof(char));
		strcpy(arguments[argument_index], argument);
		argument_index++;

		argument = strtok(NULL, " ");
	}

	find_path(command, arguments);

	// free all the space here used by arguments including the memory allocated by command_arr

	
}

void find_path(char *command, char **arguments) {
	int command_len = strlen(command);
	char *path = "PATH";
	char *get_env = getenv(path);
	char *path_result = malloc(strlen(get_env) * sizeof(char));

	strcpy(path_result, getenv(path));

	char *location;
	int access_allowed = -1;
	char *search_path;
	
	location = strtok(path_result, ":");

	while (location != NULL) {
		int location_len = strlen(location);
		int total_len = command_len + location_len+2;
		
		search_path = malloc(total_len * sizeof(char));

		strcat(search_path, location);
		strcat(search_path, "/");
		strcat(search_path, command);
		
		access_allowed = access(search_path, F_OK);
		if (access_allowed == 0) {
			break;
		}
		else {
			free(search_path);
			location = strtok(NULL, ":");
		}
	}

	if (access_allowed == -1) {
		printf("%s\n", "Command is invalid!");
	} else {
		execute_command(search_path, arguments);	
		free(search_path);
	}
}

void execute_command(char *command_path, char **arguments) {
	pid_t pid = fork();

	if (pid < 0) {
		perror("fork has failed!");
		exit(1);
	} else if (pid == 0) {
		execv(command_path, arguments);
		exit(0);
	} else {
		int stat_loc;
		waitpid(pid, &stat_loc, 0);
		
		if (!WIFEXITED(stat_loc)) {
			printf("%s\n", "Process has failed!");
		}
	}
}


	
