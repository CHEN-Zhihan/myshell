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
        buffer[j]='x'; //combined;
        ++j;
    }
    result[lastSize+j]='\0';
    free(lastOne);
    return result;
}

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

PIDNode * buildFromBuffer(char * buffer, PIDNode** iterator,ssize_t size,pid_t lsPID) {
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
        if (lsPID!=atoi(pid)) {
            if ((*iterator)==nullptr) {
                (*iterator)=buildPIDNode(atoi(pid));
                head=*iterator;
            } else {
                (*iterator)->next=buildPIDNode(atoi(pid));
                if ((*iterator)->next!=nullptr) {
                    (*iterator)=(*iterator)->next;
                }
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
    pid_t lsPID=0;
    if ((lsPID=fork())==0) {
        close(pfd[0]);
        dup2(pfd[1],1);
        char *command[3]={(char*)"ls",(char*)"/proc",nullptr};
        execvp(command[0],command);
        return nullptr;
    } else if (lsPID>0) {
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
                    if (isPID(combined)&&atoi(combined)!=lsPID) {
                        iterator->next=buildPIDNode(atoi(combined));
                        if (iterator->next!=nullptr) {
                            iterator=iterator->next;
                        }
                    }
                    free(combined);
                    combined=nullptr;
                }
                if (pidList==nullptr) {
                    pidList=buildFromBuffer(buffer,&iterator,readSize,lsPID);
                } else {
                    buildFromBuffer(buffer,&iterator,readSize,lsPID);
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
        printf("beginning: Found %d %s with PPID: %d\n",listIterator->PID,listIterator->name,listIterator->PPID);
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
            printf("middle: Found %d %s with PPID: %d\n",listIterator->next->PID,listIterator->next->name,listIterator->next->PPID);
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
        free(temp->name);
        free(temp);
    }
    return nullptr;
}



PIDNode * buildTree(PIDNode **list, PIDNode* root) {
    root->child=extract(list,root->PID);
    PIDNode * iterator=root->child;
    while (iterator!=nullptr) {
        iterator=buildTree(list,iterator);
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
        for(j=0;j!=strlen(root->name)+1;++j) {
            printf(" ");
        }
        printTree(iterator);
        iterator=iterator->next;
    }
}

PIDNode * freeTree(PIDNode * root) {
    if (root!=nullptr) {
        freeTree(root->next);
        freeTree(root->child);
        free(root->name);
        free(root);
    }
}

void viewTree() {
    PIDNode * list = getPIDList();
    PIDNode * root=buildPIDNode(getpid());
    root = buildTree(&list,root);
    printTree(root);
    list=freeList(list);
    root=freeTree(root);
    return;
}