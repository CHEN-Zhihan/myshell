#include "internalcmd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "execute.h"
#include "parser.h"



void print_timeX(int pid) {
    int pid_get;
    char cmd[MAX_PROC_FILE_PATH];
    unsigned long ut, st;


    char str[MAX_PROC_FILE_PATH];
    sprintf(str, "/proc/%d/stat", pid);
    FILE *file = fopen(str, "r");
    if (file == NULL) {
        printf("Error in open my proc file\n");
        exit(0);
    }

    int z;
    unsigned long h;
    char stat;

    fscanf(file, "%d %s %c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu", &pid_get, cmd, &stat, &z, &z, &z, &z, &z,
           (unsigned *)&z, &h, &h, &h, &h, &ut, &st);
    fclose(file);

    cmd[strlen(cmd) - 1] = 0;


    double utime = ut*1.0f/sysconf(_SC_CLK_TCK);
    double stime = st*1.0f/sysconf(_SC_CLK_TCK);

    printf("\n");
    printf("%-10s%-15s%-10s%-10s%-10s\n", "PID", "CMD", "RTIME", "UTIME", "STIME");
    printf("%-10d%-15s%-4.2lf%-6s%-4.2lf%-6s%-4.2lf%-6s\n", pid_get, cmd+1, utime + stime, " s", utime," s", stime," s");

}



int internal_command(char *buffer) {
    if (buffer[0] == 'c' && buffer[1] == 'd' && buffer[2] == ' ') {
        if (chdir(buffer + 3) < 0) {
            fprintf(stderr, "can not cd to %s\n", buffer+3);
        }
        return 1;
    }
    if (buffer[0] == 'e' && buffer[1] == 'x' && buffer[2] == 'i' && buffer[3] == 't') {
        if (allspace(buffer + 4)) {
            fprintf(stderr, "myshell: Terminated\n");
            exit(0);
        }else{
            if (buffer[4] == ' ') {
                fprintf(stderr, "myshell: \"exit\" with other arguments!!!\n");
                return 1;
            }
            return 0;
        }
        return 1;
    }
    if (buffer[0] == 't' && buffer[1] == 'i' && buffer[2] == 'm' && buffer[3] == 'e' && buffer[4] == 'X' && buffer[5] == ' ') {
        if (allspace(buffer + 6)) {
            fprintf(stderr, "myshell: \"timeX\" cannot be a standalone command\n");
            return 1;
        }
        if (background(buffer)) {
            fprintf(stderr, "myshell: \"timeX\" cannot be run in background mode\n");
            return 1;
        }
        int is_background = 0;
        struct Command *cmd = parse_cmd(buffer + 6, &is_background);
        if (cmd != NULL) {
            runcommand(cmd, timeX, is_background);
        }
        return 1;
    }


    return 0;
}
