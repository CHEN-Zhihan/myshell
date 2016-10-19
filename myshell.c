#include <stdio.h>
#include <string.h>
#include "util.h"
#include "parser.h"
#include "execute.h"
#include "sig.h"
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
extern sig_atomic_t timeX_flag;

bool get_command(char * buffer) {
    memset(buffer, 0,BUFFER_SIZE * sizeof(char));
    char * input = fgets(buffer,BUFFER_SIZE,stdin);
    if (input == nullptr) {

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
    SIGUSR1_handler_wrapper();

    char buffer[BUFFER_SIZE];
    while (true) {
        fprintf(stdout, "## myshell $ ");
        if (get_command(buffer)) {
            char * input = strndup(buffer,strlen(buffer) - 1);
            Line * line = parse(input);
            if (line) {
                //sleep(1);
                timeX_flag=0;
                execute(line);
            }
            free(input);
        }


        while(1) {
            siginfo_t tmp;

            int i = waitid(P_ALL, 1, &tmp, WNOWAIT | WNOHANG | WEXITED);
            //fprintf(stderr, "myshell: %s\n", strerror(errno));
            //printf("$$$$%d$$$\n", i);
            if (i == -1) {
                break;
            }else {
                int pid = tmp.si_pid;
                int gid = getpgid(pid);
                if(gid == -1) {
                    errno = 0;
                }
                if (pid == gid) {
                    PIDNode *pnode = buildPIDNode(pid);
                    printf("[%d] %s Done\n",pid, pnode->name);
                    fflush(stdout);
                } else if (timeX_flag) {
                    print_timeX(pid);
                }
                waitpid(pid, NULL, WNOHANG);
            }
        }


    }
}
