#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <limits.h>
#include <err.h>
#include <fcntl.h>
#include <errno.h>

void run_program();
void parse_input();
int fetch_line();
int parse_command(char *stdin_input, int* write_end_pipe, int* read_end_pipe);
void find_path(char* command, char **arguments, int* write_end_pipe, int* read_end_pipe);
void execute_command(char *command_path, char **arguments, int* write_end_pipe, int* read_end_pipe);
void free_memory_fetch_line(char* stdin_string, int** file_descriptors, int command_counter);
void free_memory_parse_command(char** arguments, int arguments_length, char* argument, char* command);
int close_pipe_end(int fd);

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

void free_memory_fetch_line(char* stdin_string, int** file_descriptors, int command_counter) {
	if (stdin_string != NULL) {
		free(stdin_string);
	}

	if (file_descriptors != NULL) {
		for (int i = 0; i < command_counter; i++) {
			if (file_descriptors[i] != NULL) {
				free(file_descriptors[i]);
			}
		}
		free(file_descriptors);
	}
}

void free_memory_parse_command(char** arguments, int arguments_length, char* argument, char* command) {
	if (arguments != NULL) {
		for (int i = 0; i < arguments_length; i++) {
			if (arguments[i] != NULL) {
				free(arguments[i]);
			}
		}
		free(arguments);	
	}
	argument = NULL;
	command = NULL;
}

int close_pipe_end(int fd) {
	if (fcntl(fd, F_GETFD) == -1) {
		if (errno != EBADF) {
			fprintf(stderr, "fcntl() command failed");
			return 0;
		}
	} else {
		if (close(fd) == -1) {
			fprintf(stderr, "close() command failed");
			return 0; 
		}
	}
	return 1;
}

//void call_parse_command(char seperator, char* stdin_string, int** file_descriptors, command_counter) {
//	if (seperator == ';' || seperator == '\n') {
//
//	} else if (seperator == '|') {
//
//	}
//}

int fetch_line() {

	int BUFFER_SIZE = 1024;

	int ch;
	int command_counter = 0;
	int count_length_string = 0;

	char *stdin_string = ( char* )malloc( BUFFER_SIZE * sizeof( char ) );
	if (stdin_string == NULL) {
		err(EXIT_FAILURE, "malloc() error when allocating stdin_string");
	}


	int **file_descriptors = (int**)malloc(BUFFER_SIZE * sizeof(int*));
	if (file_descriptors == NULL) {
		free(stdin_string);
		err(EXIT_FAILURE, "malloc() error when allocating file_descriptors");
	}

	printf("\n> ");

	while ((ch = getchar()) != EOF) {
		if (count_length_string >= BUFFER_SIZE) {
			BUFFER_SIZE = BUFFER_SIZE * 2;
			char *stdin_string_copy = (char*) realloc(stdin_string, BUFFER_SIZE * sizeof( char ) );
			if (stdin_string_copy == NULL) {
				free_memory_fetch_line(stdin_string, file_descriptors, command_counter);
				err(EXIT_FAILURE, "realloc() error when reallocating stding_string");
			}

			stdin_string = stdin_string_copy;	
		}


		if (ch == '\n') {
			stdin_string[count_length_string] = '\0';
			if (command_counter > 0 && file_descriptors[command_counter-1]) {
				int parse_command_result = parse_command(stdin_string, NULL, file_descriptors[command_counter-1]);
				if (parse_command_result == 0) {
					free_memory_fetch_line(stdin_string, file_descriptors, command_counter);
					err(EXIT_FAILURE, "parse_command() failed");	
				} else if (parse_command_result == 2) {
					free_memory_fetch_line(stdin_string, file_descriptors, command_counter);
					return 0;
				}
			} else {
				int parse_command_result = parse_command(stdin_string, NULL, NULL);
				if (parse_command_result == 0) {
					free_memory_fetch_line(stdin_string, file_descriptors, command_counter);
					err(EXIT_FAILURE, "parse_command() failed");
				} else if (parse_command_result == 2) {
					free_memory_fetch_line(stdin_string, file_descriptors, command_counter);
					return 0;	
				}
			}	
			free_memory_fetch_line(stdin_string, file_descriptors, command_counter);
			printf("\n");
			return 1;
		} else if (ch == ';') {
			stdin_string[count_length_string] = '\0';
			if (command_counter > 0 && file_descriptors[command_counter-1]) {
				int parse_command_result = parse_command(stdin_string, NULL, file_descriptors[command_counter-1]);
				if (parse_command_result == 0) {
					free_memory_fetch_line(stdin_string, file_descriptors, command_counter);
					err(EXIT_FAILURE, "parse_command() failed");
				} else if (parse_command_result == 2) {
					free_memory_fetch_line(stdin_string, file_descriptors, command_counter);
					return 0;
				}
			} else {
				int parse_command_result = parse_command(stdin_string, NULL, NULL);
				if (parse_command_result == 0) {
					free_memory_fetch_line(stdin_string, file_descriptors, command_counter);
					err(EXIT_FAILURE, "parse_command() failed");
				} else if (parse_command_result == 2) {
					free_memory_fetch_line(stdin_string, file_descriptors, command_counter);
					return 0;
				}
			}	
			count_length_string = 0;
			command_counter++;
		} else if (ch == '|') {
			file_descriptors[command_counter] = (int*)malloc(2*sizeof(int));
			if (file_descriptors[command_counter] == NULL) {
				free_memory_fetch_line(stdin_string, file_descriptors, command_counter);
				err(EXIT_FAILURE, "malloc() failed when allocating file_descriptors[command_counter]");
			}
			if (pipe(file_descriptors[command_counter]) == -1) {
				free_memory_fetch_line(stdin_string, file_descriptors, command_counter);
				err(EXIT_FAILURE, "pipe() failed");
			}
			stdin_string[count_length_string] = '\0';
			if (command_counter > 0 && file_descriptors[command_counter-1]) {
				if (parse_command(stdin_string, file_descriptors[command_counter], file_descriptors[command_counter-1]) == 0) {
					free_memory_fetch_line(stdin_string, file_descriptors, command_counter);
					err(EXIT_FAILURE, "parse_command() failed");
				}
			} else {
				if (parse_command(stdin_string, file_descriptors[command_counter], NULL) == 0) {
					free_memory_fetch_line(stdin_string, file_descriptors, command_counter);
					err(EXIT_FAILURE, "parse_command() failed");
				}
			}	
			count_length_string = 0;
			command_counter++;
		} else {
			stdin_string[count_length_string] = ch;
			count_length_string++;
		}
	}

	free_memory_fetch_line(stdin_string, file_descriptors, command_counter);
	printf("\n");
	return 0;
}

int parse_command(char *stdin_input, int* write_end_pipe, int* read_end_pipe) {
	int COMMAND_SIZE = 20; 

	char *command;
	char *argument;
	int argument_index = 0;
	char **arguments = malloc(COMMAND_SIZE * sizeof(char*));

	argument = strtok(stdin_input, " ");

	while (argument != NULL) {
		if (argument_index >= COMMAND_SIZE) {
			COMMAND_SIZE = COMMAND_SIZE * 2;
			char** arguments_copy = realloc(arguments, COMMAND_SIZE * sizeof(char*));
			if (arguments_copy == NULL) {
				fprintf(stderr, "Memory allocation error!");
				free_memory_parse_command(arguments, argument_index, argument, command);
				return 0;
			}

			arguments = arguments_copy;
		}

		int argument_length = strlen(argument);
		arguments[argument_index] = malloc((argument_length + 1) * sizeof(char));
		strcpy(arguments[argument_index], argument);

		if (argument_index == 0) {
			command = arguments[argument_index];
		}

		argument_index++;
		argument = strtok(NULL, " ");
	}

	if (strcmp(command, "cd") == 0) {
		if (write_end_pipe != NULL || read_end_pipe != NULL) {
			free_memory_parse_command(arguments, argument_index, argument, command);
			if (write_end_pipe != NULL) {
				if (close_pipe_end(write_end_pipe[1]) == 0) {
					return 0;
				}
			}
			if (read_end_pipe != NULL) {
				if (close_pipe_end(read_end_pipe[0]) == 0) {
					return 0;
				}
			}
			return 1;
		}

		if (argument_index == 1) {
			const char *HOME = "HOME";
			const char *home_dir = getenv(HOME);
			int ret = chdir(home_dir);
			if (ret != 0) {
				fprintf(stderr, "There was an issue changing the directory");
				free_memory_parse_command(arguments, argument_index, argument, command);
				return 0;
			}
		} else if (argument_index == 2) {
			int ret = chdir(arguments[1]);
			if (ret != 0) {
				fprintf(stderr, "There was an issue changing the directory");
				free_memory_parse_command(arguments, argument_index, argument, command);
				return 0;
			}
		}

		free_memory_parse_command(arguments, argument_index, argument, command);
		return 1;
	} else if (strcmp(command, "exit") == 0) {
		free_memory_parse_command(arguments, argument_index, argument, command);

		if (write_end_pipe != NULL || read_end_pipe != NULL) {
			if (write_end_pipe != NULL) {
				if (close_pipe_end(write_end_pipe[1]) == 0) {
					return 0;
				}
			}
			if (read_end_pipe != NULL) {
				if (close_pipe_end(read_end_pipe[0]) == 0) {
					return 0;
				}
			}
			return 1;
		}

		return 2;
	} else if (strcmp(command, "echo") == 0) {
		if (read_end_pipe != NULL) {
			if (close_pipe_end(read_end_pipe[0]) == 0) {
				return 0;
			}
		}

		if (write_end_pipe != NULL) {
			for (int i = 1; i < argument_index; i++) {
				if (write(write_end_pipe[1], arguments[i], strlen(arguments[i])) == -1) {
					free_memory_parse_command(arguments, argument_index, argument, command);
					return 0;
				}
				if (write(write_end_pipe[1], " ", 1) == -1) {
					free_memory_parse_command(arguments, argument_index, argument, command);
					return 0;
				}
			}
		} else {
			for (int i = 1; i < argument_index; i++) {
				printf("%s ", arguments[i]);
			}
		}

		if (write_end_pipe != NULL) {
			if (close_pipe_end(write_end_pipe[1]) == 0) {
				return 0;
			}
		}

		free_memory_parse_command(arguments, argument_index, argument, command);
		return 1;
	} else if (strcmp(command, "pwd") == 0) {
		if (read_end_pipe != NULL) {
			if (close_pipe_end(read_end_pipe[0]) == 0) {
				return 0;
			}
		}

		char cwd[PATH_MAX];
		if (getcwd(cwd, sizeof(cwd)) != NULL) {
			if (write_end_pipe != NULL) {
				if (write(write_end_pipe[1], cwd, strlen(cwd)) == -1) {
					free_memory_parse_command(arguments, argument_index, argument, command);
					return 0;
				}
			} else {
				printf("%s", cwd);
			}	
		} else {
			fprintf(stderr, "getcwd() error");
			free_memory_parse_command(arguments, argument_index, argument, command);
			return 0;
		}

		if (write_end_pipe != NULL) {
			if (close_pipe_end(write_end_pipe[1]) == 0) {
				return 0;
			}
		}

		free_memory_parse_command(arguments, argument_index, argument, command);
		return 1;
	} else {
		find_path(command, arguments, write_end_pipe, read_end_pipe);
		return 1;
	}
}

void find_path(char *command, char **arguments, int* write_end_pipe, int* read_end_pipe) {
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

void execute_command(char *command_path, char **arguments, int* write_end_pipe, int* read_end_pipe) {
	pid_t pid = fork();

	if (pid < 0) {
		printf("fork has failed!");
		exit(1);
	} else if (pid == 0) {

		int dup2_write_end;
		int dup2_read_end;

		if (write_end_pipe != NULL) {
			close_pipe_end(write_end_pipe[0]);

			dup2_write_end = dup2(write_end_pipe[1], STDOUT_FILENO);
			if (dup2_write_end == -1) {
				err(EXIT_FAILURE, "dup2() command failed for redirection to STDIN_FILENO");
			}

			close_pipe_end(write_end_pipe[1]);
		}

		if (read_end_pipe != NULL) {
			close_pipe_end(read_end_pipe[1]);

			dup2_read_end = dup2(read_end_pipe[0], STDIN_FILENO);
			if (dup2_read_end == -1) {
				err(EXIT_FAILURE, "dup2() command failed redirection to STDOUT_FILENO");
			}

			close_pipe_end(read_end_pipe[0]);
		}

		execv(command_path, arguments);

		err(EXIT_FAILURE, "execv() failed");
	} else {
		if (write_end_pipe != NULL) {
			close_pipe_end(write_end_pipe[1]); 
		}
		if (read_end_pipe != NULL) {
			close_pipe_end(read_end_pipe[0]);
		}

		int stat_loc;
		waitpid(pid, &stat_loc, 0);

		if (!WIFEXITED(stat_loc)) {
			printf("%s\n", "Process has failed!");
		}
	}
}



