#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "execute.h"
#include "internalcmd.h"




struct Command *Command() {
    struct Command *cmd = malloc(sizeof(*cmd));
    cmd->type = -1;
    return cmd;
}

struct Command *Exec_Command() {
    struct Exec_Command *cmd = malloc(sizeof(*cmd));
    memset(cmd, 0, sizeof(*cmd));
    cmd->type = 0;
    return (struct Command*)cmd;
}

struct Command *Pipe_Command(struct Command *l, struct Command *r) {
    struct Pipe_Command *cmd = malloc(sizeof(*cmd));
    memset(cmd, 0, sizeof(*cmd));
    cmd->type = 1;
    cmd->left = l;
    cmd->right = r;
    return (struct Command*)cmd;
}




void execute(struct Exec_Command *cmd,int is_background) {

    if (is_background) {
        setpgid(0, 0);
    }
    if(cmd->argv[0] == 0) exit(EXIT_FAILURE);
    execvp(cmd->argv[0], cmd->argv);
    fprintf(stderr, "myshell: '%s': %s\n", cmd->argv[0], strerror(errno));
    exit(EXIT_FAILURE);
}

void wait_warpped(int pid, int is_background, int flag) {
	if (!is_background) {
        if (flag) {
        	waitid(P_PID, pid, NULL, WNOWAIT | WEXITED);
        	print_timeX(pid);
        }
        waitpid(pid, NULL, 0);
    }

}


int safe_fork() {
	int pid = fork();
	if (pid == -1) {
		fprintf(stderr, "can not fork\n");
        return -1;
	}//else if (pid == 0) {
		//signal(SIGCHLD, SIG_IGN);
	//}
	return pid;
}





void runcommand(struct Command *cmd, int flag, int is_background) {

    if (cmd->type == 0) {
        int pid = safe_fork();
        if (pid == 0) {
        	execute((struct Exec_Command *)cmd, is_background);
        }else if (pid > 0) {
        	wait_warpped(pid, is_background, flag);
        }
        return;
    } else if (cmd->type == 1) {

    	struct Pipe_Command *pipe_command = (struct Pipe_Command*)cmd;

    	int pipefd[MAX_PIPE_NUMBER][2];
    	int pid_list[MAX_PIPE_NUMBER] = {0};
    	int pipe_number = 0;

        pipe(pipefd[pipe_number]);


        int pid = safe_fork();
        pid_list[pipe_number] = pid;

        if (pid == 0) {
        	pipe_out(pipefd[pipe_number]);
            execute((struct Exec_Command *)(pipe_command->left), is_background);
        }

        while(pipe_command->right->type != 0) {
            pipe_command = (struct Pipe_Command*)(pipe_command->right);
            ++pipe_number;
            pipe(pipefd[pipe_number]);
            int pid = safe_fork();
            pid_list[pipe_number] = pid;
            if(pid == 0) {
            	pipe_in(pipefd[pipe_number-1]);
                pipe_out(pipefd[pipe_number]);
                execute((struct Exec_Command *)(pipe_command->left),is_background);
            }else if (pid > 0) {
            	close_pipe(pipefd[pipe_number-1]);
            }

        }

        pid = safe_fork();

        if (pid == 0) {
            pipe_in(pipefd[pipe_number]);
            execute((struct Exec_Command *)(pipe_command->right), is_background);
        }else if (pid > 0) {
            close_pipe(pipefd[pipe_number]);
        }

        wait_warpped(pid, is_background, flag);

        for (size_t i = 0; i < pipe_number + 1; i++) {
            wait_warpped(pid_list[i], is_background, flag);
        }
    }
    return;
}
