#include <stdlib.h>
#include <stdio.h>
#include "Label.h"

StackNode* stackTop = NULL;
StackNode* stackTopLogic = NULL;

void pushLabel(long iJmp) {
    StackNode* newNode = (StackNode*)malloc(sizeof(StackNode));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation failed for push operation.\n");
        exit(1);
    }
    newNode->iJmp = iJmp;
    newNode->next = stackTop;
    stackTop = newNode;
}

long popLabel() {
    if (stackTop == NULL) {
        fprintf(stderr, "Stack is empty. Cannot pop.\n");
        return 0;
    }
    
    StackNode* tempNode = stackTop;
    long iJmp = stackTop->iJmp;
    
    stackTop = stackTop->next;
    free(tempNode);
    
    return iJmp;
}

void pushLogic(long iJmp) {
    StackNode* newNode = (StackNode*)malloc(sizeof(StackNode));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation failed for push operation.\n");
        exit(1);
    }
    newNode->iJmp = iJmp;
    newNode->next = stackTopLogic;
    stackTopLogic = newNode;
}

long popLogic() {
    if (stackTopLogic == NULL) {
        //fprintf(stderr, "Stack is empty. Cannot pop.\n");
        return 0;
    }
    
    StackNode* tempNode = stackTopLogic;
    long iJmp = stackTopLogic->iJmp;
    
    stackTopLogic = stackTopLogic->next;
    free(tempNode);
    
    return iJmp;
}