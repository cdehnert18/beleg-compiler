#include <stdlib.h>
#include <stdio.h>

typedef struct StackNode {
    long data;
    struct StackNode* next;
} StackNode;

typedef struct Stack {
    StackNode* top;
} Stack;

extern Stack labelStack;
extern Stack logicStack;

void stackPush(Stack* stack, long data);
long stackPop(Stack* stack);
