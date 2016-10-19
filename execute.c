#include "execute.h"
#include <unistd.h>
#include <errno.h>
#include <wait.h>
#include "util.h"
#include "sig.h"
#include "viewtree.h"
//#define BUFFER_SIZE 3
extern sig_atomic_t sigusr1_flag;
extern sig_atomic_t timeX_flag;

void run_command(Command *cmd, int is_background) {
    if (is_background) {
        setpgid(0, 0);
    }
    while(sigusr1_flag == 0);
    execvp(cmd->argv[0], cmd->argv);
    fprintf(stderr, "myshell: '%s': %s\n", cmd->argv[0], strerror(errno));
    exit(EXIT_FAILURE);
}

void wait_wrapped(int pid, int is_background, int flag) {
    if (!is_background) {
        struct sigaction act;
        sigaction(SIGINT,nullptr,&act);
        signal(SIGINT,SIG_IGN);
        waitid(P_PID, pid, NULL, WNOWAIT | WEXITED);
        sigaction(SIGINT,&act,nullptr);
    }
}
int safe_fork() {
    sigusr1_flag = 0;
    int pid = fork();
    if (pid == -1) {
        fprintf(stderr, "can not fork\n");
        return -1;
    }
    if (pid>0) {
        usleep(200);
        kill(pid, SIGUSR1);
    }
    return pid;
}


void execute(struct Line *line) {

    if (line->type ==EXIT_TYPE) {
        fprintf(stderr, "myshell: Terminated\n");
        exit(EXIT_SUCCESS);
    } else if (line->type==VIEWTREE_TYPE) {
        viewTree();
    } else {

        if (line->type==TIMEX_TYPE) {
            timeX_flag=1;
        }
        if (line->head->next == NULL) {
            int pid = safe_fork();
            if (pid == 0) {
                run_command(line->head, line->background);
            }else if (pid > 0) {
                //signal(SIGINT,SIG_IGN);
                wait_wrapped(pid, line->background, line->type);
                //sigaction(SIGINT,&act,nullptr);
            }
        } else {

            Command *iterator = line->head;

            int pipefd[MAX_PIPE_NUMBER][2];
            pid_t pid_list[MAX_PIPE_NUMBER] = {0};
            int pipe_number = 0;

            pipe(pipefd[pipe_number]);

            pid_t pid = safe_fork();
            pid_list[pipe_number] = pid;
            if (pid == 0) {
                pipe_out(pipefd[pipe_number]);
                run_command(iterator, line->background);
            } //else {
                //signal(SIGINT,SIG_IGN);
            //}
            while(iterator->next->next != NULL) {
                iterator = iterator -> next;
                ++pipe_number;
                pipe(pipefd[pipe_number]);
                pid_t pid = safe_fork();
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
           
            //sigaction(SIGINT,&act,nullptr);
            for (size_t i = 0; i < pipe_number + 1; i++) {
                wait_wrapped(pid_list[i], line->background, line->type);
            }    
        }
        timeX_flag=0;
        
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
    double time_from_boot;
    int efb;

    fscanf(file, "%d %s %c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu %d %d %d %d %d %d %d", &pid_get, cmd, &stat, &z, &z, &z, &z, &z,
           (unsigned *)&z, &h, &h, &h, &h, &ut, &st, &z, &z, &z, &z, &z, &z, &efb);
    fclose(file);

    cmd[strlen(cmd) - 1] = 0;


    double utime = ut*1.0f/sysconf(_SC_CLK_TCK);
    double stime = st*1.0f/sysconf(_SC_CLK_TCK);
    double exec_from_boot = efb*1.0f/sysconf(_SC_CLK_TCK);

    FILE *uptime = fopen("/proc/uptime", "r");
    if (uptime == NULL) {
        printf("Error in open uptime file\n");
        exit(0);
    }
    fscanf(uptime, "%lf", &time_from_boot);
    fclose(uptime);
    double runtime=time_from_boot-exec_from_boot;
    runtime=runtime<0?0:runtime;

    printf("\n");
    printf("%-10s%-15s%-10s%-10s%-10s\n", "PID", "CMD", "RTIME", "UTIME", "STIME");
    printf("%-10d%-15s%-4.2lf%-6s%-4.2lf%-6s%-4.2lf%-6s\n", pid_get, cmd+1,runtime, " s", utime," s", stime," s");

}
