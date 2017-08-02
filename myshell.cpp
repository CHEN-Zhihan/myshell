#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <pwd.h>
#include <sys/types.h>
#include <signal.h>
#include "parser.h"
#include "execute.h"



int main(int argc, const char * argv[]) {
    struct sigaction act;
    sigaction(SIGINT, nullptr, &act);
    char cwd[BUFFER_SIZE];
    struct passwd * pwd = getpwuid(getuid());
    char * user = pwd->pw_name;
    while (true) {
        if (getcwd(cwd, BUFFER_SIZE) != nullptr) {
            fprintf(stdout, "%s@%s: ", user, cwd);
        } else {
            fprintf(stderr, "error getcwd()");
            perror(nullptr);
            exit(EXIT_FAILURE);
        }
        const Line * line = parse();
        if (line != nullptr) {
            execute(line);
            freeLine(line);
        } 
    }
    return 0;
}
