#include <stdio.h>
#include <stdlib.h>
#include "Lexer.h"
#include "Codegenerierung.h"

typedef enum BG_TYP {
    BgNichts = 0,             // NIL
    BgSymbol = 1,           // Symbol
    BgMorphem = 2,          // Morphem
    BgGraph = 4,            // Graph
    BgGraphEnde = 8,        // Graphende (altenativ 0 oder -1 als Folgebogen)
}BgTyp;

typedef struct BOGEN {
    BgTyp bgTyp;                // Bogentyp (Nil, Symbol, Name/Zahl, Graph)
    union BGX {                 // Bogenbeschreibung
        unsigned long X;            // für Initialisierung notwendig
        int symbol;                      // Symbol (Ascii oder Wortsymbolcode aus enum)
        MorphemeCode morphemeCode;  // Morphemtyp (Zahl oder Bezeichner)
        struct BOGEN* graph;        // Verweis auf Graph
    } BgX;
    int (*fx)(void);            // Funktionspointer (Funktion, wenn Bogen akzeptiert)
    int iNext;                  // Folgebogen, wenn Bogen akzeptiert
    int iAlt;                   // Alternativbogen, wenn Bogen nicht akzeptiert
                                // oder 0 oder -1, wenn es keinen
                                // Alternativbogen gibt.
}Bogen;

extern Bogen gBlock[];
extern Bogen gStat[];
extern Bogen gLog[];
extern Bogen gCond[];
extern Bogen gFact[];
extern Bogen gExpr[];
extern Bogen gTerm[];

Bogen gProg[]= {
    /* 0*/ {BgGraph,        {(unsigned long)gBlock},    NULL, 1, 0},
    /* 1*/ {BgSymbol,       {(unsigned long)'.'},       Pr1, 2, 0},
    /* 2*/ {BgGraphEnde,    {(unsigned long)0},         NULL, 0, 0}
};

Bogen gStat[]= {
    /* 0 */ {BgMorphem,     {(unsigned long)mcIdentifier},  st1, 1,    3},
    /* 1 */ {BgSymbol,      {(unsigned long)128/*ERG*/},    NULL, 2,    0},
    /* 2 */ {BgGraph,       {(unsigned long)gExpr},         st2, 22,   0},
    /* 3 */ {BgSymbol,      {(unsigned long)136/*IF*/},     NULL, 4,    7},
    /* 4 */ {BgGraph,       {(unsigned long)gLog},         /*st3*/NULL, 5,    0},
    /* 5 */ {BgSymbol,      {(unsigned long)139/*THE*/},    lo1, 6,    0},
    /* 6 */ {BgGraph,       {(unsigned long)gStat},         st4, 22,   0},
    /* 7 */ {BgSymbol,      {(unsigned long)141/*WHI*/},    st5, 8,    11},
    /* 8 */ {BgGraph,       {(unsigned long)gLog},          /*st6*/NULL, 9,    0},
    /* 9 */ {BgSymbol,      {(unsigned long)134/*DO*/},     lo2, 10,   0},
    /* 10*/ {BgGraph,       {(unsigned long)gStat},         st7, 22,   0},
    /* 11*/ {BgSymbol,      {(unsigned long)131/*BEG*/},    NULL, 12,   15},
    /* 12*/ {BgGraph,       {(unsigned long)gStat},         NULL, 13,   0},
    /* 13*/ {BgSymbol,      {(unsigned long)135/*END*/},    NULL, 22,   14},
    /* 14*/ {BgSymbol,      {(unsigned long)';'},           NULL, 12,   0},
    /* 15*/ {BgSymbol,      {(unsigned long)132/*CAL*/},    NULL, 16,   17},
    /* 16*/ {BgMorphem,     {(unsigned long)mcIdentifier},  st8, 22,   0},
    /* 17*/ {BgSymbol,      {(unsigned long)'?'},           NULL, 18,   19},
    /* 18*/ {BgMorphem,     {(unsigned long)mcIdentifier},  st9, 22,   0},
    /* 19*/ {BgSymbol,      {(unsigned long)'!'},           NULL, 20,   21},
    /* 20*/ {BgGraph,       {(unsigned long)gExpr},         st10, 22,   0},
    /* 21*/ {BgNichts,      {(unsigned long)0},             NULL, 22,   0},
    /* 22*/ {BgGraphEnde,   {(unsigned long)0},             NULL, 0,    0}
};

Bogen gBlock[]= {
    /* 0 */ {BgSymbol,      {(unsigned long)133/*CST*/},    NULL, 1,    6},
    /* 1 */ {BgMorphem,     {(unsigned long)mcIdentifier},  bl1,  2,    0},
    /* 2 */ {BgSymbol,      {(unsigned long)'='},           NULL, 3,    0},
    /* 3 */ {BgMorphem,     {(unsigned long)mcNumber},      bl2, 4,    0},
    /* 4 */ {BgSymbol,      {(unsigned long)','},           NULL, 1,    5},
    /* 5 */ {BgSymbol,      {(unsigned long)';'},           NULL, 7,    0},
    /* 6 */ {BgNichts,      {(unsigned long)0},             NULL, 7,    0},
    /* 7 */ {BgSymbol,      {(unsigned long)140/*VAR*/},    NULL, 8,    11},
    /* 8 */ {BgMorphem,     {(unsigned long)mcIdentifier},  bl3, 10,   0},
    /* 9 */ {BgSymbol,      {(unsigned long)','},           NULL, 8,    0},
    /* 10*/ {BgSymbol,      {(unsigned long)';'},           NULL, 12,   9},
    /* 11*/ {BgNichts,      {(unsigned long)0},             NULL, 12,   0},
    /* 12*/ {BgSymbol,      {(unsigned long)138/*PRO*/},    NULL, 13,   17},
    /* 13*/ {BgMorphem,     {(unsigned long)mcIdentifier},  bl4,  14,   0},
    /* 14*/ {BgSymbol,      {(unsigned long)';'},           NULL, 15,   0},
    /* 15*/ {BgGraph,       {(unsigned long)gBlock},        NULL, 16,   0},
    /* 16*/ {BgSymbol,      {(unsigned long)';'},           NULL, 12,   0},
    /* 17*/ {BgNichts,      {(unsigned long)0},             bl6, 18,   0},
    /* 18*/ {BgGraph,       {(unsigned long)gStat},         bl5, 19,   0},
    /* 19*/ {BgGraphEnde,   {(unsigned long)0},             NULL, 0,    0}
};

Bogen gLog[]= {
    /* 0*/  {BgGraph,       {(unsigned long)gCond},         NULL, 1,    0},
    /* 1*/  {BgSymbol,      {(unsigned long)/*AND*/142},    and, 2,    3},
    /* 2*/  {BgGraph,       {(unsigned long)gCond},         NULL, 1,    0},
    /* 3*/  {BgNichts,      {(unsigned long)0},             NULL, 4,    0},
    /* 4*/  {BgSymbol,      {(unsigned long)/*OR*/143},     or, 0,    5},
    /* 5*/  {BgGraphEnde,   {(unsigned long)0},             NULL, 0,    0},
};

Bogen gCond[]= {
    /* 0*/  {BgGraph,       {(unsigned long)gExpr},         NULL, 1,    8},
    /* 1*/  {BgSymbol,      {(unsigned long)'='},           co2, 7,    2},
    /* 2*/  {BgSymbol,      {(unsigned long)'#'},           co3, 7,    3},
    /* 3*/  {BgSymbol,      {(unsigned long)'<'},           co4, 7,    4},
    /* 4*/  {BgSymbol,      {(unsigned long)'>'},           co6, 7,    5},
    /* 5*/  {BgSymbol,      {(unsigned long)129/*LET*/},    co5, 7,    6},
    /* 6*/  {BgSymbol,      {(unsigned long)130/*GRT*/},    co7, 7,    0},
    /* 7*/  {BgGraph,       {(unsigned long)gExpr},         co8, 10,   0},
    /* 8*/  {BgSymbol,      {(unsigned long)137/*ODD*/},    NULL, 9,    0},
    /* 9*/  {BgGraph,       {(unsigned long)gExpr},         co1, 10,   0},
    /* 10*/ {BgGraphEnde,   {(unsigned long)0},             NULL, 0,    0}
};

Bogen gTerm[]= {
    /* 0*/ {BgGraph,        {(unsigned long)gFact}, NULL, 1, 0},
    /* 1*/ {BgNichts,       {(unsigned long)0},     NULL, 3, 0},
    /* 2*/ {BgGraphEnde,    {(unsigned long)0},     NULL, 0, 0},
    /* 3*/ {BgSymbol,       {(unsigned long)'*'},   NULL, 4, 5},
    /* 4*/ {BgGraph,        {(unsigned long)gFact}, te1, 1, 0},
    /* 5*/ {BgSymbol,       {(unsigned long)'/'},   NULL, 6, 2},
    /* 6*/ {BgGraph,        {(unsigned long)gFact}, te2, 1, 0}
};

Bogen gExpr[]= {
    /* 0*/ {BgSymbol,       {(unsigned long)'-'},   NULL, 1, 4},
    /* 1*/ {BgGraph,        {(unsigned long)gTerm}, ex1, 2, 0},
    /* 2*/ {BgNichts,       {(unsigned long)0},     NULL, 5, 0},
    /* 3*/ {BgGraphEnde,    {(unsigned long)0},     NULL, 0, 0},
    /* 4*/ {BgGraph,        {(unsigned long)gTerm}, NULL, 2, 0},
    /* 5*/ {BgSymbol,       {(unsigned long)'+'},   NULL, 6, 7},
    /* 6*/ {BgGraph,        {(unsigned long)gTerm}, ex2, 2, 0},
    /* 7*/ {BgSymbol,       {(unsigned long)'-'},   NULL, 8, 3},
    /* 8*/ {BgGraph,        {(unsigned long)gTerm}, ex3, 2, 0}
};

Bogen gFact[]= {
    /*      BogenTyp        Bogenbeschreibung               F.-P.   FolgeBogen  Alternativbogen*/
    /* 0*/ {BgMorphem,      {(unsigned long)mcIdentifier},  fa2, 5, 1},    /*(0)---ident--->(E)*/
    /* 1*/ {BgMorphem,      {(unsigned long)mcNumber},      fa1, 5, 2},    /* +---number--->(E)*/
    /* 2*/ {BgSymbol,       {(unsigned long)'('},           NULL, 3, 0},    /*(+)----'('---->(3)*/
    /* 3*/ {BgGraph,        {(unsigned long)gExpr},         NULL, 4, 0},    /*(1)---express->(4)*/
    /* 4*/ {BgSymbol,       {(unsigned long)')'},           NULL, 5, 0},    /*(0)----')'---->(E)*/
    /* 5*/ {BgGraphEnde,    {(unsigned long)0},             NULL, 0, 0}     /*(E)--------(ENDE) */
};

morphem Morph;

void printMorpheme(morphem* m) {
    printf("Line: %d, Column: %d: ", m->line, m->column);
    switch (m->morphemeCode) {
        case mcSymbol:
            /* zusammengesetzte Symbole*/
            if(m->Val.Symbol > 127) {
                switch(m->Val.Symbol) {
                    case 128: printf("Symbol: :="); break;
                    case 129: printf("Symbol: <="); break;
                    case 130: printf("Symbol: >="); break;
                    case 131: printf("Symbol: BEGIN"); break;
                    case 132: printf("Symbol: CALL"); break;
                    case 133: printf("Symbol: CONST"); break;
                    case 134: printf("Symbol: DO"); break;
                    case 135: printf("Symbol: END"); break;
                    case 136: printf("Symbol: IF"); break;
                    case 137: printf("Symbol: ODD"); break;
                    case 138: printf("Symbol: PROCEDURE"); break;
                    case 139: printf("Symbol: THEN"); break;
                    case 140: printf("Symbol: VAR"); break;
                    case 141: printf("Symbol: WHILE"); break;
                    case 142: printf("Symbol: AND"); break;
                    case 143: printf("Symbol: OR"); break;
                }
                printf("(Code: %d)\n", m->Val.Symbol);
            } else {
                printf("Symbol: %c\n", (char)m->Val.Symbol);
            }
            break;
        case mcNumber:
            printf("Number: %ld\n", m->Val.Number);
            break;
        case mcIdentifier:
            printf("Identifier: %s\n", m->Val.String);
            break;
        default:
            printf("Unknown Token: End of File reached.\n");
            break;
    }
}

int parse(Bogen* pGraph) {
    Bogen* pBog = pGraph;
    int succ = 0;
    if (Morph.morphemeCode == mcEmpty) { //Lex();
        morphem* m = Lex();
        printMorpheme(m);
    }
    while(1) {
        switch(pBog->bgTyp) {
            case BgNichts:
                succ = 1;
                break;
            case BgSymbol:
                succ = (Morph.Val.Symbol == pBog->BgX.symbol);
                break;
            case BgMorphem:
                succ = (Morph.morphemeCode == (MorphemeCode)pBog->BgX.morphemeCode);
                break;
            case BgGraph:
                succ = parse(pBog->BgX.graph);
                break;
            /* Ende erreichet - Erfolg */
            case BgGraphEnde:
                return 1;
        }
        if ((succ == 1) && pBog->fx != NULL) succ = pBog->fx();
        /* Alternativbogen probieren */
        if (succ != 1){         
            if (pBog->iAlt != 0) {
                pBog = pGraph + pBog->iAlt;
            } else return FAIL;
        }
        /* Morphem formal akzeptiert (eaten) */
        else {
            if (pBog->bgTyp & BgSymbol || pBog->bgTyp & BgMorphem) {
                morphem* m = Lex();
                if (m->morphemeCode == mcEmpty) {
                    printf("Parser: End of File reached.\n");
                } else {
                    printMorpheme(m);
                }
            }
            pBog = pGraph + pBog->iNext;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <input_filename>\n", argv[0]);
        return 1;
    }

    if (initLex(argv[1]) == FAIL) {
        printf("Failed to open file %s\n", argv[1]);
        return 1;
    }

    if(openCodeFile("output.pl0") == -1) exit(1);

    printf("Output Parser: %d\n", parse(gProg));
    closeCodeFile();
    return 0;
}
