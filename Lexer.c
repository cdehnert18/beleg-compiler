#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "Lexer.h"

/* Zeichenklassen */
// 0 Sonderzeichen ('+', '-', '*', '/', ',', '.', ';', '(', ')', '?', '!', '#')
// 1 Ziffern
// 2 Buchstaben, kein SW
// 3 :
// 4 =
// 5 <
// 6 >
// 7 Sonstige

// Erweiterung
// 8 Buchstabe, evnt SW

/* Zeichenklassenvektor */
static unsigned char zeichenklassenvektor[128]=
/*     0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F     */
/*--------------------------------------------------------*/
/* 0*/{7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,/* 0*/
/*10*/ 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,/*10*/
/*20*/ 7, 0, 7, 0, 7, 7, 7, 7, 0, 0, 0, 0, 0, 0, 0, 0,/*20*/
/*30*/ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 0, 5, 4, 6, 0,/*30*/
/*40*/ 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 2, 2, 8, 2, 8, 8,/*40*/
/*50*/ 8, 2, 8, 8, 8, 8, 8, 8, 2, 2, 2, 7, 7, 7, 7, 7,/*50*/
/*60*/ 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 2, 2, 8, 2, 8, 8,/*60*/
/*70*/ 8, 2, 8, 8, 8, 8, 8, 8, 2, 2, 2, 7, 7, 0, 0, 0}/*70*/;

/* Schalt- und Ausgabefunktionen des Automaten */
static void lesen(void);
static void beenden(void);
static void schreiben_gross_lesen(void);
static void schreiben_lesen(void);
static void schreiben_lesen_beenden(void);

typedef void (*funktionszeiger)(void);
static funktionszeiger funktionszeiger_array[] = {lesen, beenden, schreiben_gross_lesen, schreiben_lesen, schreiben_lesen_beenden};

/* Funktionsindex *0x10, bzw. *16*/
typedef enum T_Fx {
    i_lesen = 0x0,
    i_beenden = 0x10,
    i_schreiben_gross_lesen = 0x20,
    i_schreiben_lesen = 0x30,
    i_schreiben_lesen_beenden = 0x40
} tFx;

typedef enum SchlusselwortCode {
    /*              */  zNIL,
    /* :=           */  zERG = 128,
    /* <=           */  zLE,
    /* >=           */  zGE,
    /* BEGIN        */  zBGN,
    /* CALL         */  zCLL,
    /* CONST        */  zCST,
    /* DO           */  zDO,
    /* END          */  zEND,
    /* IF           */  zIF,
    /* ODD          */  zODD,
    /* PROCEDURE    */  zPRC,
    /* THEN         */  zTHN,
    /* VAR          */  zVAR,
    /* WHILE        */  zWHL,
    // Erweiterung
    /* AND          */  zAND,
    /* OR           */  zOR
} SchlusselwortCode;

static char automatentabelle[][9]={
/*    Sonderzeichen                  Ziffer                 Buchstabe(kein SW)          ':'                    '='                            '<'                    '>'                     Sonstige       Buchstabe(evnt SW)  */
/*------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
/*0*/{0 + i_schreiben_lesen_beenden, 1 + i_schreiben_lesen, 2 + i_schreiben_gross_lesen, 3 + i_schreiben_lesen, 0 + i_schreiben_lesen_beenden, 4 + i_schreiben_lesen, 5 + i_schreiben_lesen, 0 + i_lesen,   9 + i_schreiben_gross_lesen},
/*1*/{1 + i_beenden                , 1 + i_schreiben_lesen, 1 + i_beenden              , 1 + i_beenden        , 1 + i_beenden                , 1 + i_beenden        , 1 + i_beenden        , 1 + i_beenden, 1 + i_beenden},
/*2*/{2 + i_beenden                , 2 + i_schreiben_lesen, 2 + i_schreiben_gross_lesen, 2 + i_beenden        , 2 + i_beenden                , 2 + i_beenden        , 2 + i_beenden        , 2 + i_beenden, 2 + i_schreiben_gross_lesen},
/*3*/{3 + i_beenden                , 3 + i_beenden        , 3 + i_beenden              , 3 + i_beenden        , 6 + i_schreiben_lesen        , 3 + i_beenden        , 3 + i_beenden        , 3 + i_beenden, 3 + i_beenden},
/*4*/{4 + i_beenden                , 4 + i_beenden        , 4 + i_beenden              , 4 + i_beenden        , 7 + i_schreiben_lesen        , 4 + i_beenden        , 4 + i_beenden        , 4 + i_beenden, 4 + i_beenden},
/*5*/{5 + i_beenden                , 5 + i_beenden        , 5 + i_beenden              , 5 + i_beenden        , 8 + i_schreiben_lesen        , 5 + i_beenden        , 5 + i_beenden        , 5 + i_beenden, 5 + i_beenden},
/*6*/{6 + i_beenden                , 6 + i_beenden        , 6 + i_beenden              , 6 + i_beenden        , 6 + i_beenden                , 6 + i_beenden        , 6 + i_beenden        , 6 + i_beenden, 6 + i_beenden}, 
/*7*/{7 + i_beenden                , 7 + i_beenden        , 7 + i_beenden              , 7 + i_beenden        , 7 + i_beenden                , 7 + i_beenden        , 7 + i_beenden        , 7 + i_beenden, 7 + i_beenden}, 
/*8*/{8 + i_beenden                , 8 + i_beenden        , 8 + i_beenden              , 8 + i_beenden        , 8 + i_beenden                , 8 + i_beenden        , 8 + i_beenden        , 8 + i_beenden, 8 + i_beenden},

// Erweiterung
/*9*/{9 + i_beenden                , 2 + i_schreiben_lesen, 2 + i_schreiben_gross_lesen, 9 + i_beenden        , 9 + i_beenden                , 9 + i_beenden        , 9 + i_beenden        , 9 + i_beenden, 9 + i_schreiben_gross_lesen}};

static FILE* file;
static morphem MorphInit;   // alles mit 0 belegt
extern morphem Morph;       // globale Variable f. Akt. Token
static int eingabeZeichen;
static int Z;               // Aktueller Zustand
static char vBuf[1024+1];   // Buffer zum Sammeln
static char* pBuf;          // Pointer in den Buffer
static int currentLine, currentColumn; // Zeile und Spalte
static int End;

/*---- Initialisierung der lexiaklischen Analyse ----*/
int initLex(char* fname) {
    char vName[128+1];
    strcpy(vName, fname);
    if (strstr(vName,".pl0") == NULL) strcat(vName, ".pl0");
    file = fopen(vName, "r");
    if (file != NULL) {
        currentLine = 1;
        currentColumn = 0; 
        eingabeZeichen = fgetc(file); 
        if (eingabeZeichen == EOF) {
            return FAIL;
        }
        return OK;
    }
    
    return FAIL;
}

morphem* Lex(void) {
    Z = 0;                  // Anfangszustand
    int zx;                 // Zustand merken
    Morph = MorphInit;      // Morphem mit 0 löschen
    pBuf = vBuf;            // Pointer auf Bufferanfang
    End = 0;

    if (eingabeZeichen == EOF) {
        Morph.morphemeCode = mcEmpty;
        End = 1;                       
        return &Morph;
    }

    do {
        zx = automatentabelle[Z][zeichenklassenvektor[eingabeZeichen&0x7f]]&0xF;
        funktionszeiger_array[(automatentabelle[Z][zeichenklassenvektor[eingabeZeichen&0x7f]])>>4]();
        Z = zx;
    } while (End == 0); // (Z!=zEnd) // zEnd:9
    
    Morph.line = currentLine;
    Morph.column = currentColumn;

    return &Morph;
}

/*---- lesen ----*/
static void lesen(void) { 
    eingabeZeichen = fgetc(file);
    if (eingabeZeichen == '\n') {
        currentLine++;
        currentColumn = 0;
    } else currentColumn++;
}

/*---- schreiben als Grossbuchstabe, lesen ----*/
static void schreiben_gross_lesen(void) {
    *pBuf=(char)toupper(eingabeZeichen);// oder *Buf=(char)X&0xdf;
    *(++pBuf) = 0;
    lesen();
}

/*---- schreiben, lesen ----*/
static void schreiben_lesen(void) {
    *pBuf=(char)eingabeZeichen;
    *(++pBuf)=0;
    lesen();
}

/*---- schreiben, lesen, beenden ----*/
static void schreiben_lesen_beenden(void) {
    schreiben_lesen();
    beenden();
}

static void beenden() {
    /* Schlüsselwörter */
    const char* schlusselworter[] = {
        "BEGIN", "CALL", "CONST", "DO", "END", 
        "IF", "ODD", "PROCEDURE", "THEN", "VAR", "WHILE", "AND", "OR"
    };

    switch (Z) {
        /* einzelnes Symbol */
        case 0: Morph.Val.Symbol = vBuf[0];
                Morph.morphemeCode = mcSymbol;
                break;
        
        /* Zahl */
        case 1: Morph.Val.Number = atol(vBuf);
                Morph.morphemeCode = mcNumber;
                break;

        /* Bezeichner */
        case 2: Morph.Val.String = vBuf;
                Morph.morphemeCode = mcIdentifier;
                break;

        /* Bezeichner/Schlüsselwort*/
        case 9: Morph.Val.String = vBuf;
                Morph.morphemeCode = mcIdentifier;

                for(int i = 0; i < (sizeof(schlusselworter) / sizeof(schlusselworter[0])); i++) {
                    if (strcmp(vBuf, schlusselworter[i]) == 0) {
                        Morph.Val.Symbol = i + 131;
                        Morph.morphemeCode = mcSymbol;
                        break;
                    }
                }
                break;

        /* : */
        case 3: 
        /* < */
        case 4:
        /* > */
        case 5: Morph.Val.Symbol = vBuf[0];
                Morph.morphemeCode = mcSymbol;
                break;
        
        /* := */
        case 6: Morph.Val.Symbol = zERG;
                Morph.morphemeCode = mcSymbol;
                break;
        /* <= */
        case 7: Morph.Val.Symbol = zLE;
                Morph.morphemeCode = mcSymbol;
                break;
        /* >= */
        case 8: Morph.Val.Symbol = zGE;
                Morph.morphemeCode = mcSymbol;
                break;
    }

    End = 1;
}