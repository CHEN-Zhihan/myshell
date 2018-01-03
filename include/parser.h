#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>

#define MAX_ARGS_NUMBER 30
#define MAX_ARG_LENGTH 30
#define BUFFER_SIZE 1000
#define MAX_NO_CMD_PER_LINE 30
#define nullptr NULL

#define DEBUG 1

typedef struct Command {
    int argc;
    const char * * argv;
} Command;

typedef enum redirectToFile {
    NO_REDIRECT, REDIRECT, REDIRECT_APPEND
} redirectToFile;


typedef struct Line {
    bool background;
    redirectToFile redirect[2];
    const char * redirectFile[2];
    int noCmd;
    const Command * cmds[MAX_NO_CMD_PER_LINE];
} Line;

const Line * parse(char *);
void freeLine(const Line * );
#endif
