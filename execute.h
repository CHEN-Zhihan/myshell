#include "util.h"
void run_command(Command *cmd, int is_background);
void wait_warpped(int pid, int is_background, int flag);
int safe_fork();
void execute(struct Line *line, int flag, int is_background);
