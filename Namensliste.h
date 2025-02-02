#ifndef NAMENSLISTE_H
#define NAMENSLISTE_H

#include <stdint.h>

#define VERARBEITUNGSBREITE 2

typedef enum Kennzeichen {
    Prozedur, 
    Bezeichner, 
    Konstante, 
    Variable
}Kennzeichen;

// Namenslisteneintrag
typedef struct tBEZ {
    Kennzeichen Kz;
    struct tBEZ* pNext;
    short idxProc;
    void* pObj;
    char* pName;
}tBez;

// Variablenbeschreibung
typedef struct tVAR {
    Kennzeichen Kz;
    int Dspl;   // Relativadresse
}tVar;

// Konstantenbeschreibung
typedef struct tCONST {
    Kennzeichen Kz;
    int32_t Val;
    int Idx;    // Index der Konstanten im Konstantenblock 
}tConst;

// Prozedurbeschreibung
typedef struct tPROC {
    Kennzeichen Kz;
    short IdxProc;  // Prozedurnummer
    struct tPROC* pParent;
    tBez* pLBez;
    int SpzzVar;    // Speicherplatzzuordnungszähler f. Variablen
}tProc;

extern tProc* currentProc;

extern int procCounter;

tBez* createBez(char* pBez);
tConst* createConst(int32_t Val);
tConst* searchConst(int32_t Val);
tVar* createVar(void);
tProc* createProc(tProc* pParent);
tBez* searchBez(tProc* pProc, char* pBez);
tBez* searchBezGlobal(char* pBez);
void clearCurrentProc();

#endif