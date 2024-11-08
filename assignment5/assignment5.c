/*
* File: assignment5.c
* Owner: 
* Date: 4.2.2024
* Description: Implementing a shell program in C
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "assignment5.h"

// Function to parse user input into command and arguments
void command_line_parse(char *input, char *command, char **arguments) {
    char *token = strtok(input, " ");
    strcpy(command, token);

    int arg_count = 0;
    while (token != NULL) {
        arguments[arg_count] = token;
        arg_count++;

        token = strtok(NULL, " ");
    }
    arguments[arg_count] = NULL;
}

void command_line_execute(char *command, char **arguments) {
    if (execvp(command, arguments) == -1) {
        perror("Command execution failed");
        exit(EXIT_FAILURE);
    }
}

int main() {
    char input[MAX_COMMAND_LENGTH];
    char command[MAX_COMMAND_LENGTH];
    char *arguments[MAX_ARGUMENTS];

    while (1) {
        printf("CSE320_SHELL> ");
        fflush(stdout);

        // Read user input
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }

        // Remove newline character
        input[strcspn(input, "\n")] = '\0';

        // TODO: Exit the shell if the user enters "exit"
        if (strcmp(input, "exit") == 0) {
            break;
        }

        // Parse the input into command and arguments
        command_line_parse(input, command, arguments);

        int pipes[2]; // Used for pipe if needed

        // Check for input/output redirection
        int input_fd = 0; // Default to stdin
        int output_fd = 1; // Default to stdout
	    int pipe_flag = 0; // 0: no pipe, 1: pipe instruction

        for (int i = 0; arguments[i] != NULL; i++) {
            if (strcmp(arguments[i], "<") == 0) {
		        printf("Input Redirection Detected!\n");
                // Input redirection
                arguments[i] = NULL; // Remove "<" from the argument list
                input_fd = open(arguments[i + 1], O_RDONLY);
                if (input_fd == -1) {
                    perror("Input redirection failed");
                    exit(EXIT_FAILURE);
                }
                i++;
            } else if (strcmp(arguments[i], ">") == 0) {
		        printf("Output Redirection Detected!\n");
                // Output redirection
                arguments[i] = NULL; // Remove ">" from the argument list
                output_fd = open(arguments[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0666);
                if (output_fd == -1) {
                    perror("Output redirection failed");
                    exit(EXIT_FAILURE);
                }
                i++;
            } else if (strcmp(arguments[i], "|") == 0) {
		        printf("Pipe Detected!\n");
                // Pipe
                arguments[i] = NULL; // Split the command into two parts
                if (pipe(pipes) == -1) {
                    perror("Pipe creation failed");
                    exit(EXIT_FAILURE);
                }
                
		        pipe_flag = 1;
                // TODO: Fork children processes for the left side of the pipe
		        // and the right side of the pipe
                pid_t pid = fork();
                if (pid < 0) {
                    perror("Fork failed");
                    exit(EXIT_FAILURE);
                } else if (pid == 0) {
                    // Child process
                    // Redirect input if needed
                    if (input_fd != 0) {
                        printf("Redirecting input\n");
                        dup2(input_fd, 0);
                        close(input_fd);
                    }
                } else {
                    // Parent process
                    // Redirect output if needed
                    if (output_fd != 1) {
                        printf("Redirecting output\n");
                        dup2(output_fd, 1);
                        close(output_fd);
                    }
                    command_line_execute(command, arguments);
                }
            }
        }

        // Fork a child process to execute the command except the pipe command
	    if (!pipe_flag) {
            pid_t pid = fork();

            if (pid < 0) {
                perror("Fork failed");
                exit(EXIT_FAILURE);
            } else if (pid == 0) {
                // Child process
                // Redirect input if needed
                if (input_fd != 0) {
                    dup2(input_fd, 0);
                    close(input_fd);
                }

                // Redirect output if needed
                if (output_fd != 1) {
                    dup2(output_fd, 1);
                    close(output_fd);
                }
                command_line_execute(command, arguments);
            } else {
                // Parent process
	            // TODO: Wait child process and check if it is exited or terminated
                int status;
                waitpid(pid, &status, 0);
                if (WIFEXITED(status)) {
                    printf("Child process exited with status %d\n", WEXITSTATUS(status));
                } else if (WIFSIGNALED(status)) {
                    printf("Child process terminated by signal %d\n", WTERMSIG(status));
                }
            }
	    }
    }

    return 0;
}