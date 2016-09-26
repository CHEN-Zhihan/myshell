#include "execute.h"
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

void print_timeX(int pid) {
    printf("as if I am TIMEX\n");
}

void print_tree() {
    printf("as if I am a tree\n");
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

void built_in(int type) {
    if (type==EXIT_TYPE) {
        printf("myshell: Terminated\n");
        exit(0);
    } else if (type==VIEWTREE_TYPE) {
        print_tree();
    }
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