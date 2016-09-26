#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "util.h"

#define BUFFER_SIZE 1024
#define MAX_ARGS_NUMBER 30
#define VALID 0
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


bool get_command(char * buffer) {
    memset(buffer, 0,BUFFER_SIZE * sizeof(char));
    char * input = fgets(buffer,BUFFER_SIZE,stdin);
    if(input == nullptr) {
        return false;
    }
    if (input[strlen(input)-1]!='\n') {
        fprintf(stderr, "myshell: '%s': file name too long\n",input);
        return false;
    }
    if (split_input(input,nullptr," ",false)>MAX_ARGS_NUMBER) {
        fprintf(stderr,"myshell: Too many arguments\n");
        return false;
    }
    return true;
}


int main(int argc, char const *argv[]) {
    char buffer[BUFFER_SIZE];
    while (true) {
        fprintf(stdout, "## myshell $ ");
        while (!get_command(buffer));
            char * input = strndup(buffer,strlen(buffer) - 1);
            Line * line = parse(input);
            if (line) {
                execute(line);
            }
            free(input);
    }
}