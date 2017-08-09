#ifndef PARSER_H
#define PARSER_H

constexpr int MAX_ARGS_NUMBER = 30;
constexpr int MAX_ARG_LENGTH = 30;
constexpr int BUFFER_SIZE = 1000;
constexpr int MAX_NO_CMD_PER_LINE = 30;


struct Command {
    int argc;
    const char * * argv;
};



struct Line {
    bool background;
    int noCmd;
    const Command * cmds[MAX_NO_CMD_PER_LINE];
};

const Line * parse(char *);
void freeLine(const Line * );
#endif
