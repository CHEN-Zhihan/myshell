#include "execute.h"
#include <unistd.h>
#include <errno.h>
#include <wait.h>
//#define BUFFER_SIZE 3
void run_command(Command *cmd, int is_background) {

    if (is_background) {
        setpgid(0, 0);
    }
    execvp(cmd->argv[0], cmd->argv);
    fprintf(stderr, "myshell: '%s': %s\n", cmd->argv[0], strerror(errno));
    exit(EXIT_FAILURE);
}

void wait_wrapped(int pid, int is_background, int flag) {
	if (!is_background) {
        if (flag) {
        	waitid(P_PID, pid, NULL, WNOWAIT | WEXITED);
        	print_timeX(pid);
        }
        waitpid(pid, NULL, 0);
    }
}

char * getLastOne(char * buffer) {
    ssize_t i=BUFFER_SIZE-1;
    while (buffer[i]!='\n') {
        --i;
    }
    ssize_t size=BUFFER_SIZE-i-1;
    char * lastOne=(char*)malloc(sizeof(char)*(size+1));
    ssize_t j=0;
    while (j!=size) {
        lastOne[j]=buffer[i+j+1];
        ++j;
    }
    lastOne[j]='\0';
    return lastOne;
}

char* combine(char * buffer,char *lastOne) {
    ssize_t i=0;
    while (buffer[i]!='\n') {
        ++i;
    }
    ssize_t lastSize=strlen(lastOne);
    ssize_t size=lastSize+i;
    char * result=(char*)malloc(sizeof(char)*(size+1));
    ssize_t j=0;
    while (j!=lastSize) {
        result[j]=lastOne[j];
        ++j;
    }
    j=0;
    while (j!=i) {
        result[lastSize+j]=buffer[j];
        buffer[j]='x'; //combined;
        ++j;
    }
    result[lastSize+j]='\0';
    free(lastOne);
    return result;
}

typedef struct PIDNode {
    pid_t PID;
    pid_t PPID;
    char * name;
    struct PIDNode *next;
} PIDNode;

bool isDigit(char c) {
    return c>='0'&&c<='9';
}

bool isPID(char *str) {
    ssize_t i=0;
    ssize_t j=strlen(str);
    for (i=0;i!=j;++i) {
        if (!isDigit(str[i])) {
            return false;
        }
    }
    return true;
}


PIDNode * buildPIDNode(char *inp) {
    pid_t pid;
    char* name=malloc(sizeof(char)*MAX_PROC_FILE_PATH);
    unsigned long ut, st;
    pid_t ppid;

    char str[MAX_PROC_FILE_PATH];
    sprintf(str, "/proc/%d/stat",atoi(inp));
    FILE *file = fopen(str, "r");
    if (file == NULL) {
        printf("Error in open my proc file\n");
        exit(0);
    }
    int z;
    unsigned long h;
    char stat;
    fscanf(file, "%d %s %c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu", &pid, name, &stat, &ppid, &z, &z, &z, &z,
           (unsigned *)&z, &h, &h, &h, &h, &ut, &st);
    fclose(file);

    PIDNode * result = (PIDNode*)(malloc(sizeof(PIDNode)));
    result->PPID=ppid;
    result->PID=pid;
    result->name=name;
    result->next=nullptr;
    return result;
}


char * copy(char * buffer,ssize_t i, ssize_t j) {
    char * result=(char*)malloc(sizeof(char)*(j-i+1));
    ssize_t c=0;
    while (c!=j-i) {
        result[c]=buffer[i+c];
        ++c;
    }
    result[c]='\0';
    return result;
}

PIDNode * buildFromBuffer(char * buffer, PIDNode** iterator,ssize_t size,pid_t childPid) {
    PIDNode * head=nullptr;
    if (size==BUFFER_SIZE&&buffer[size]!='\n') {
        while(buffer[size]!='\n') {
            --size;
        }
    }
    ssize_t i=0;
    ssize_t j=0;
    while (i!=size) {
        while (!isDigit(buffer[i])&&i<size) {
            ++i;
        }
        if (i>=size) {
            break;
        }
        j=i;
        while(isDigit(buffer[j])) {
            ++j;
        }
        char * pid=copy(buffer,i,j);
        i=j;
        if (atoi(pid)!=childPid) {
            if ((*iterator)==nullptr) {
                (*iterator)=buildPIDNode(pid);
                head=*iterator;
            } else {
                (*iterator)->next=buildPIDNode(pid);
                (*iterator)=(*iterator)->next;
            }
        }
        free(pid);
        pid=nullptr;
    }
    return head;
}

PIDNode * getPIDList() {
    int pfd[2];
    pipe(pfd);
    pid_t childPid=0;
    if ((childPid=safe_fork())==0) {
        close(pfd[0]);
        dup2(pfd[1],1);
        char *command[3]={(char*)"ls",(char*)"/proc",nullptr};
        execvp(command[0],command);
        return nullptr;
    } else {
        close(pfd[1]);

        char *lastOne=nullptr;
        char buffer[BUFFER_SIZE];
        char * combined=nullptr;
        ssize_t readSize=0;
        PIDNode * pidList=nullptr;
        PIDNode * iterator=nullptr;
        waitpid(childPid,nullptr,0);
        do {
            memset(buffer,0,BUFFER_SIZE*sizeof(char));
            readSize=read(pfd[0],buffer,BUFFER_SIZE);
            if (readSize!=0) {
                if (lastOne!=nullptr) {
                    combined=buffer[0]=='\n'?lastOne:combine(buffer,lastOne);
                    lastOne=nullptr;
                    if (isPID(combined)&&atoi(combined)!=childPid) {
                        iterator->next=buildPIDNode(combined);
                        iterator=iterator->next;
                    }
                    free(combined);
                    combined=nullptr;
                }
                if (pidList==nullptr) {
                    pidList=buildFromBuffer(buffer,&iterator,readSize,childPid);
                } else {
                    buildFromBuffer(buffer,&iterator,readSize,childPid);
                }
                if (readSize==BUFFER_SIZE &&buffer[readSize-1]!='\n') {
                    lastOne=getLastOne(buffer);
                } else {
                    lastOne=nullptr;
                }
            }
        } while (readSize==BUFFER_SIZE);
        return pidList;
    }
}

PIDNode * extract(PIDNode ** list,pid_t ppid) {
    PIDNode *result=nullptr;
    PIDNode * resultIterator=nullptr;
    PIDNode * listIterator=*list;
    if (listIterator==nullptr) {
        return nullptr;
    }
    while(listIterator!=nullptr&&listIterator->PPID==ppid) {
        if (result==nullptr) {
            result=listIterator;
            resultIterator=result;
        } else {
            resultIterator->next=listIterator;
            resultIterator=resultIterator->next;
        }
        listIterator=listIterator->next;
        *list=listIterator;
        resultIterator->next=nullptr;
    }
    while (listIterator!=nullptr&&listIterator->next!=nullptr) {
        if (listIterator->next->PPID==ppid) {
            if (result==nullptr) {
                result=listIterator->next;
                resultIterator=result;
            } else {
                resultIterator->next=listIterator->next;
                resultIterator=resultIterator->next;
            }
            listIterator->next=listIterator->next->next;
            resultIterator->next=nullptr;
        }
        listIterator=listIterator->next;
    }
    return result;
}

PIDNode * freeList(PIDNode * list) {
    PIDNode * iterator=list;
    PIDNode * temp=nullptr;
    while (iterator!=nullptr) {
        temp=iterator;
        iterator=iterator->next;
        free(temp);
    }
    return nullptr;
}

struct PIDTreeNode {
    PIDNode * node;
    PIDNode * child;
} PIDTreeNode;

void viewTree() {
    PIDNode * list = getPIDList();
//    PIDNode * iterator=list;
//    PIDTreeNode root = buildTree(list,getpid());
//    printTree(root);
//    freeList(list);
    PIDNode * gg = extract(&list,getpid());
    PIDNode *iterator=gg;
    while (iterator!=nullptr) {
        printf("%s\n",iterator->name);
        iterator=iterator->next;
    }
    freeList(list);
    freeList(gg);
    return;
}


int safe_fork() {
	int pid = fork();
	if (pid == -1) {
		fprintf(stderr, "can not fork\n");
        return -1;
	}
	return pid;
}


void execute(struct Line *line) {

	if (line->type ==EXIT_TYPE) {
		fprintf(stderr, "myshell: Terminated\n");
		exit(EXIT_SUCCESS);
	} else if (line->type==VIEWTREE_TYPE) {
        viewTree();
    } else {
		if (line->head->next == NULL) {
            int pid = safe_fork();
        	if (pid == 0) {
        		run_command(line->head, line->background);
        	}else if (pid > 0) {
        		wait_wrapped(pid, line->background, line->type);
        	}
		} else {

			Command *iterator = line->head;

			int pipefd[MAX_PIPE_NUMBER][2];
	    	pid_t pid_list[MAX_PIPE_NUMBER] = {0};
	    	int pipe_number = 0;

	        pipe(pipefd[pipe_number]);

	        pid_t pid = safe_fork();
	        pid_list[pipe_number] = pid;

	        if (pid == 0) {
	        	pipe_out(pipefd[pipe_number]);
	            run_command(iterator, line->background);
	        }

	        while(iterator->next->next != NULL) {
	            
	            iterator = iterator -> next;
	            ++pipe_number;
	            pipe(pipefd[pipe_number]);
	            pid_t pid = safe_fork();
	            pid_list[pipe_number] = pid;
	            if(pid == 0) {
	            	pipe_in(pipefd[pipe_number-1]);
	                pipe_out(pipefd[pipe_number]);
	                run_command(iterator, line->background);
	            }else if (pid > 0) {
	            	close_pipe(pipefd[pipe_number-1]);
	            }
	        }

	        iterator = iterator->next;

	        pid = safe_fork();

	        if (pid == 0) {
	            pipe_in(pipefd[pipe_number]);
	            run_command(iterator, line->background);
	        }else if (pid > 0) {
	            close_pipe(pipefd[pipe_number]);
	        }

	        wait_wrapped(pid, line->background, line->type);

	        for (size_t i = 0; i < pipe_number + 1; i++) {
	            wait_wrapped(pid, line->background, line->type);
	        }
		}
	}
}




void print_timeX(int pid) {
    int pid_get;
    char cmd[MAX_PROC_FILE_PATH];
    unsigned long ut, st;


    char str[MAX_PROC_FILE_PATH];
    sprintf(str, "/proc/%d/stat", pid);
    FILE *file = fopen(str, "r");
    if (file == NULL) {
        printf("Error in open my proc file\n");
        exit(0);
    }

    int z;
    unsigned long h;
    char stat;

    fscanf(file, "%d %s %c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu", &pid_get, cmd, &stat, &z, &z, &z, &z, &z,
           (unsigned *)&z, &h, &h, &h, &h, &ut, &st);
    fclose(file);

    cmd[strlen(cmd) - 1] = 0;


    double utime = ut*1.0f/sysconf(_SC_CLK_TCK);
    double stime = st*1.0f/sysconf(_SC_CLK_TCK);

    printf("\n");
    printf("%-10s%-15s%-10s%-10s%-10s\n", "PID", "CMD", "RTIME", "UTIME", "STIME");
    printf("%-10d%-15s%-4.2lf%-6s%-4.2lf%-6s%-4.2lf%-6s\n", pid_get, cmd+1, utime + stime, " s", utime," s", stime," s");
}
