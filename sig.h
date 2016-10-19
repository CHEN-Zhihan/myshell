#ifndef SIG_H
#define SIG_H
#include <signal.h>
int sigusr1_flag = 0;
void SIGCHLD_handler(int signum, siginfo_t * info, void *context);
void SIGINT_handler(int signum);
void SIGUSR1_parent_handler(int signum, siginfo_t * info, void *context);
void SIGUSR1_child_handler(int signum);

void SIGCHLD_handler_wrapper();
void SIGUSR1_parent_handler_wrapper();
void SIGUSR1_child_handler_wrapper();
void SIGINT_handler_wrapper();



#endif