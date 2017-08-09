#include "parser.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <signal.h>


char * buffer;

int next;
int length;

inline const Command * initCommand(int argc, const char * argv[]) {
    Command * c = (Command *)malloc(sizeof(Command));
    c->argc = argc;
    c->argv = (const char * *)malloc(sizeof(char *) * MAX_ARGS_NUMBER);
    for (int i = 0; i != argc; ++i) {
        c->argv[i] = argv[i];
    }
    c->argv[c->argc] = nullptr;
    return c;
}

inline Line * initLine(const Command * c) {
    Line * l = (Line *)malloc(sizeof(Line));
    l->cmds[0] = c;
    l->noCmd = 1;
    return l;
}


inline void escapeSpace() {
    while (next != length && buffer[next] == ' ') {
        ++next;
    }
}

inline void freeCmd(const Command * c) {
    for (int i = 0; i != c->argc; ++i) {
        free((char*)c->argv[i]);
    }
    free(c->argv);
    free((Command*)c);
}

inline void freeLine(const Line * l) {
    for (int i = 0; i != l->noCmd; ++i) {
        freeCmd(l->cmds[i]);
    }
    free((Line *)l);
}

inline bool isArg(char c) {
    return (c != '&') and (c != '|') and (c != '\n') and (c != ' ');
}

inline bool parsePipe() {
    return next != length and buffer[next++] == '|';
}

inline bool parseBackground() {
    return next != length and buffer[next++] == '&';
}

const char * parseArg() {
    int begin = next;
    while (next != length && isArg(buffer[next])) {
        ++next;
    }
    if (next == begin) {
        return nullptr;
    }
    char * arg = (char*)malloc(sizeof(char)* MAX_ARG_LENGTH);
    strncpy(arg, buffer + begin, next - begin);
    arg[next - begin] = '\0';
    return arg;
}

const Command * parseCmd() {
    escapeSpace();
    const char * argv[MAX_ARGS_NUMBER];
    int argc = 0;
    const char * arg = nullptr;
    while ((arg = parseArg()) != nullptr) {
        argv[argc++] = arg;
        escapeSpace();
    }
    if (argc == 0) {
        return nullptr;
    }
    return initCommand(argc, argv);
}

bool parseOpCmd(Line * l) {
    escapeSpace();
    if (!parsePipe()) {
        --next;
        return true;
    }
    const Command * c = parseCmd();
    if (c == nullptr) {
        return false;
    }
    l->cmds[l->noCmd++] = c;
    return parseOpCmd(l);
}


const Line * parseLine() {
    next = 0;
    length = strlen(buffer);
    const Command * c = parseCmd();
    if (c == nullptr) {
        return nullptr;
    }
    Line * l = initLine(c);
    if (parseOpCmd(l) == false) {
        freeLine(l);
        return nullptr;
    }
    l->background = parseBackground();
    if (not l->background) {
        --next;
    }
    escapeSpace();
    if (next != length - 1) {
        fprintf(stderr, "myshell: syntax error\n");
        freeLine(l);
        return nullptr;
    }
    return l;
}


const Line * parse(char * input) {
    buffer = input;
    if (input == nullptr) {
        if (errno != EINTR) {
            perror(nullptr);
        }
        return nullptr;
    }
    return parseLine();
}
