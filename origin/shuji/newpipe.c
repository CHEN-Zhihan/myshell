#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "execute.h"
#include "parser.h"
#include "internalcmd.h"


int flag = 0;


void SIGINT_handler_wrapper();
void SIGINT_handler(int signo);

void SIGINT_handler_wrapper() {
    struct sigaction act;
    memset(&act, 0, sizeof(sigaction));
    act.sa_handler = SIGINT_handler;
    act.sa_flags = 0;
    sigaction(SIGINT, &act, NULL);
}

void SIGINT_handler(int signo) {
    flag = 1;
}

void SIGCHLD_handler() {
    siginfo_t info;
    //waitid(P_ALL, 0 , &info, WNOWAIT|WNOHANG);
    //printf("----%d----%d\n", info.si_code, info.si_status);
/*
    int i = 0;
    while(pid_timeX[i] != 0) {
        if (pid_timeX[i] == info.si_pid) {
            print_timeX(info.si_pid)
        }
    }
    waitpid(info.si_pid, NULL, 0);*/
    //printf("Program terminated.\n");

}



int main(int argc, char const *argv[]) {

    signal(SIGCHLD, SIGCHLD_handler);
    SIGINT_handler_wrapper();

    char buffer[MAX_COMMAND_SIZE];
    int is_background = 0;
    struct Command* cmd;

    while (1) {
        while(waitpid(-1, NULL, 0) > 0);
        int error = get_command(buffer);
        if (error == 1) {
            format_buffer(buffer);
            if (internal_command(buffer)) continue;

            cmd = parse_cmd(buffer, &is_background);
            if (cmd != NULL) {
                runcommand(cmd, no_timeX, is_background);
            }

        } else if(error == -1) return 0;
    }
    return 0;
}