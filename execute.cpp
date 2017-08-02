#include "execute.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>

bool preprocess(const Line * l) {
    const Command * firstCmd = l->cmds[0];
    if (strcmp(firstCmd->argv[0], "exit\0") == 0) {
        fprintf(stdout, "bye\n");
        exit(EXIT_SUCCESS);
    } else if (strcmp(firstCmd->argv[0], "cd\0") == 0) {
        if (firstCmd->argc != 2) {
            fprintf(stderr, "myshell: invalid cd operation\n");
            return true;
        }
        if (chdir(firstCmd->argv[1]) != 0) {
            perror(nullptr);
        }
        return true;
    }
    return false;
}

void executeCmd(const Command * c, bool) {

}

void execute(const Line * l) {
    if (not preprocess(l)) {
        if (l->noCmd == 1) {
            const Command * c = l->cmds[0];
            pid_t pid = fork();
            if (pid < 0) {
                perror(nullptr);
                return;
            } else if (pid == 0) {
                execvp(c->argv[0], (char * const *)c->argv);
                fprintf(stderr, "myshell: '%s': %s\n", c->argv[0], strerror(errno));
            } else {
                waitpid(pid, nullptr, 0);
            }
        } else {

        }
    }
}
