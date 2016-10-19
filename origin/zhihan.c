#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <wait.h>

#define nullptr NULL
#ifndef BUFFER_SIZE
#define BUFFER_SIZE 1024
#endif

#define DEBUG 0
typedef struct Command {
    int argc;
    char ** argv;
    bool valid;
    bool time;
} Command;



typedef struct Line {
    Command cmd[5];
    int cmdNumber;
    bool background;
    bool valid;
} Line;

typedef struct strList {
    char * content;
    struct strList * next;
} strList;

Line LineConstructor(Command * cmds, int cmdNumber, bool background,bool valid) {
    Line result;
    int i=0;
    for (i=0;i<cmdNumber;++i) {
        result.cmd[i]=cmds[i];
    }
    result.cmdNumber=cmdNumber;
    result.background=background;
    result.valid=valid;
    return result;
}


Command parseCommand(char * line) {
    char * word = strtok(line," ");
    Command result;
    result.argc=0;
    strList * head=nullptr;
    strList * iterator = head;
    while (word) {
        ++result.argc;
        if (head!=nullptr) {
            iterator->next=(strList*)malloc(sizeof(strList));
            iterator->next->content=strdup(word);
            iterator->next->next=nullptr;
            iterator=iterator->next;
        } else {
            head=(strList*)malloc(sizeof(strList));
            head->content=strdup(word);
            head->next=nullptr;
            iterator=head;
        }
        word=strtok(nullptr," ");
    }
    if (strcmp(iterator->content,"&\0")==0) {
        --result.argc;
    }
    iterator=head;
    result.argv=(char **)malloc(sizeof(char*)*result.argc);
    int i=0;
    for (i=0;i!=result.argc;++i) {
        result.argv[i] = iterator->content;
        iterator=iterator->next;
    }
    char * lastOne = result.argv[result.argc-1];
    if (lastOne[strlen(lastOne)-1]=='&') {
        lastOne[strlen(lastOne)-1]='\0';
    }
    result.valid=true;
    return result;
}

Line invalidLine() {
    Line result;
    result.valid=false;
    result.cmdNumber=0;
    return result;
}

void freeLine(Line * line) {
    int i=0;
    int j=0;
    for (i=0;i!=line->cmdNumber;++i) {
        for (j=0;j!=line->cmd[i].argc;++j) {
            free(line->cmd[i].argv[j]);
            line->cmd[i].argv[j]=nullptr;
        }
        free(line->cmd[i].argv);
        line->cmd[i].argv=nullptr;
    }
}


Line cmdCheckedLine(Line * line) {
    Command firstCommand = line->cmd[0];
    if (strcmp(firstCommand.argv[0],"exit\0")==0) {
        if (firstCommand.argc!=1 || line->cmdNumber!=1 || line->background) {
            printf("myshell: \"exit\" with other arguments!!!\n");
            freeLine(line);
            return invalidLine(); // invalid
        } else {
            printf("myshell: Terminated\n");
            freeLine(line);
            return LineConstructor(nullptr,-1,false,true); // exit
        }
    }
    int i=0;
    for (i=0;i!=line->cmdNumber;++i) {
        if (strcmp(line->cmd[i].argv[0],"timeX\0")==0) {
            if (line->background) {
                printf("myshell: \"timeX\" cannot be run in background mode\n");
                freeLine(line);
                return invalidLine();
            }
            if (line->cmd[i].argc==1) {
                printf("myshell: \"timeX\" cannot be a standalone command\n");
                freeLine(line);
                return invalidLine();
            } else  {
                line->cmd[i].time=true;
                --line->cmd[i].argc;
                char ** argv=(char **)malloc(sizeof(char*)*line->cmd[i].argc);
                int j=0;
                free(line->cmd[i].argv[0]);
                for (j=0;j!=line->cmd[i].argc;++j) {
                    argv[j]=strdup(line->cmd[i].argv[j+1]);
                    free(line->cmd[i].argv[j+1]);
                }
                free(line->cmd[i].argv);
                line->cmd[i].argv=argv;
            }
        }
    }
    return *line;
}

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

int syntaxCheck(char * line,const size_t size) {
    // & check
    char * temp = strdup(line);
    char * word = strtok(temp," ");
    if (word==nullptr) {
        return -1;
    }
    free(temp);
    char * background = strchr(line,'&');
    if (background!=nullptr) {
        if (background==line) {
            printf("myshell: syntax error near unexpected token '&'\n");
            return -1;
        }
        int before = (int)(background-line)-1;
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
    char * pipe = strchr(line,'|');
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
        if (pipe!=nullptr) {
            return 3;
        } else {
            return 1;
        }
    } else {
        if (pipe!=nullptr) {
            return 2;
        } else {
            return 0;
        }
    }
}

Line parse(char * line) {
    int i=0;
    size_t lineSize=strlen(line);
    int syntaxResult = syntaxCheck(line,lineSize);
    if (syntaxResult==-1) {
        return invalidLine();
    }
    Line result;
    result.valid=true;
    result.cmdNumber=0;
    result.background=syntaxResult%2==1;
    if (syntaxResult<2) { // NO PIPE
        ++result.cmdNumber;
        result.cmd[0]=parseCommand(line);
        result.valid=result.cmd[0].valid;
        return cmdCheckedLine(&result);
    }
    char *rawCmd[5];
    char * cmd = strtok(line,"|");
    while (cmd) {
        rawCmd[result.cmdNumber++]=strdup(cmd);
        cmd=strtok(nullptr,"|");
    }
    for(i=0;i!=result.cmdNumber;++i) {
        result.cmd[i]=parseCommand(rawCmd[i]);
    }
    for(i=0;i!=result.cmdNumber;++i) {
        free(rawCmd[i]);
    }
    return cmdCheckedLine(&result);
}

pid_t execute(Line * line) {
    int i=0;
    for (i=0;i!=line->cmdNumber;++i) {
        pid_t pid = fork();
        if (pid==0) {
            #if DEBUG
            printf("executing: %s\n",line->cmd[i].argv[0]);
            printf("%d created ",getpid());
            if (line->background) {
                printf("in background\n");;
            } else {
                printf("\n");
            }
            #endif
            int j=0;
            for (j=0;j!=line->cmd[i].argc;++j) {
                printf("*%s*",line->cmd[i].argv[i]);
            }
            execvp(line->cmd[i].argv[0],line->cmd[i].argv);
            if (line->cmdNumber==1) {
                printf("myshell: '%s': No such file or directory\n",line->cmd[i].argv[0]);
            } else {
                printf("myshell: Fail to execute '%s': No such file or directory\n",line->cmd[0].argv[0]);
            }
            freeLine(line);
            return 0;
        } else {
            int status=0;
            waitpid(pid,&status,0);
        }
    }
    freeLine(line);
    return getpid();
}

int main(int argc, const char ** argv) {
    char * buffer=(char*)malloc(sizeof(char)*BUFFER_SIZE);
    char * line =nullptr;
    size_t bufferSize = BUFFER_SIZE;
    Line input;
    while (true) {
        printf("## myshell $ ");
        ssize_t size = getline(&buffer,&bufferSize,stdin);
        if (size>1) {
            line = strndup(buffer,(size_t)--size);
            input = parse(line);
            if (input.cmdNumber==-1) {
                break;
            }
            if (input.valid) {
                pid_t result = execute(&input);
                if (result==0) {
                    break;
                }
            }
            free(line);
        }
    }
    #if DEBUG
    printf("%d exit\n",getpid());
    #endif
    free(buffer);
    return 0;
}