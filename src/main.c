#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <limits.h>

/*
1. Read Me
    1. Motivation Section
    2. Features
    3. Learnings
    4. Compiling and running it
2. Add a .gitignore
3. See if I am using malloc and realloc correctly 
    1. Find a tool and implement it
    2. Quickly implement a malloc and realloc wrapper
4. Implement semicolons, pipes, redirection
5. Implement an extra feature
    1. Implementing environment variables
    2. Implementing some more built in commands
6. Come up with test cases
7. Fix the return values of each function
8. Fix the error output
*/

void run_program();
void parse_input();
int fetch_line();
void parse_line(char *stdin_input, int write_end_pipe, int read_end_pipe);
void find_path(char* command, char **arguments, int write_end_pipe, int read_end_pipe);
void execute_command(char *command_path, char **arguments, int write_end_pipe, int read_end_pipe);

const int NO_PIPE = -1000;

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

	int command_counter = 0;
	int **file_descriptors = (int*)malloc(BUFFER_SIZE * sizeof(int*));
	int PIPE_PRESENT = 0;
	
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
		        if (PIPE_PRESENT) {
				parse_line(stdin_string, NO_PIPE, file_descriptors[command_counter-1][0]);
			} else {
				parse_line(stdin_string, NO_PIPE, NO_PIPE);
			}	
			free(stdin_string);
			count_length_string = 0;
			PIPE_PRESENT = 0;
			return 1;
		} else if (ch == ';') {
			stdin_string[count_length_string] = '\0';
			if (PIPE_PRESENT) {
				parse_line(stdin_string, NO_PIPE, file_descriptors[command_counter-1][0]);
			} else {
				parse_line(stdin_string, NO_PIPE, NO_PIPE);
			}	
			free(stdin_string);
			count_length_string = 0;
			stdin_string = ( char* )malloc( BUFFER_SIZE * sizeof( char ) );
			command_counter++;
			PIPE_PRESENT = 0;
		} else if (ch == '|') {
			file_descriptors[command_counter] = (int*)malloc(2*sizeof(int));
			if (pipe(file_descriptors[command_counter]) == -1) {
				err(EXIT_FAILURE, "pipe() failed");
			}
			stdin_string[count_length_string] = '\0';
			if (PIPE_PRESENT) {
				parse_line(stdin_string, file_descriptors[command_counter][1], file_descriptors[command_counter-1][0]);
			} else {
				parse_line(stdin_string, file_descriptors[command_counter][1], NO_PIPE);
			}	
			free(stdin_string);
			count_length_string = 0;
			stdin_string = ( char* )malloc( BUFFER_SIZE * sizeof( char ) );
			PIPE_PRESENT = 1;
			command_counter++;
		} else {
			stdin_string[count_length_string] = ch;
			count_length_string++;
		}
	}

	free(stdin_string);
	return 0;

}

void parse_line(char *stdin_input, int write_end_pipe, int read_end_pipe) {
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

	if (strcmp(command, "cd") == 0) {
		if (argument_index == 1) {
			const char *HOME = "HOME";
			const char *home_dir = getenv(HOME);
			int ret = chdir(home_dir);
			if (ret != 0) {
				printf("There was an issue changing the directory!");
			}
		} else if (argument_index == 2) {
			int ret = chdir(arguments[1]);
			if (ret != 0) {
				printf("There was an issue changing the directory!");
			}
		}
	} else if (strcmp(command, "exit") == 0) {
		exit(0);	
	} else if (strcmp(command, "echo") == 0) {
		for (int i = 1; i < argument_index; i++) {
			printf("%s ", arguments[i]);
		}
	} else if (strcmp(command, "pwd") == 0) {
		char cwd[PATH_MAX];
		if (getcwd(cwd, sizeof(cwd)) != NULL) {
       			printf("%s", cwd);
   		} else {
       			printf("getcwd() error");
   		}
	} else {
		find_path(command, arguments, write_end_pipe, read_end_pipe);
	}

 	free(argument);
	free(command);
	for (int i = 0; i < argument_index; i++) {
		free(arguments[argument_index]);
	}
	free(arguments);	
}

void find_path(char *command, char **arguments, write_end_pipe, read_end_pipe) {
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
		execute_command(search_path, arguments, write_end_pipe, read_end_pipe);	
		free(search_path);
	}
}

void execute_command(char *command_path, char **arguments, int write_end_pipe, int read_end_pipe) {
	pid_t pid = fork();

	if (pid < 0) {
		printf("fork has failed!");
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


	
