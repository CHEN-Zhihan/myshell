#include "util.h"


bool allSpace(char * input) {
    int i=0;
    while (input[i]!='\0') {
        if (input[i]!=' ') {
            return false;
        }
        ++i;
    }
    return true;
}


int split_input(char *inp, char **output, char *delimiter, bool flag) {
    int i = 0;
    char * input = strdup(inp);
    char *tmp = strtok(input, delimiter);
    while (tmp) {
        if(!allSpace(tmp)) {
            if (flag) {
                output[i]=strdup(tmp);
            }
            ++i;
        }
        tmp = strtok(nullptr, delimiter);
    }
    free(input);
    return i;
}