#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include "parser.h"


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
    l->redirectFile[0] = nullptr;
    l->redirectFile[1] = nullptr;
    l->redirect[0] = NO_REDIRECT;
    l->redirect[1] = NO_REDIRECT;
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
    for (int i = 0; i != 2; ++i) {
        if (l->redirectFile[i] != nullptr) {
            free((char*)l->redirectFile[i]);
        }
    }
    free((Line *)l);
}

inline bool isArg() {
    char c = buffer[next];
    if (c == '1' || c == '2') {
        return next + 1 == length || buffer[next + 1] != '>';
    }
    return (c != '&') && (c != '|') && (c != '\n') && (c != ' ') && (c != '>');
}

inline bool parsePipe() {
    return next != length && buffer[next] == '|' && (++next);
}

inline bool parseRedirect() {
    return next != length && buffer[next] == '>' && (++next);
}

inline bool parseBackground() {
    return next != length && buffer[next] == '&' && (++next);
}

const char * parseArg() {
    int begin = next;
    while (next != length && isArg()) {
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

inline int parseOpNumber() {
    if (next != length) {

        if (buffer[next] == '1') {
            ++next;
            return 0;
        }
        if (buffer[next] == '2') {
            ++next;
            return 1;
        }
    }
    return 0;
}

inline Line* parseOneRedirect(Line* l, int redirectNumber) {
    if (parseRedirect()) {
        l->redirect[redirectNumber] = parseRedirect() ? REDIRECT_APPEND : REDIRECT;
        escapeSpace();
        l->redirectFile[redirectNumber] = parseArg();
        if (l->redirectFile[redirectNumber] == nullptr) {
            freeLine(l);
            return nullptr;
        }
    }
    return l;
}

Line * parseRedirectToFile(Line* l) {
    int firstRedirect = parseOpNumber();
    l = parseOneRedirect(l, firstRedirect);
    if (l == nullptr) {
        return nullptr;
    }
    escapeSpace();
    int secondRedirect = parseOpNumber();
    if (secondRedirect == firstRedirect && parseRedirect()) {
        fprintf(stderr, "myshell: incorrect redirection *%c* %d\t%d\n", buffer[next], firstRedirect, secondRedirect);
        freeLine(l);
        return nullptr;
    }
    l = parseOneRedirect(l, secondRedirect);
    if (l == nullptr) {
        return nullptr;
    }

    return l;
}

bool parseOpCmd(Line * l) {
    escapeSpace();
    if (!parsePipe()) {
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
    if (!parseOpCmd(l)) {
        freeLine(l);
        #ifdef DEBUG
        #endif
        fprintf(stderr, "myshell: syntax error\n");
        return nullptr;
    }
    escapeSpace();
    l = parseRedirectToFile(l);
    if (l == nullptr) {
        return nullptr;
    }
    escapeSpace();
    l->background = parseBackground();
    escapeSpace();
    if (next != length) {
        #ifdef DEBUG
        fprintf(stderr, "[DEBUG] has more input, %d parsed, %d expected\n", next, length);
        exit(EXIT_FAILURE);
        #endif
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
