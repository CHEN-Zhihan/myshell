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
    char *input = strdup(inp);
    char *tmp = strtok(input, delimiter);
    while (tmp) {
        if (!allSpace(tmp)) {
            if (flag) {
                output[i] = strdup(tmp);
            }
            ++i;
        }
        tmp = strtok(nullptr, delimiter);
    }
    free(input);
    return i;
}
// /home/zhchen/HKU/COMP3230/assignments/myshell/test/nested 5 &
PIDNode * buildPIDNode(pid_t inp) {
    pid_t pid=0;
    char * name=(char*)malloc(sizeof(char)*MAX_PROC_FILE_PATH);
    unsigned long ut, st;
    pid_t ppid;

    char str[MAX_PROC_FILE_PATH];
    sprintf(str, "/proc/%d/stat",inp);
    FILE *file = fopen(str, "r");
    if (file == NULL) {
        printf("Error in open my proc file: %d\n",inp);
        return nullptr;
    }
    int z;
    unsigned long h;
    char stat;
    fscanf(file, "%d %s %c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu", &pid, name, &stat, &ppid, &z, &z, &z, &z,
           (unsigned *)&z, &h, &h, &h, &h, &ut, &st);
    fclose(file);

    PIDNode * result = (PIDNode*)(malloc(sizeof(PIDNode)));
    result->PPID=ppid;
    result->PID=pid;
    result->name=name;
    result->child=nullptr;
    result->next=nullptr;
    return result;
}