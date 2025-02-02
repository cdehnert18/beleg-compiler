#include "Namensliste.h"

typedef struct StackNode {
    long iJmp;
    struct StackNode* next;
} StackNode;

void pushLabel(long iJmp);
long popLabel();

void pushLogic(long iJmp);
long popLogic();