#include "util.h"
void run_command(Command *cmd, int is_background);
void wait_wrapped(int pid, int is_background, int flag);
int safe_fork();
void execute(struct Line *line);
