#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_LINE 1024
#define MAX_ARGS 15
#define MAX_COMMANDS 10
#define MAX_REDIR 3 // stdin, stdout, stderr

char *argvv[MAX_ARGS];
char *filev[MAX_REDIR];
int background = 0;

//Read one line at a time from the file
ssize_t read_line(int fd, char *linea, size_t max_len) {
    ssize_t bytes_read = 0;
    char c;
    while (bytes_read < max_len - 1 && read(fd, &c, 1) > 0) {
        if (c == '\n') {
            linea[bytes_read++] = c;
            linea[bytes_read] = '\0';  // null-terminate the string
            return bytes_read;
        }
        linea[bytes_read++] = c;
    }
    if (bytes_read > 0) {
        linea[bytes_read] = '\0';  // null-terminate the string
    }
    return bytes_read;
}

// Function to tokenize a command line
int tokenizar_linea(char *linea, char *delim, char *tokens[], int max_tokens) {
    int i = 0;
    char *token = strtok(linea, delim);
    while (token != NULL && i < max_tokens - 1) {
        tokens[i++] = token;
        token = strtok(NULL, delim);
    }
    tokens[i] = NULL;
    return i;
}

// Function to process redirections
void procesar_redirecciones(char *args[]) {
    filev[0] = filev[1] = filev[2] = NULL;
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "<") == 0) {
            filev[0] = args[i+1]; args[i] = args[i+1] = NULL; i++;
        } else if (strcmp(args[i], ">") == 0) {
            filev[1] = args[i+1]; args[i] = args[i+1] = NULL; i++;
        } else if (strcmp(args[i], "!>") == 0) {
            filev[2] = args[i+1]; args[i] = args[i+1] = NULL; i++;
        }
    }
}

// Function to execute a command
void execute_command(char *argvv[], int in_fd, int out_fd) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork failed");
        return;
    }
    if (pid == 0) { // Child process
        // Handle input redirection
        if (in_fd != STDIN_FILENO) {
            dup2(in_fd, STDIN_FILENO);
            close(in_fd);
        }

        // Handle output redirection
        if (out_fd != STDOUT_FILENO) {
            dup2(out_fd, STDOUT_FILENO);
            close(out_fd);
        }

        // Handle redirection for file descriptors
        if (filev[0]) {
            int fd_in = open(filev[0], O_RDONLY);
            if (fd_in < 0) { perror("Input redirection failed"); exit(EXIT_FAILURE); }
            dup2(fd_in, STDIN_FILENO); close(fd_in);
        }
        if (filev[1]) {
            int fd_out = open(filev[1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd_out < 0) { perror("Output redirection failed"); exit(EXIT_FAILURE); }
            dup2(fd_out, STDOUT_FILENO); close(fd_out);
        }
        if (filev[2]) {
            int fd_err = open(filev[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd_err < 0) { perror("Error redirection failed"); exit(EXIT_FAILURE); }
            dup2(fd_err, STDERR_FILENO); close(fd_err);
        }
        

        execvp(argvv[0], argvv);
        perror("Exec failed");
        exit(EXIT_FAILURE);
    }
}

// Function to process and execute a command line with pipes
void procesar_linea(char *linea) {
    char *comandos[MAX_COMMANDS];
    int num_comandos = tokenizar_linea(linea, "|", comandos, MAX_COMMANDS);

    if (strchr(comandos[num_comandos - 1], '&')) {
        background = 1;
        char *pos = strchr(comandos[num_comandos - 1], '&');
        *pos = '\0';
    }

    else background=0;

    int prev_fd = STDIN_FILENO;
    int pipefds[2];
    
    for (int i = 0; i < num_comandos; i++) {
        tokenizar_linea(comandos[i], " \t\n", argvv, MAX_ARGS);
        procesar_redirecciones(argvv);

        // Create a pipe for each command in the pipeline except for the last one
        if (i < num_comandos - 1) {
            if (pipe(pipefds) == -1) {
                perror("Pipe failed");
                return;
            }
        }

        // Execute the command
        execute_command(argvv, prev_fd, (i < num_comandos - 1) ? pipefds[1] : STDOUT_FILENO);

        // Close write end of pipe after use
        if (i < num_comandos - 1) {
            close(pipefds[1]);
        }

        // Update prev_fd to read from the pipe (for the next command in the pipeline)
        prev_fd = pipefds[0];
    }

    // Wait for the last process to finish if not in background mode
    if (!background) {
        waitpid(-1, NULL, 0);
    } else {
        printf("%d \n", getpid());
    }
}

// Main function
int main(int argc, char *argv[]) {
    if (argc != 2) {
        perror("Usage: <program> <script_file>");
        return -1;
    }
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("Error opening file");
        return -1;
    }
    char linea[MAX_LINE];
    ssize_t bytes_read;
    bytes_read = read_line(fd,linea,MAX_LINE);
    if (bytes_read <= 0   || strncmp(linea, "## Script de SSOO\n", 18) != 0) {
        perror("Invalid script format");
        return -1;
    }


    while (read_line(fd,linea,MAX_LINE) > 0) {
        if (linea[0] == '\n' || linea[0] == '\0') {
            wait(NULL);
            perror("Empty line detected");
            return -1;
            
        }
        procesar_linea(linea);
    }
    close(fd);
    return 0;
}
