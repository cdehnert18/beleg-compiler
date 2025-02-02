#include "Namensliste.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

tProc hauptProgramm =   {   .Kz = Prozedur,
                            .IdxProc = 0,
                            .pParent = NULL,
                            .pLBez = NULL,
                            .SpzzVar = 0
                        };

tProc* currentProc = &hauptProgramm;
int procCounter = 1;

tBez* createBez(char* pBez) {
    
    tBez* bezeichner = (tBez* )malloc(sizeof(tBez));
    if(bezeichner == NULL) {
        return NULL;
    }
    
    // pNext des Vorgängers setzen 
    if(currentProc->pLBez == NULL) {
        currentProc->pLBez = bezeichner;
    } else {
        tBez* it = currentProc->pLBez;
        while(it->pNext != NULL) {
            it = it->pNext;
        }
        it->pNext = bezeichner;
    }

    bezeichner->Kz = Bezeichner;
    bezeichner->pNext = NULL;
    bezeichner->idxProc = currentProc->IdxProc;
    bezeichner->pObj = NULL;
    
    if(pBez == NULL) bezeichner->pName = NULL;
    else {
        bezeichner->pName = malloc(strlen(pBez) + 1);
        if (bezeichner->pName == NULL) {
            return NULL;
        }
        strcpy(bezeichner->pName, pBez);
    }
    return bezeichner;
}

tConst* createConst(int32_t Val) {
    
    tConst* cons = (tConst*)malloc(sizeof(tConst));
    if (cons == NULL) {
        return NULL;
    }

    if (currentProc == NULL || currentProc->pLBez == NULL) {
        fprintf(stderr, "Error: Invalid current process or missing identifier list\n");
        free(cons);
        return NULL;
    }

    tBez* currentBez = currentProc->pLBez;
    while (currentBez->pNext != NULL) {
        currentBez = currentBez->pNext;
    }

    if (currentBez->pObj != NULL) {
        fprintf(stderr, "Error: Last identifier already in use\n");
        free(cons);
        return NULL;
    }
    currentBez->pObj = cons;

    cons->Kz = Konstante;
    cons->Val = Val;

    return cons;
}


tConst* searchConst(const int32_t Val) {

    if (currentProc == NULL || currentProc->pLBez == NULL) {
        return NULL;
    }

    // Iteration durch die Bezeichnerliste
    for (tBez* currentBez = currentProc->pLBez; currentBez != NULL; currentBez = currentBez->pNext) {
        // Überprüfen, ob der Bezeichner eine Konstante ist
        if (currentBez->Kz == Konstante && currentBez->pObj != NULL) {
            tConst* constant = (tConst*)currentBez->pObj;

            if (constant->Val == Val) {
                return constant;
            }
        }
    }

    return NULL;
}

tVar* createVar(void) {
    tVar* var = (tVar* )malloc(sizeof(tVar));
    if(var == NULL) {
        return NULL;
    }

    // Letztes Element in der Bezeichnerliste finden
    tBez* currentBez = currentProc->pLBez;
    while (currentBez->pNext != NULL) {
        currentBez = currentBez->pNext;
    }

    // Überprüfung: Kann pObj gesetzt werden?
    if (currentBez->pObj != NULL) {
        fprintf(stderr, "No free identifier available for variable\n");
        free(var); // Speicher freigeben
        return NULL;
    }
    currentBez->pObj = var;

    var->Kz = Variable;
    currentProc->SpzzVar += VERARBEITUNGSBREITE;
    var->Dspl = currentProc->SpzzVar - VERARBEITUNGSBREITE;

    return var;
}

tProc* createProc(tProc* pParent) {
    
    if (currentProc == NULL || currentProc->pLBez == NULL) {
        return NULL;
    }

    tProc* proc = (tProc*)malloc(sizeof(tProc));
    if (proc == NULL) {
        return NULL; 
    }

    tBez* it = currentProc->pLBez;
    while (it->pNext != NULL) {
        it = it->pNext;
    }

    if (it->pObj != NULL) {
        free(proc);
        return NULL;
    }
    it->pObj = proc;

    proc->Kz = Prozedur;
    proc->IdxProc = procCounter;
    proc->pParent = pParent;
    proc->pLBez = NULL;
    proc->SpzzVar = 0;

    procCounter++;

    currentProc = proc;

    return proc;
}

tBez* searchBez(tProc* pProc, char* pBez) {
    if (pProc == NULL || pBez == NULL) {
        return NULL;
    }
    for (tBez* it = pProc->pLBez; it != NULL; it = it->pNext) {
        if (strcmp(pBez, it->pName) == 0) {
            return it;
        }
    }
    return NULL;
}

tBez* searchBezGlobal(char* pBez) {
    tProc* itProc = currentProc;

    while (itProc != NULL) {
        tBez* itBez = itProc->pLBez;

        while (itBez != NULL) {
            if (itBez->pName != NULL && strcmp(pBez, itBez->pName) == 0) {
                return itBez;
            }
            itBez = itBez->pNext;
        }

        itProc = itProc->pParent;
    }

    return NULL;
}

void clearCurrentProc() {
    while(currentProc->pLBez != NULL) {
        tBez* it = currentProc->pLBez;
        currentProc->pLBez = it->pNext;

        Kennzeichen k = it->Kz;
        void* pObj = it->pObj;

        switch(k) {
            case Prozedur:  
            {
                tProc* p = (tProc*)pObj;
                free(p);
            }
            break;  
            case Konstante: 
            {
                tConst* p = (tConst*)pObj;
                free(p);
            }
            break;
            case Variable:
            {
                tVar* p = (tVar*)pObj;
                free(p);
            }
            break;
            default:
            break;
        }
        free(it);
    }
    currentProc = currentProc->pParent;
}
