#include <stdio.h>
#include <string.h>
#include "util.h"
#include "parser.h"
#include "execute.h"
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
void SIGCHLD_handler(int signal) {
    int pid = waitpid(-1, NULL, 0);
    printf("%d, %d\n", pid, getpgid(pid));
    if (pid == getpgid(pid)) {
        printf("[%d] %s Done",pid,"wtf\0");
    }
}

void SIGCHLD_handler_wrapper() {
    struct sigaction act;
    memset(&act, 0, sizeof(sigaction));
    act.sa_handler = SIGCHLD_handler;
    act.sa_flags = SA_NOCLDSTOP;
    sigaction(SIGCHLD, &act, NULL);
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
    if (input == nullptr) {
        fprintf(stdout, "\n");
        return false;
    }
    if (split_input(input,nullptr," ",false)>MAX_ARGS_NUMBER) {
        fprintf(stderr,"myshell: Too many arguments\n");
        return false;
    }
    return true;
}

void printCmd(Command * cmd) {
    printf("executing %s with %d arguments: ",cmd->argv[0],cmd->argc-1);
    int i=0;
    for (i=1;i!=cmd->argc;++i) {
        printf("%s ", cmd->argv[i]);
    }
    printf("\n");
}

void printLine(Line * line) {
    if (line->type==TIMEX_TYPE) {
        printf("timeXing\n");
    } else if (line->type == VIEWTREE_TYPE) {
        printf("view tree\n");
    }
    Command * iterator= line->head;
    while (iterator!=nullptr) {
        printCmd(iterator);
        iterator=iterator->next;
        if (iterator!=nullptr) {
            printf("piping \n");
        }
    }
    if (line->background) {
        printf("in background\n");
    }
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
                printLine(line);
                execute(line);
            }
            free(input);
        }
    }
}