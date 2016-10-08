#include <stdio.h>
#include <string.h>
#include "util.h"
#include "parser.h"
#include "execute.h"


void SIGCHLD_handler() {
    int pid = waitpid(-1, NULL, 0);
    if (pid == getgpid(pid)) {
        printf("zhe ge shi hou tai a\n");
    }
}

void SIGCHLD_handler_wrapper() {
    struct sigaction act;
    memset(&act, 0, sizeof(sigaction));
    act.sa_handler = SIGCHLD_handler;
    act.sa_flags = SA_NOCLDSTOP;
    sigaction(SIGINT, &act, NULL);
}


void SIGINT_handler_wrapper() {
    struct sigaction act;
    memset(&act, 0, sizeof(sigaction));
    act.sa_handler = SIG_IGN;
    act.sa_flags = 0;
    sigaction(SIGINT, &act, NULL);
}






bool get_command(char * buffer) {
    memset(buffer, 0,BUFFER_SIZE * sizeof(char));
    char * input = fgets(buffer,BUFFER_SIZE,stdin);
    if(input == nullptr) {
        fprintf(stdout, "\n");
        return false;
    }
    if (split_input(input,nullptr," ",false)>MAX_ARGS_NUMBER) {
        fprintf(stderr,"myshell: Too many arguments\n");
        return false;
    }
    return true;
}

int main(int argc, char const *argv[]) {
    
    SIGINT_handler_wrapper();
    SIGCHLD_handler_wrapper();

    char buffer[BUFFER_SIZE];
    while (true) {
        fprintf(stdout, "## myshell $ ");
        if (get_command(buffer)) {
            char * input = strndup(buffer,strlen(buffer) - 1);
            Line * line = parse(input);
            if (line) {
                execute(line);
            }
            free(input);
        }
    }
}