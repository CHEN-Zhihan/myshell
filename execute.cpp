#include "execute.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>

#define PIPE_IN(pipe) close(pipe[1]);dup2(pipe[0], STDIN_FILENO);
#define PIPE_OUT(pipe) close(pipe[0]);dup2(pipe[1], STDOUT_FILENO);
#define CLOSE_PIPE(pipe) close(pipe[1]);close(pipe[0]);
#define EXEC(argv)  execvp(argv[0], (char * const *)argv); \
                    fprintf(stderr, "myshell: '%s': %s\n", argv[0], strerror(errno));

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


void execute(const Line * l) {
    if (not preprocess(l)) {
        if (l->noCmd == 1) {
            const Command * c = l->cmds[0];
            pid_t pid = fork();
            if (pid < 0) {
                perror(nullptr);
                return;
            } else if (pid == 0) {
                EXEC(l->cmds[0]->argv);
            } else {
                waitpid(pid, nullptr, 0);
            }
        } else {
            int pipes[MAX_NO_CMD_PER_LINE][2];
            pid_t pids[MAX_NO_CMD_PER_LINE];
            int noPipe = 0;
            pipe(pipes[noPipe]);
            pid_t pid = fork();
            if (pid < 0) {
                perror(nullptr);
                return;
            } else if (pid == 0) {
                PIPE_OUT(pipes[noPipe]);
                EXEC(l->cmds[noPipe]->argv);
            }
            CLOSE_PIPE(pipes[noPipe]);
            pids[noPipe++] = pid;
            for (;noPipe != l->noCmd - 1; ++noPipe) {
                fprintf(stderr, "gg\n");
                pipe(pipes[noPipe]);
                pid = fork();
                if (pid < 0) {
                    perror(nullptr);
                    return;
                } else if (pid == 0) {
                    PIPE_IN(pipes[noPipe - 1]);
                    PIPE_OUT(pipes[noPipe]);
                    EXEC(l->cmds[noPipe]->argv);
                }
                CLOSE_PIPE(pipes[noPipe]);
                pids[noPipe] = pid;
            }
            pipe(pipes[noPipe]);
            pid = fork();
            if (pid < 0) {
                perror(nullptr);
                return;
            } else if (pid == 0) {
                PIPE_IN(pipes[noPipe - 1]);
                EXEC(l->cmds[noPipe]->argv);
            }
            CLOSE_PIPE(pipes[noPipe]);
            pids[noPipe] = pid;
            for (int i = 0; i != l->noCmd; ++i) {
                waitpid(pids[i], nullptr, 0);
            }
        }
    }
}
