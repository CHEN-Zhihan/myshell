#include "execute.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>


#include <unistd.h>
#include <wait.h>
#include <errno.h>

void run_command(Command *cmd, int is_background) {

    if (is_background) {
        setpgid(0, 0);
    }
    execvp(cmd->argv[0], cmd->argv);
    fprintf(stderr, "myshell: '%s': %s\n", cmd->argv[0], strerror(errno));
    exit(EXIT_FAILURE);
}

void wait_wrapped(int pid, int is_background, int flag) {
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
	}else if (pid == 0) {
		signal(SIGCHLD, SIG_IGN);
	}
	return pid;
}


void execute(struct Line *line) {

	if (line->type < 0) {
		built_in(line->type);
	} else {

		if (line->head->next == NULL) {
			 int pid = safe_fork();
        	if (pid == 0) {
        		run_command(line->head, line->background);
        	}else if (pid > 0) {
        		wait_wrapped(pid, line->background, line->type);
        	}
		} else {

			Command *iterator = line->head;

			int pipefd[MAX_PIPE_NUMBER][2];
	    	int pid_list[MAX_PIPE_NUMBER] = {0};
	    	int pipe_number = 0;

	        pipe(pipefd[pipe_number]);

	        int pid = safe_fork();
	        pid_list[pipe_number] = pid;

	        if (pid == 0) {
	        	pipe_out(pipefd[pipe_number]);
	            run_command(iterator, line->background);
	        }

	        while(iterator->next->next != NULL) {
	            
	            iterator = iterator -> next;
	            ++pipe_number;
	            pipe(pipefd[pipe_number]);
	            int pid = safe_fork();
	            pid_list[pipe_number] = pid;
	            if(pid == 0) {
	            	pipe_in(pipefd[pipe_number-1]);
	                pipe_out(pipefd[pipe_number]);
	                run_command(iterator, line->background);
	            }else if (pid > 0) {
	            	close_pipe(pipefd[pipe_number-1]);
	            }
	        }

	        iterator = iterator->next;

	        pid = safe_fork();

	        if (pid == 0) {
	            pipe_in(pipefd[pipe_number]);
	            run_command(iterator, line->background);
	        }else if (pid > 0) {
	            close_pipe(pipefd[pipe_number]);
	        }

	        wait_wrapped(pid, line->background, line->type);

	        for (size_t i = 0; i < pipe_number + 1; i++) {
	            wait_wrapped(pid, line->background, line->type);
	        }
		}
	}
}


void print_timeX(int pid) {
    int pid_get;
    char cmd[MAX_PROC_FILE_PATH];
    unsigned long ut, st;


    char str[MAX_PROC_FILE_PATH];
    sprintf(str, "/proc/%d/stat", pid);
    FILE *file = fopen(str, "r");
    if (file == NULL) {
        printf("Error in open my proc file\n");
        exit(0);
    }

    int z;
    unsigned long h;
    char stat;

    fscanf(file, "%d %s %c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu", &pid_get, cmd, &stat, &z, &z, &z, &z, &z,
           (unsigned *)&z, &h, &h, &h, &h, &ut, &st);
    fclose(file);

    cmd[strlen(cmd) - 1] = 0;


    double utime = ut*1.0f/sysconf(_SC_CLK_TCK);
    double stime = st*1.0f/sysconf(_SC_CLK_TCK);

    printf("\n");
    printf("%-10s%-15s%-10s%-10s%-10s\n", "PID", "CMD", "RTIME", "UTIME", "STIME");
    printf("%-10d%-15s%-4.2lf%-6s%-4.2lf%-6s%-4.2lf%-6s\n", pid_get, cmd+1, utime + stime, " s", utime," s", stime," s");
}


void built_in(int type) {
	if (type == -1) {
		fprintf(stderr, "myshell: Terminated\n");
        exit(EXIT_SUCCESS);
	} else if (type == -2) {
		return;
		

	}


}