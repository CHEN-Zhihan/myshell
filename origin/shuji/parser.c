#include "parser.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "execute.h"
extern int flag;

int get_command (char *buffer) {
    fprintf(stdout, "## myshell $ ");
    memset(buffer, 0,MAX_COMMAND_SIZE * sizeof(char));

    char *s = fgets(buffer, MAX_COMMAND_SIZE, stdin);

    if(s == NULL) {
        if (flag == 1) {
            flag = 0;
            fprintf(stdout, "\n");
            return 0;
        }

        if (flag == 0) {
            fprintf(stdout, "\n");
            return -1;
        }
    }
    char ch;
    if(buffer[strlen(buffer)-1] != '\n') {
        while ((ch = getchar()) != '\n' && ch != EOF);
        fprintf(stderr, "The command line is upper-bounded by 1024 characters\n");
        return 0;
    }
    char tmp[MAX_COMMAND_SIZE];
    strcpy(tmp, buffer);
    if(split_input(tmp, NULL, " ", NO_RESULT) > MAX_TOKEN_NUMBER) {
        fprintf(stderr, "The command line is upper-bounded by 30 arguments\n");
        return 0;
    }
    return 1;
}


struct Command *parse_cmd(char *input, int *is_background) {
    *is_background = background(input);
    if(countchar(input, '&')> 0) {
        fprintf(stderr, "\'&\' should not appear in the middle of hte command line\n");
        return NULL;
    }

    return (struct Command *)parse_pipe(input);

}

struct Command *parse_pipe(char *input) {

    char *token[MAX_TOKEN_NUMBER];
    int error = countchar(input, '|');
    int number = split_input(input, token, "|", GET_RESULT);
    if ((error > 0) && (number - error != 1)) {
        fprintf(stderr, "Wrong number of \'|\'\n");
        return NULL;
    }
    //printf("the number is %d\n", number);
    if (number < 1) {
        return NULL;
    }else if (number == 1) {
        return (struct Command *)parse_exec(token[0]);
    }

    return make_pipe_from_toke(token, 0, number - 1);


}

struct Command *make_pipe_from_toke(char **token, int i, int number) {
    if (i == number) {
        return (struct Command *)parse_exec(token[i]);
    }else {
        return (struct Command *)Pipe_Command((struct Command *)parse_exec(token[i]), make_pipe_from_toke(token, i+1, number));
    }
}

struct Command *parse_exec(char *input) {
    struct Exec_Command *exec_command = (struct Exec_Command*)Exec_Command();
    int number = split_input(input, exec_command->argv, " ", GET_RESULT);
    exec_command->argv[number] = 0;
    return (struct Command *)exec_command;
}




int countchar (char* input, int delimiter) {
    int i = 0;
    for ( ; *input ; ++input) {
        if (*input == delimiter) {
            i++;
        }
    }
    return i;
}


int split_input(char *input, char **output, char *delimiter, int flag) {
    int i = 0;
    char *tmp = strtok(input, delimiter);
    while (tmp) {
        if(!allspace(tmp)) {
            if (flag) {
                output[i] = malloc(strlen(tmp) * sizeof(char));
                strcpy(output[i],tmp);
            }
            i = i + 1;
        }
        tmp = strtok(NULL, delimiter);
    }
    return i;
}

int allspace(char *s) {
    if (strlen(s) == 0) {
        return 1;
    }
    for(int i = 0; i < strlen(s) ; ++i) {
        if (s[i] != ' ') {
            return 0;
        }
    }
    return 1;
}

int background(char *buffer) {
    int i;
    for (i = strlen(buffer)-1; i >= 0; --i) {
        if (buffer[i] == ' ') {
            buffer[i] = 0;
            continue;
        }
        if (buffer[i] == '&') {
            buffer[i] = 0;
            return 1;
        }
        return 0;
    }
    return 0;
}
