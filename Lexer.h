#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>

#define OK 0
#define FAIL -1

typedef enum MorphemeCode {
    mcEmpty, 
    mcSymbol, 
    mcNumber, 
    mcIdentifier
} MorphemeCode;

typedef struct {
    MorphemeCode morphemeCode;
    int line;
    int column;
    union Value {
        long Number;
        char* String;
        int Symbol;
    } Val;
    int length;
} morphem;

extern morphem Morph;

int initLex(char* fname);
morphem* Lex(void);

#endif // LEXER_H
