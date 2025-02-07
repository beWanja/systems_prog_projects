#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h> 
#include <sys/time.h>

#define MAX_WORD_LENGTH 256  // Maximum length for a word
#define BUFFER_SIZE 1024     // Buffer size for reading input -- read line by line

// Helper func to convert words to lowercase
void to_lowercase(char *word) {
    for (int i = 0; word[i]; i++) {
        word[i] = tolower(word[i]);
    }
}
// helper function to tokenize words
void preprocess_input(char *buffer) {
    for (int i = 0; buffer[i] != '\0'; i++) {
        if (!isalnum((unsigned char)buffer[i])) {
            buffer[i] = ' ';
        }
    }
}

// helper function to check validity of word token
int check_for_word(const char *token) {
    while (*token) {
        if (isalnum((unsigned char)*token)) {
            return 1;
        } else if (*token == '-' || *token == '–' || *token == '—') {
            return 0; 
        }
        token++;
    }
    return 0;
}


void parse_input(int pipe_fd, int short_len, int long_len) {
    char buffer[BUFFER_SIZE];  // Buffer for reading input from stdin
    FILE *sorter_stream = fdopen(pipe_fd, "w");
    if (sorter_stream == NULL) {
        perror("fdopen");
        exit(1);
    }

    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    if (flags != -1) {
        fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
    }

    int read_count = 0;  // Counter for number of reads
    const int MAX_READS = 1000000;  // Limit for reads to avoid indefinite looping for very large files

    // Read input line by line
    while (fgets(buffer, BUFFER_SIZE, stdin)) {
        preprocess_input(buffer);  // Preprocess the buffer to remove punctuation and other elements    
        read_count++;
        // printf("Reading line %d: %s", read_count, buffer);  // Debug print

        if (read_count > MAX_READS) {
            fprintf(stderr, "Error: Exceeded maximum read count, breaking loop.\n");
            break;
        }
       
        char *word = strtok(buffer, " \t\n"); 
        while (word != NULL && check_for_word(word)) {
            int len = strlen(word);
            
            //printf("word in parse_input: %s \n", word); //DEBUGGING PRINTS

            // Convert the word to lowercase for case-insensitive comparison
            to_lowercase(word);

            // Filter words based on `short_len` and `long_len`
            if (len > short_len) {
                if (len > long_len) {
                    word[long_len] = '\0';  // Truncate the word if it exceeds `long_len`
                }
                // Write the word to the sorter pipe
                fprintf(sorter_stream, "%s\n", word);
                if (ferror(sorter_stream)) {
                    perror("Error writing to sorter stream");
                    break;
                }
            }

            // Get the next word from the input line
            word = strtok(NULL, " \t\n"); 
        }
    }

    // Close the write end of the pipe after writing all input words
    fclose(sorter_stream);
}

// Count words and print at terminal
void count_words(int pipe_fd) {

    FILE *sorter_output = fdopen(pipe_fd, "r");
    if (sorter_output == NULL) {
        perror("fdopen");
        exit(1);
    }
        
    char word[MAX_WORD_LENGTH];
    char prev_word[MAX_WORD_LENGTH] = "";
    int word_count = 0;
    
    // Read from sorter output and count unique words
    while (fgets(word, MAX_WORD_LENGTH, sorter_output)) {
        
        word[strcspn(word, "\n")] = '\0';  // Remove newline
        // Compare with the previous word for counting because sorting has already been done
        if (strcmp(word, prev_word) == 0) {
            word_count++;
        } else {
            // Print the previous word and its count if it exists
            if (prev_word[0] != '\0') {
                printf("%-10d%s\n", word_count, prev_word);  // Print count and word
            }
            //printf("word in counting: %s \n", prev_word);
            // Update the previous word to the current word
            strcpy(prev_word, word);
            word_count = 1;  // Reset the count for the new word
        }
    }
    
    // Print the last word and its count
    if (prev_word[0] != '\0') {
        printf("%-10d%s\n", word_count, prev_word);
    }

    fclose(sorter_output);
}

int main(int argc, char *argv[]) {
    int opt;
    int short_len = 0; 
    int long_len = MAX_WORD_LENGTH;

    // Parse command-line options with getopt
    while ((opt = getopt(argc, argv, "n:s:l:")) != -1) {
        switch (opt) {
            case 'n': // sorter == 1 by default
                break;
            case 's':
                short_len = atoi(optarg);  // minimum word length
                if (short_len < 0) {
                    fprintf(stderr, "Invalid short length\n");
                    exit(1);
                }
                break;
            case 'l':
                long_len = atoi(optarg);  // maximum word length
                if (long_len <= 0) {
                    fprintf(stderr, "Invalid long length\n");
                    exit(1);
                }
                break;
            default:
                // Print usage information 
                fprintf(stderr, "Usage: pipesort [-n count] [-s short] [-l long]\n");
                exit(1);
        }
    }

    // pipes fo
    int parse_to_sort_pipe[2];
    int sort_to_count_pipe[2];
    
    if (pipe(parse_to_sort_pipe) < 0 || pipe(sort_to_count_pipe) < 0) {
        perror("pipe");
        exit(1);
    }

    // Fork the sorter process
    pid_t sorter_pid = fork();
    if (sorter_pid < 0) {
        perror("fork");
        exit(1);
    } else if (sorter_pid == 0) { // Child process --  sorter  (main is parent)
        close(parse_to_sort_pipe[1]); // Close write end of the first pipe
        close(sort_to_count_pipe[0]); // Close read end of the second pipe
        if (dup2(parse_to_sort_pipe[0], STDIN_FILENO) < 0 || dup2(sort_to_count_pipe[1], STDOUT_FILENO) < 0) {
            perror("dup2");
            exit(1);
        }
        close(parse_to_sort_pipe[0]);
        close(sort_to_count_pipe[1]);

        execl("/usr/bin/sort", "sort", NULL);
        perror("execl");
        exit(1);
    }

    // Parent process continues
    close(parse_to_sort_pipe[0]); // Close read end of the first pipe
    close(sort_to_count_pipe[1]); // Close write end of the second pipe

    parse_input(parse_to_sort_pipe[1], short_len, long_len);
    close(parse_to_sort_pipe[1]); // Close write end after parsing input

    // Continue to counting words after sorter completes
    count_words(sort_to_count_pipe[0]);
    close(sort_to_count_pipe[0]);

    return 0;
}
