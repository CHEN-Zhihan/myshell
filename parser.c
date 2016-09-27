#include "parser.h"
#define BACKGROUND_MODE 1
#define FRONTGROUND_MODE 0
int hasCmd(const char * line, const int begin, const int end) {
    int i=begin;
    bool hascmd=false;
    while (i<end &&line[i] != '|') {
        if (line[i]!=' '&&line[i]!='&') {
            hascmd=true;
        }
        ++i;
    }
    if (hascmd) {
        return i;
    } else {
        return -1;
    }
}

void freeCommand(Command * cmd) {
    int i=0;
    for (;i!=cmd->argc;++i) {
        free(cmd->argv[i]);
    }
    free(cmd);
}

void freeLine(Line * line) {
    Command * iterator = line->head;
    while (iterator) {
        Command * temp = iterator;
        iterator=iterator->next;
        freeCommand(temp);
    }
    free(line);
}

int syntaxCheck(char * line) {
    ssize_t size = strlen(line);
    if(split_input(line, nullptr," ",false)==0) {
        return -1;
    }
    char * background = strchr(line,'&');

    // & check
    if (background) {
        if (background==line) {
            printf("myshell: syntax error near unexpected token '&'\n");
            return -1;
        }
        int before=(int)(background-line)-1;
        bool emptyBefore=true;
        while (before>=0) {
            if (line[before]!=' ') {
                emptyBefore=false;
                break;
            }
            --before;
        }
        if (emptyBefore) {
            printf("myshell: syntax error near unexpected token '&'\n");
            return -1;
        }
        int after = (int)(background-line)+1;
        while (after<size) {
            if (line[after]!=' ') {
                printf("myshell: syntax error near unexpected token '&'\n");
                return -1;
            }
            ++after;
        }
    }

    // | check
    char * pipe = strchr(line, '|');
    if (pipe!=nullptr) {
        int itr = hasCmd(line,0,(int)(pipe-line));
        if (itr==-1||line[size-1]=='|') {
            printf("myshell: syntax error near unexpeced token '|'\n");
            return -1;
        }
        while (itr!=size)  {
            itr = hasCmd(line,itr+1,(int)size);
            if (itr==-1) {
                printf("myshell: syntax error near unexpeced token '|'\n");
                return -1;
            }
        }
    }
    if (background!=nullptr) {
        return BACKGROUND_MODE;
    } else {
        return FRONTGROUND_MODE;
    }
}

Line * process(Line * line, char * message) {
    if (line->head->argc!=1||line->head->next!=nullptr||line->background) {
        printf("myshell: \"%s\" with other arguments!!!\n",message);
        freeLine(line);
        return nullptr;
    } else {
        return line;
    }
}


Line * processBuiltin(Line * line) {
    Command * first = line->head;
    if (strcmp(first->argv[0],"exit\0")==0) {
        line->type=EXIT_TYPE;
        return process(line,"exit\0");
    } else if (strcmp(first->argv[0],"viewtree\0")==0) {
        line->type=VIEWTREE_TYPE;
        return process(line,"viewtree\0");
    }
    Command * iterator=line->head;
    if (strcmp(iterator->argv[0],"timeX\0")==0) {
        if (line->background) {
            printf("myshell: \"timeX\" cannot be run in background mode\n");
            freeLine(line);
            return nullptr;
        }
        if (iterator->argc==1) {
            printf("myshell: \"timeX\" cannot be a standalone command\n");
            freeLine(line);
            return nullptr;
        }
        --iterator->argc;
        int i=0;
        for (i=0;i!=iterator->argc;++i) {
            iterator->argv[i]=iterator->argv[i+1];
        }
        iterator->argv[i]=nullptr;
        line->type=TIMEX_TYPE;
    } else {
        line->type=NORMAL_TYPE;
    }
    return line;
}

Command * parseCommand(char * input) {
    Command * result = (Command *)malloc(sizeof(Command));
    result->argc=split_input(input,result->argv," ",true);
    if (strcmp(result->argv[result->argc-1],"&")==0) {
        free(result->argv[--result->argc]);
    }
    char * last = result->argv[result->argc-1];
    size_t size = strlen(last);
    if (last[size-1]=='&') {
        last[size-1]='\0';
    }
    result->next=nullptr;
    return result;
}

Line * parse(char * line) {
    int i=0;
    int syntaxResult = syntaxCheck(line);
    if (syntaxResult==-1) {
        return nullptr;
    }
    Line * result = (Line*)malloc(sizeof(Line));
    result->type=0;
    result->background=syntaxResult;
    char * rawCmd[5];
    int cmdNumber = split_input(line,rawCmd,"|",true);
    Command *iterator = nullptr;
    while (i<cmdNumber) {
        if (iterator==nullptr) {
            result->head=parseCommand(rawCmd[i]);
            free(rawCmd[i]);
            iterator=result->head;
        } else {
            iterator->next=parseCommand(rawCmd[i]);
            free(rawCmd[i]);
            iterator=iterator->next;
        }
        ++i;
    }
    result = processBuiltin(result);
    return result;
}


