#include "util.h"
#include <unistd.h>

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
        buffer[j]='\n'; //combined;
        ++j;
    }
    result[lastSize+j]='\0';
    free(lastOne);
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

PIDNode * buildFromBuffer(char * buffer, PIDNode** iterator,ssize_t size) {
    PIDNode * head=nullptr;
    if (size==BUFFER_SIZE&&buffer[size]!='\n') {
        while(buffer[size]!='\n') {
            --size;
        }
    }
    ssize_t i=0;
    ssize_t j=0;
    while (j!=size) {
        i=j;
        while(buffer[j]!='\n') {
            ++j;
        }
        char * pid=copy(buffer,i,j);
        if ((*iterator)==nullptr) {
            (*iterator)=buildPIDNode(atoi(pid));
            head=*iterator;
        } else {
            (*iterator)->next=buildPIDNode(atoi(pid));
            if ((*iterator)->next!=nullptr) {
                (*iterator)=(*iterator)->next;
            }
        }
        ++j;
    }
    return head;
}

PIDNode * getChildren(pid_t pid) {
    int pfd[2];
    pipe(pfd);
    pid_t pgrepPID=0;
    if ((pgrepPID=fork())==0) {
        close(pfd[0]);
        dup2(pfd[1],1);
        char * PID=(char *)malloc(sizeof(char)*MAX_PROC_FILE_PATH);
        sprintf(PID,"%d",pid);
        char *command[4]={(char*)"pgrep",(char*)"-P",PID,nullptr};
        execvp(command[0],command);
        return nullptr;
    } else if (pgrepPID>0){
        close(pfd[1]);
        char *lastOne=nullptr;
        char buffer[BUFFER_SIZE];
        char * combined=nullptr;
        ssize_t readSize=0;
        PIDNode * pidList=nullptr;
        PIDNode * iterator=nullptr;
        do {
            memset(buffer,0,BUFFER_SIZE*sizeof(char));
            readSize=read(pfd[0],buffer,BUFFER_SIZE);
            if (readSize!=0) {
                if (lastOne!=nullptr) {
                    combined=buffer[0]=='\n'?lastOne:combine(buffer,lastOne);
                    lastOne=nullptr;
                    iterator->next=buildPIDNode(atoi(combined));
                    if (iterator->next!=nullptr) {
                        iterator=iterator->next;
                    }
                    free(combined);
                    combined=nullptr;
                }
                if (pidList==nullptr) {
                    pidList=buildFromBuffer(buffer,&iterator,readSize);
                } else {
                    buildFromBuffer(buffer,&iterator,readSize);
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


PIDNode * buildTree(PIDNode* root) {
    root->child=getChildren(root->PID);
    PIDNode * iterator=root->child;
    while (iterator!=nullptr) {
        iterator=buildTree(iterator);
        iterator=iterator->next;
    }
    return root;
}

void printTree(PIDNode * root) {
    printf("%s",root->name);
    PIDNode * iterator=root->child;
    if (iterator!=nullptr) {
        printf("-");
    } else {
        printf("\n");
    }
    if (iterator!=nullptr) {
        printTree(iterator);
        iterator=iterator->next;
    }
    while (iterator!=nullptr) {
        ssize_t j=0;
        for(j=0;j!=strlen(root->name);++j) {
            printf(" ");
        }
        printf("-");
        printTree(iterator);
        iterator=iterator->next;
    }
}

// /home/zhchen/HKU/COMP3230/assignments/myshell/test/loopf 3600&
PIDNode * freeTree(PIDNode * root) {
    if (root!=nullptr) {
        freeTree(root->next);
        freeTree(root->child);
        free(root->name);
        free(root);
    }
}

void viewTree() {
    PIDNode * root=buildPIDNode(getpid());
    root = buildTree(root);
    printTree(root);
    root=freeTree(root);
    return;
}
