#ifndef SIG_H
#define SIG_H
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include "util.h"
#include <errno.h>
#include <wait.h>

void SIGCHLD_handler(int signum, siginfo_t * info, void *context);
void SIGINT_handler(int signum);
void SIGUSR1_parent_handler(int signum, siginfo_t * info, void *context);
void SIGUSR1_child_handler(int signum);

void SIGCHLD_handler_wrapper();
void SIGUSR1_parent_handler_wrapper();
void SIGUSR1_child_handler_wrapper();
void SIGINT_handler_wrapper();

void SIGUSR1_handler_wrapper();



#endif