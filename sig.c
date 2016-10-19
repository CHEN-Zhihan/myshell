#include "sig.h"

int sigusr1_flag = 0;


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
        waitpid(pid, NULL, 0);
    }
}

void SIGINT_handler(int signum) {
    printf("\n");
}

void SIGUSR1_parent_handler(int signum, siginfo_t * info, void *context) {
    kill(info->si_pid, SIGUSR1);
}

void SIGUSR1_child_handler(int signum) {
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

void SIGUSR1_parent_handler_wrapper() {
    struct sigaction act;
    sigaction(SIGUSR1, NULL, &act);
    act.sa_sigaction = SIGUSR1_parent_handler;
    act.sa_flags |= SA_SIGINFO;
    act.sa_flags |= SA_RESTART;
    sigaction(SIGUSR1, &act, NULL);
}

void SIGUSR1_child_handler_wrapper() {
    struct sigaction act;
    sigaction(SIGUSR1, NULL, &act);
    act.sa_handler = SIGUSR1_child_handler;
    act.sa_flags |= SA_SIGINFO;
    act.sa_flags |= SA_RESTART;
    sigaction(SIGUSR1, &act, NULL);
}


void SIGINT_handler_wrapper() {
    struct sigaction act;
    sigaction(SIGCHLD, NULL, &act);
    act.sa_handler = SIGINT_handler;
    act.sa_flags &=~SA_RESTART;
    sigaction(SIGINT, &act, NULL);
}