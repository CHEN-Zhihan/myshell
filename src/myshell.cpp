#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <pwd.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "execute.h"
#include "parser.h"

#define COLOR_GREEN "\x1B[32m"

void SIGINTHandler(int) {
    fprintf(stdout, "\n");
}

int main(int argc, const char * argv[]) {
    using_history();
    struct sigaction act;
    sigaction(SIGINT, nullptr, &act);
    act.sa_handler = SIGINTHandler;
    act.sa_flags &= ~SA_RESTART;
    sigaction(SIGINT, &act, nullptr);
    char cwd[BUFFER_SIZE];
    struct passwd * pwd = getpwuid(getuid());
    char * user = pwd->pw_name;
    char prompt[BUFFER_SIZE];
    while (true) {
        if (getcwd(cwd, BUFFER_SIZE) != nullptr) {
            sprintf(prompt, "%s@%s: ", user, cwd);
        } else {
            perror(nullptr);
            exit(EXIT_FAILURE);
        }
        char * l = readline(prompt);
        add_history(l);
        const Line * line = parse(l);
        if (line != nullptr) {
            execute(line);
            freeLine(line);
        }
        free(l);
    }
    return 0;
}
