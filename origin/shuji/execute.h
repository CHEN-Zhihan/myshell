#ifndef EXECUTE_H_
#define EXECUTE_H_

#define pipe_out(pipefd) close(pipefd[0]);dup2(pipefd[1],STDOUT_FILENO);close(pipefd[1]);
#define pipe_in(pipefd) close(pipefd[1]);dup2(pipefd[0], STDIN_FILENO);close(pipefd[0]);
#define close_pipe(pipefd) close(pipefd[0]);close(pipefd[1]);

#define MAX_PIPE_NUMBER 5
#define MAX_ARG_NUMBER 100 //


struct Command {
    int type;
};

struct Exec_Command {
    int type;
    char *argv[MAX_ARG_NUMBER];
};

struct Pipe_Command {
    int type;
    struct Command *left;
    struct Command *right;
};

struct Command *Command();

struct Command *Exec_Command();

struct Command *Pipe_Command(struct Command *l, struct Command *r);


void runcommand(struct Command *cmd, int flag, int is_background);
void execute(struct Exec_Command *cmd,int is_background);
void wait_warpped(int pid, int is_background, int flag);
int safe_fork();


#endif
