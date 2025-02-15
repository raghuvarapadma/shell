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
int find_path(char* command, char **arguments, int* write_end_pipe, int* read_end_pipe);
int execute_command(char *command_path, char **arguments, int* write_end_pipe, int* read_end_pipe);
void free_memory_fetch_line(char* stdin_string, int** file_descriptors, int command_counter);
void free_memory_parse_command(char** arguments, int arguments_length, char* argument, char* command);
void free_memory_find_path(char* path_result, char* search_path);
int close_pipe_end(int fd);
int call_parse_command(char seperator, char* stdin_string, int** file_descriptors, int command_counter);

int main() {
	run_program();
}

void run_program() {
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
	int command_counter = 0;
	int count_length_string = 0;

	char *stdin_string = ( char* )malloc( BUFFER_SIZE * sizeof( char ) );
	if (stdin_string == NULL) {
		fprintf(stderr, "malloc() error with stdin_string\n");
		free_memory_fetch_line(NULL, NULL, command_counter);
		return 1;
	}


	int **file_descriptors = (int**)malloc(BUFFER_SIZE * sizeof(int*));
	if (file_descriptors == NULL) {
		fprintf(stderr, "malloc() error with file_descriptors\n");
		free_memory_fetch_line(stdin_string, NULL, command_counter);
		return 1;
	}

	printf("\n> ");

	while ((ch = getchar()) != EOF) {
		if (count_length_string >= BUFFER_SIZE) {
			BUFFER_SIZE = BUFFER_SIZE * 2;
			char *stdin_string_copy = (char*) realloc(stdin_string, BUFFER_SIZE * sizeof( char ) );
			if (stdin_string_copy == NULL) {
				fprintf(stderr, "realloc() error with stdin_string");
				free_memory_fetch_line(stdin_string, file_descriptors, command_counter);
				return 1;
			}

			stdin_string = stdin_string_copy;	
		}


		if (ch == '\n') {
			stdin_string[count_length_string] = '\0';
			if (call_parse_command('\n', stdin_string, file_descriptors, command_counter) == 0) {
				return 1;
			}
			free_memory_fetch_line(stdin_string, file_descriptors, command_counter);
			return 1;
		} else if (ch == ';') {
			stdin_string[count_length_string] = '\0';
			if (call_parse_command(';', stdin_string, file_descriptors, command_counter) == 0) {
				return 1;
			}
			count_length_string = 0;
			command_counter++;
		} else if (ch == '|') {
			file_descriptors[command_counter] = (int*)malloc(2*sizeof(int));
			if (file_descriptors[command_counter] == NULL) {
				fprintf(stderr, "malloc() error with file_descriptors[command_counter]\n");
				free_memory_fetch_line(stdin_string, file_descriptors, command_counter);
				return 1;
			}
			if (pipe(file_descriptors[command_counter]) == -1) {
				fprintf(stderr, "pipe() failed");
				free_memory_fetch_line(stdin_string, file_descriptors, command_counter);
				return 1;
			}
			stdin_string[count_length_string] = '\0';
			if (call_parse_command('|', stdin_string, file_descriptors, command_counter) == 0) {
				return 1;
			}
			count_length_string = 0;
			command_counter++;
		} else {
			stdin_string[count_length_string] = ch;
			count_length_string++;
		}
	}

	free_memory_fetch_line(stdin_string, file_descriptors, command_counter);
	return 0;
}

int parse_command(char *stdin_input, int* write_end_pipe, int* read_end_pipe) {
	int COMMAND_SIZE = 20; 

	char *command;
	char *argument;
	int argument_index = 0;
	char **arguments = malloc(COMMAND_SIZE * sizeof(char*));
	if (arguments == NULL) {
		fprintf(stderr, "malloc() error with arguments\n");
		free_memory_parse_command(arguments, argument_index, argument, command);
		return 0;
	}

	argument = strtok(stdin_input, " ");

	while (argument != NULL) {
		if (argument_index >= COMMAND_SIZE) {
			COMMAND_SIZE = COMMAND_SIZE * 2;
			char** arguments_copy = realloc(arguments, COMMAND_SIZE * sizeof(char*));
			if (arguments_copy == NULL) {
				fprintf(stderr, "realloc() error with arguments\n");
				free_memory_parse_command(arguments, argument_index, argument, command);
				return 0;
			}

			arguments = arguments_copy;
		}

		int argument_length = strlen(argument);
		arguments[argument_index] = malloc((argument_length + 2) * sizeof(char));
		if (arguments[argument_index] == NULL) {
			fprintf(stderr, "malloc() error with arguments[argument_index]");
			free_memory_parse_command(arguments, argument_index, argument, command);
			return 0;
		}
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

		int ret;

		if (argument_index == 1) {
			const char *HOME = "HOME";
			const char *home_dir = getenv(HOME);
			ret = chdir(home_dir);
		} else if (argument_index == 2) {
			ret = chdir(arguments[1]);
		}

		if (ret != 0) {
			fprintf(stderr, "There was an issue changing the directory\n");
			free_memory_parse_command(arguments, argument_index, argument, command);
			return 0;
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
				strcat(arguments[i], " ");
				if (write(write_end_pipe[1], arguments[i], strlen(arguments[i])) == -1) {
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
			fprintf(stderr, "getcwd() error\n");
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
		int find_path_result = find_path(command, arguments, write_end_pipe, read_end_pipe);
		free_memory_parse_command(arguments, argument_index, argument, command);
		if (find_path_result == 0) {
			fprintf(stderr, "Call to find_path() failed\n");
			return 0;	
		} 
		return 1;
	}
}

int find_path(char *command, char **arguments, int* write_end_pipe, int* read_end_pipe) {
	char *path = "PATH";

	int command_len = strlen(command);
	char *get_env = getenv(path);
	char *path_result = malloc(strlen(get_env) * sizeof(char));
	if (path_result == NULL) {
		fprintf(stderr, "malloc() error with path_result\n");
		free_memory_find_path(path_result, NULL);
		return 0;
	}

	strcpy(path_result, getenv(path));

	char *location;
	char *search_path;
	int access_allowed = -1;

	location = strtok(path_result, ":");

	while (location != NULL) {
		int location_len = strlen(location);
		int total_len = command_len + location_len + 2;

		search_path = malloc(total_len * sizeof(char));
		if (search_path == NULL) {
			fprintf(stderr, "malloc() error with search_path\n");
			free_memory_find_path(path_result, search_path);
			return 0;
		}

		strcat(search_path, location);
		strcat(search_path, "/");
		strcat(search_path, command);

		access_allowed = access(search_path, F_OK);
		if (access_allowed == 0) {
			break;
		}
		else {
			free(search_path);
			search_path = NULL;
			location = strtok(NULL, ":");
		}
	}

	if (access_allowed == -1) {
		fprintf(stderr, "Could not find path for command\n");
		free_memory_find_path(path_result, search_path);
		return 1;
	} else {
		int execute_command_result = execute_command(search_path, arguments, write_end_pipe, read_end_pipe);	
		free_memory_find_path(path_result, search_path);
		if (execute_command_result == 0) {
			fprintf(stderr, "execute_command() call failed\n");
			return 0;
		}
		return 1;
	}
}

int execute_command(char *command_path, char **arguments, int* write_end_pipe, int* read_end_pipe) {
	pid_t pid = fork();

	if (pid < 0) {
		fprintf(stderr, "fork() has failed\n");
		return 0;	
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

		err(EXIT_FAILURE, "execv() failed in child process");
	} else {
		if (write_end_pipe != NULL) {
			close_pipe_end(write_end_pipe[1]); 
		}
		if (read_end_pipe != NULL) {
			close_pipe_end(read_end_pipe[0]);
		}

		int stat_loc;
		waitpid(pid, &stat_loc, 0);

		if (WIFEXITED(stat_loc)) {
			int exit_status = WEXITSTATUS(stat_loc);
			if (exit_status != 0) {
				fprintf(stderr, "Child process exited with error\n");
				return 0;
			}
			return 1;
		} else {
			fprintf(stderr, "Child process did not exit correctly\n");
			return 0;
		}
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

void free_memory_find_path(char* path_result, char* search_path) {
	if (path_result != NULL) {
		free(path_result);
	}
	if (search_path != NULL) {
		free(search_path);
	}
}

int close_pipe_end(int fd) {
	if (fcntl(fd, F_GETFD) == -1) {
		if (errno != EBADF) {
			fprintf(stderr, "fcntl() command failed\n");
			return 0;
		}
	} else {
		if (close(fd) == -1) {
			fprintf(stderr, "close() command failed\n");
			return 0; 
		}
	}
	return 1;
}

int call_parse_command(char seperator, char* stdin_string, int** file_descriptors, int command_counter) {
	int parse_command_result;

	if (seperator == ';' || seperator == '\n') {
		if (command_counter > 0 && file_descriptors[command_counter-1]) {
			int parse_command_result = parse_command(stdin_string, NULL, file_descriptors[command_counter-1]);
		} else {
			int parse_command_result = parse_command(stdin_string, NULL, NULL);
		}
	} else if (seperator == '|') {
		if (command_counter > 0 && file_descriptors[command_counter-1]) {
			parse_command_result = parse_command(stdin_string, file_descriptors[command_counter], file_descriptors[command_counter-1]);
		} else {
			 parse_command_result = parse_command(stdin_string, file_descriptors[command_counter], NULL);
		}
	} 

	if (parse_command_result == 0) {
		fprintf(stderr, "parse_command() failed\n");
		free_memory_fetch_line(stdin_string, file_descriptors, command_counter);
		return 0;
	} else if ( (seperator == '\n' || seperator == ';') && parse_command_result == 2) {
		printf("%s", "Goodbye!\n");
		free_memory_fetch_line(stdin_string, file_descriptors, command_counter);
		exit(0);
	}

	return 1;
}

