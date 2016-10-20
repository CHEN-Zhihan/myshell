#include "sig.h"
#include "execute.h"
#include "util.h"
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <wait.h>


volatile sig_atomic_t sigusr1_flag =0;
volatile sig_atomic_t timeX_flag=0;



/*
    
*/
void SIGCHLD_handler(int signum, siginfo_t * info, void *context) {
    int pid = info->si_pid;
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
    waitpid(pid, NULL, 0);
}

void SIGINT_handler(int signum) {
    printf("\n");
}


void SIGUSR1_handler(int signum) {
    sigusr1_flag = 1;
}

void SIGCHLD_handler_wrapper() {
    struct sigaction act;
    sigaction(SIGCHLD, NULL, &act);
    act.sa_sigaction = SIGCHLD_handler;
    act.sa_flags |= SA_NOCLDSTOP;
    act.sa_flags |= SA_SIGINFO;
    act.sa_flags |= SA_RESTART;
    sigaction(SIGCHLD, &act, NULL);
}
void SIGUSR1_handler_wrapper() {
    struct sigaction act;
    sigaction(SIGUSR1, NULL, &act);
    act.sa_handler = SIGUSR1_handler;
    act.sa_flags |= SA_SIGINFO;
    act.sa_flags |= SA_RESTART;
    act.sa_flags |= SA_NODEFER;
    sigaction(SIGUSR1, &act, NULL);
}

void SIGINT_handler_wrapper() {
    struct sigaction act;
    sigaction(SIGCHLD, NULL, &act);
    act.sa_handler = SIGINT_handler;
    act.sa_flags &=~SA_RESTART;
    sigaction(SIGINT, &act, NULL);
}

void cleanup_wrapper() {
    for(int i = 0 ; i < 256; ++i){
        siginfo_t tmp;
        int i = waitid(P_ALL, getpid(), &tmp, WNOWAIT | WNOHANG | WEXITED);
        if (i == -1) {
            break;
        } else {
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