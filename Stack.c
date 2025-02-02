#include <stdlib.h>
#include <stdio.h>
#include "Stack.h"

Stack labelStack = { NULL };
Stack logicStack = { NULL };

// Allgemeine Push-Funktion
void stackPush(Stack* stack, long data) {
    StackNode* newNode = (StackNode*)malloc(sizeof(StackNode));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation failed for push operation.\n");
        exit(1);
    }
    newNode->data = data;
    newNode->next = stack->top;
    stack->top = newNode;
}

// Allgemeine Pop-Funktion
long stackPop(Stack* stack) {
    if (stack->top == NULL) {
        fprintf(stderr, "Stack is empty. Cannot pop.\n");
        return 0;
    }

    StackNode* tempNode = stack->top;
    long data = stack->top->data;
    stack->top = stack->top->next;
    free(tempNode);

    return data;
}
