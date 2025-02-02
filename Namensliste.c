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
        //fprintf(stderr, "Could not allocate memory\n");
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
            //fprintf(stderr, "Could not allocate memory for pName\n");
            return NULL;
        }
        strcpy(bezeichner->pName, pBez);
    }
    return bezeichner;
}

tConst* createConst(int32_t Val) {
    tConst* cons = (tConst* )malloc(sizeof(tConst));
    if(cons == NULL) {
        //fprintf(stderr, "Could not allocate memory\n");
        return NULL;
    }
    // pObj des letzten Bezeichners setzen 
    if(currentProc->pLBez == NULL) {
        //fprintf(stderr, "Could not find free bez\n");
        return NULL;
    } else {
        tBez* it = currentProc->pLBez;
        while(it->pNext != NULL) {
            it = it->pNext;
        }
        if(it->pObj == NULL) {
            it->pObj = cons;
        } else {
            //fprintf(stderr, "Could not find free bez\n");
            return NULL;
        }
    }

    cons->Kz = Konstante;
    cons->Val = Val;

    return cons;
}

tConst* searchConst(int32_t Val) {
    tBez* it = currentProc->pLBez;
    if(it == NULL) {
        //fprintf(stderr, "Could not find const\n");
        return NULL;
    }
    
    if(it->Kz == Konstante) {
        tConst* cons = (tConst* )it->pObj;
        if(cons == NULL) return NULL;
        if(cons->Val == Val) {
            return cons;
        }
    }
    while(it->pNext != NULL) {
        it = it->pNext;
        tConst* cons = (tConst* )it->pObj;
        if(cons->Kz == Konstante) {
            if(cons->Val == Val) {
                return cons;
            }
        }
    }
    
    return NULL;
}

tVar* createVar(void) {
    tVar* var = (tVar* )malloc(sizeof(tVar));
    if(var == NULL) {
        //fprintf(stderr, "Could not allocate memory\n");
        return NULL;
    }
    // pObj des letzten Bezeichners setzen 
    if(currentProc->pLBez == NULL) {
        //fprintf(stderr, "Could not find free bez\n");
        return NULL;
    } else {
        tBez* it = currentProc->pLBez;
        while(it->pNext != NULL) {
            it = it->pNext;
        }
        if(it->pObj == NULL) {
            it->pObj = var;
        } else {
            //fprintf(stderr, "Could not find free bez\n");
            return NULL;
        }
    }

    var->Kz = Variable;
    currentProc->SpzzVar += VERARBEITUNGSBREITE;
    var->Dspl = currentProc->SpzzVar - 4;

    return var;
}

tProc* createProc(tProc* pParent) {
    tProc* proc = (tProc* )malloc(sizeof(tProc));
    if(proc == NULL) {
        //fprintf(stderr, "Could not allocate memory\n");
        return NULL;
    }
    // pObj des letzten Bezeichners setzen 
    if(currentProc->pLBez == NULL) {
        //fprintf(stderr, "Could not find free bez\n");
        return NULL;
    } else {
        tBez* it = currentProc->pLBez;
        while(it->pNext != NULL) {
            it = it->pNext;
        }
        if(it->pObj == NULL) {
            it->pObj = proc;
        } else {
            //fprintf(stderr, "Could not find free bez\n");
            return NULL;
        }
    }

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
    tBez* it = pProc->pLBez;
    if(it == NULL) {
        //fprintf(stderr, "Could not find bez\n");
        return NULL;
    }
    if(strcmp(pBez, it->pName) == 0) return it;
    while(it->pNext != NULL) {
        it = it->pNext;
        if(strcmp(pBez, it->pName) == 0) return it;
    }
    
    return NULL;
}

/*tBez* searchBezGlobal(char* pBez) {
    tProc* itProc = currentProc;
    tBez* itBez = itProc->pLBez;
    
    while(itProc != NULL) {
        if(itBez == NULL) {
            itProc = itProc->pParent;
            itBez = itProc->pLBez;
        } else {
            if(itBez->pName != NULL) {
                if(strcmp(pBez, itBez->pName) == 0) return itBez;    
            }
            
            while(itBez->pNext != NULL) {
                itBez = itBez->pNext;
                if(itBez->pName != NULL) {
                    if(strcmp(pBez, itBez->pName) == 0) return itBez;
                }
            }
            itProc = itProc->pParent;
            itBez = itProc->pLBez;
        }
    }
    
    return NULL;
}*/

tBez* searchBezGlobal(char* pBez) {
    tProc* itProc = currentProc;

    while (itProc != NULL) {
        tBez* itBez = itProc->pLBez;

        while (itBez != NULL) {
            if (itBez->pName != NULL && strcmp(pBez, itBez->pName) == 0) {
                return itBez;  // Gefunden
            }
            itBez = itBez->pNext;
        }

        // Zum Elternprozess wechseln
        itProc = itProc->pParent;
    }

    return NULL;  // Nicht gefunden
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
