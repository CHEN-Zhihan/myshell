#ifndef UTIL_H
#define UTIL_H
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#ifndef nullptr
#define nullptr NULL
#endif

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

#ifndef bool
#define bool int
#endif

#define BUFFER_SIZE 1024
#define MAX_ARGS_NUMBER 30
#define pipe_out(pipefd) close(pipefd[0]);dup2(pipefd[1],STDOUT_FILENO);close(pipefd[1]);
#define pipe_in(pipefd) close(pipefd[1]);dup2(pipefd[0], STDIN_FILENO);close(pipefd[0]);
#define close_pipe(pipefd) close(pipefd[0]);close(pipefd[1]);

#define MAX_PIPE_NUMBER 5
#define MAX_ARG_NUMBER 100

typedef struct Command {
    int argc;
    char * argv[MAX_ARGS_NUMBER];
    struct Command *next;
} Command;

typedef struct Line {
    int type;
    int background;
    Command * head;
} Line;

bool allSpace(char *input);

int split_input(char *input, char **output, char *delimiter, bool flag);

#endif