#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

// Function to search for a term in the specified file using system calls
void search_file(const char *file_path, const char *search_term) {
    int file_desc = open(file_path, O_RDONLY); // Open file in read-only mode
    if (file_desc == -1) {
        perror("Unable to open file");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    char line[BUFFER_SIZE];
    int line_index = 0, bytes_read, current_line = 1;

    while ((bytes_read = read(file_desc, buffer, BUFFER_SIZE)) > 0) {
        for (int i = 0; i < bytes_read; i++) {
            if (buffer[i] == '\n' || line_index >= BUFFER_SIZE - 1) {
                line[line_index] = '\0'; // Null-terminate the string
                if (strstr(line, search_term)) {
                    printf("%d: %s\n", current_line, line);
                }
                line_index = 0;
                current_line++;
            } else {
                line[line_index++] = buffer[i];
            }
        }
    }
    
    close(file_desc); // Close the file descriptor
}

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: %s <file_path> <search_term>\n", argv[0]);
        return -1;
    }

    search_file(argv[1], argv[2]);
    return 1;
}