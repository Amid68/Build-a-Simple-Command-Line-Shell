#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

// max len of input line
#define MAX_LINE 1024
// max num of arg
#define MAX_ARGS 64
// default prompt
#define DEFAULT_PROMPT "mysh$ "

void print_prompt(char * prompt);
char * read_input(void);
char ** parse_input(char * input);
int execute_command(char ** args);
void trim_whitespace(char * str);

int main(void) {
    char * prompt = DEFAULT_PROMPT;

    while(1) {
        print_prompt(prompt);

        char * input = read_input();
        if (input == NULL) {
            printf("\n");
            break;
        }

        if (strlen(input) == 0) {
            free(input);
            continue;
        }

        char ** args = parse_input(input);

        if (args[0] == NULL) {
            free(input);
            free(args);
            continue;
        }

        if (strcmp(args[0], "exit") == 0) {
            free(input);
            free(args);
            break;
        }

        execute_command(args);

        free(input);
        free(args);
    }

    return 0;
}


void print_prompt(char * prompt) {
    printf("%s", prompt);
    fflush(stdout); // display prompt immediately
}

char * read_input(void) {
    char * line = NULL;
    size_t bufsize = 0; // let getline() allocate the buffer

    ssize_t characters = getline(&line, &bufsize, stdin);
    if (characters == -1) {
        free(line);
        return NULL;
    }

    if (line[characters - 1] == '\n') {
        line[characters - 1] = '\0';
    }

    trim_whitespace(line);

    return line;
}

void trim_whitespace(char * str) {
    // trim leading whitespace
    char * start = str;
    while (*start && (*start == ' ' || *start == '\t')) {
        start++;
    }

    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }

    // trim trailing whitespace
    char * end = str + strlen(str) - 1;
    while (end >= str && (*end == ' ' || *end == '\t')) {
        *end = '\0';
        end--;
    }
}

char ** parse_input(char * input) {
    int bufsize = MAX_ARGS, position = 0;
    char ** tokens = malloc(bufsize * sizeof(char*));
    if (!tokens) {
        fprintf(stderr, "mysh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    char * token = strtok(input, " \t");
    while (token != NULL) {
        tokens[position++] = token;

        // realloc if buffer exceeded
        if (position >= bufsize) {
            bufsize += MAX_ARGS;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "mysh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, " \t");
    }

    tokens[position] = NULL;
    return tokens;
}

int execute_command(char ** args) {
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0) {
        if (execvp(args[0], args) == -1) {
            perror("mysh");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("mysh");
    } else {
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
            if (wpid == -1) {
                perror("mysh");
                break;
            }
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

















