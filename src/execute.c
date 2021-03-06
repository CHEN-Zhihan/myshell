#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "execute.h"

#define PIPE_IN(pipe) \
    close(pipe[1]);   \
    dup2(pipe[0], STDIN_FILENO);
#define PIPE_OUT(pipe) \
    close(pipe[0]);    \
    dup2(pipe[1], STDOUT_FILENO);
#define CLOSE_PIPE(pipe) \
    close(pipe[1]);      \
    close(pipe[0]);
#define EXEC(argv)                                                    \
    execvp(argv[0], (char* const*)argv);                              \
    fprintf(stderr, "myshell: '%s': %s\n", argv[0], strerror(errno)); \
    exit(EXIT_FAILURE);
#define SAFE_PIPE(argv)                                            \
    if (pipe(argv)) {                                              \
        fprintf(stderr, "myshell: 'pipe': %s\n", strerror(errno)); \
        exit(EXIT_FAILURE);                                        \
    }

bool preprocess(const Line* l) {
    const Command* firstCmd = l->cmds[0];
    if (strcmp(firstCmd->argv[0], "exit") == 0) {
        fprintf(stdout, "bye\n");
        exit(EXIT_SUCCESS);
    } else if (strcmp(firstCmd->argv[0], "cd") == 0) {
        if (firstCmd->argc == 1) {
            firstCmd->argv[1] = getenv("HOME");
        }
        if (firstCmd->argc > 2) {
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

void processRedirect(const Line* l) {
    for (int i = 0; i != 2; ++i) {
        if (l->redirect[i] != NO_REDIRECT) {
            int flag = O_WRONLY | O_CREAT;
            flag |= l->redirect[i] == REDIRECT_APPEND ? O_APPEND : O_TRUNC;
            int fd = open(l->redirectFile[i], flag, 0664);
            if (fd < 0) {
                fprintf(stderr, "myshell: '%s': %s\n", l->redirectFile[i],
                        strerror(errno));
                return;
            }
            dup2(fd, i == 0 ? STDOUT_FILENO : STDERR_FILENO);
            close(fd);
        }
    }
}

void execute(const Line* l) {
    if (!preprocess(l)) {
        if (l->noCmd == 1) {
            pid_t pid = fork();
            if (pid < 0) {
                perror(nullptr);
                return;
            } else if (pid == 0) {
                processRedirect(l);
                EXEC(l->cmds[0]->argv);
            } else {
                waitpid(pid, nullptr, 0);
            }
        } else {
            int pipes[MAX_NO_CMD_PER_LINE][2];
            pid_t pids[MAX_NO_CMD_PER_LINE];
            int noPipe = 0;
            SAFE_PIPE(pipes[noPipe]);
            pid_t pid = fork();
            if (pid < 0) {
                perror(nullptr);
                return;
            } else if (pid == 0) {
                PIPE_OUT(pipes[noPipe]);
                EXEC(l->cmds[noPipe]->argv);
            }
            pids[noPipe++] = pid;
            for (; noPipe != l->noCmd - 1; ++noPipe) {
                SAFE_PIPE(pipes[noPipe]);
                pid = fork();
                if (pid < 0) {
                    perror(nullptr);
                    return;
                } else if (pid == 0) {
                    PIPE_IN(pipes[noPipe - 1]);
                    PIPE_OUT(pipes[noPipe]);
                    EXEC(l->cmds[noPipe]->argv);
                }
                CLOSE_PIPE(pipes[noPipe - 1]);
                pids[noPipe] = pid;
            }
            SAFE_PIPE(pipes[noPipe]);
            pid = fork();
            if (pid < 0) {
                perror(nullptr);
                return;
            } else if (pid == 0) {
                PIPE_IN(pipes[noPipe - 1]);
                processRedirect(l);
                EXEC(l->cmds[noPipe]->argv);
            }
            CLOSE_PIPE(pipes[noPipe - 1]);
            CLOSE_PIPE(pipes[noPipe]);
            pids[noPipe] = pid;
            for (int i = 0; i != l->noCmd; ++i) {
                waitpid(pids[i], nullptr, 0);
            }
        }
    }
}
