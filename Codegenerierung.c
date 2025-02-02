#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include "Codegenerierung.h"
#include "Namensliste.h"
#include "Lexer.h"
#include "Label.h"

char* vCode;            /* dynamischen Bereich des Codes */
unsigned short pc;      /* aktuelle Position im Code (Programmcounter)*/
int LenCode = 1;        /* Länge des Codeausgabebereiches */

FILE* file;

char* constBlock;
int constCounter;

int compOp;

int code(tCode Code, ...) {
  va_list ap;
  short sarg;
  if (pc + MAX_LEN_OF_CODE >= LenCode) {
    char* xCode = realloc(vCode, (LenCode += 1024));
    if (xCode == NULL) /*Error(ENoMem)*/ exit(1);
    vCode = xCode;
  }
  vCode[pc++] = (char)Code;
  va_start(ap, Code);
  
  switch (Code) {
    /* Befehle mit 3 Parametern */
    case entryProc:
      sarg = va_arg(ap, int);
      wr2ToCode(sarg);
    
    /* Befehle mit 2 Parametern */
    case puValVrGlob:
    case puAdrVrGlob:
      sarg = va_arg(ap, int);
      wr2ToCode(sarg);
    
    /* Befehle mit 1 Parameter */
    case puValVrMain:
    case puAdrVrMain:
    case puValVrLocl:
    case puAdrVrLocl:
    case puConst:
    case jmp :
    case jnot:
    case call:
      sarg = va_arg(ap, int);
      wr2ToCode(sarg);
    break;
    
    /* keine Parameter */
    default: break;
  }
  va_end(ap);

  return 1;
}

// Abschlussarbeiten
int Pr1() {
    // da bl5 bei 'statement' aufgerufen wird: kein erneuter Aufruf
    
    /* Schreiben des Konstantenblocks */
    fwrite(constBlock, sizeof(char), constCounter * VERARBEITUNGSBREITE, file);

    /* Schreiben der Anzahl der Prozeduren in das Codefile am Anfang */
    // Erfolgt bei closeCodeFile()

  return 1;
}

// Konstantenbezeichner
int bl1() {
  /* lokale Suche nach dem Bezeichner
      – gefunden -> Fehlerbehandlung
      – nicht gefunden -> Bezeichner anlegen
  */
  char* value = Morph.Val.String;
  if(searchBez(currentProc, value) != NULL) {
    return -1;
  }
  
  tBez* b = createBez(value);
  b->Kz = Konstante;

  return 1;
}

// Konstantenwert
int bl2() {
  /*  
    – Konstantenbeschreibung anlegen
    – Suche nach Konstante im Konstantenblock
    – gefunden -> Index der Konstanten eintragen in Konstantenbeschreibung
    – Konstante anlegen im Konstantenblock und Index der Konstanten eintragen in Konstantenbeschreibung
    – In letzten Bezeichner Zeiger auf Konstante eintragen
  */
  
  /*
  int32_t value = Morph.Val.Number;
  tConst* oldConst = searchConst(value);
  if(oldConst != NULL) {
    tConst* newConst = createConst(value);
    newConst->Idx = oldConst->Idx;
    constCounter--;
  } else {
    createConst(value);

    // Konstante in Konstantenblock schreiben
    addToConstBlock(value);
  }

  return 1;*/
  int32_t value = Morph.Val.Number;
  tConst* newConst = createConst(value);

  short found = -1;
  for(int i = 0; i < constCounter; i++) {
    int32_t result = 0;
    // Bytes aus constBlock lesen und zusammensetzen
    for (int v = 0; v < VERARBEITUNGSBREITE; v++) {
      result |= (int32_t)constBlock[i * VERARBEITUNGSBREITE + v] << (v * 8);
      if(result == value) {
        found = i;
        break;
      }
    }
  }

  if(found != -1) {
    newConst->Idx = found;
  } else {
    newConst->Idx = constCounter;
    addToConstBlock(value);
  }

  return 1;
}

// Variablenbezeichner
int bl3() {
  /*
    – lokale Suche nach dem Bezeichner
    – gefunden -> Fehlerbehandlung
    – nicht gefunden -> Bezeichner anlegen
    – Variablenbeschreibung anlegen und Pointer in Bezeichner eintragen
    – Relativadresse ermitteln aus SpzzVar, SpzzVar um 4 erhöhen
      (Virtuelle Maschine arbeitet mit 4 Byte langen long-Werten)
  */
  
  char* value = Morph.Val.String;
  if(searchBez(currentProc, value) != NULL) {
    return -1;
  }
  
  tBez* b = createBez(value);
  b->Kz = Variable;
  createVar();
  //currentProc->SpzzVar += 4; // Passiert bereits in Namensliste

  return 1;
}

// Prozedurbezeichner
int bl4() {
  /*
    – lokale Suche nach dem Bezeichner
    – gefunden -> Fehlerbehandlung
    – nicht gefunden -> Bezeichner anlegen
    – Prozedurbeschreibung anlegen
    – Pointer auf Parent-Prozedur eintragen
    – Pointer auf Prozedurbeschreibung in letzten Bezeichner eintragen
    – Neue Prozedur ist jetzt aktuelle Prozedur (Passiert in Namensliste)
  */
  char* value = Morph.Val.String;
  if(searchBez(currentProc, value) != NULL) {
    return -1;
  }

  tBez* b = createBez(value);
  b->Kz = Prozedur;
  createProc(currentProc);

  return 1;
}

// Ende der Prozedurvereinbarung
int bl5() {
  code(retProc);
  /*Codelänge in den Befehl entryProc als 1. Parameter nachtragen*/
  vCode[1] = (unsigned char)(pc & 0xff);
  vCode[2] = (unsigned char)(pc >> 8);

  writeCodeToFile();

  clearCurrentProc();
  
  free(vCode);

  return 1;
}

// Beginn des Anweisungsteils
int bl6() {
  vCode = malloc(sizeof(char));
  pc = 0;
  LenCode = 1;
  // Codelänge (temp 0), ProzedurID, Länge Variablenblock
  code(entryProc, 0, currentProc->IdxProc+, currentProc->SpzzVar);
  return 1;
}

// Zuweisung
int st1() {
  /*
    Bezeichner global suchen
      nicht gefunden -> Fehlerbehandlung
      gefunden -> ok.
    Bezeichnet der Bezeichner eine Variable?
      Nein, eine Konstante oder Prozedur -> Fehlerbehandlung
      ja -> ok,
    Codegenerierung
  */
  tBez* b = searchBezGlobal(Morph.Val.String);
  if(b == NULL) {
    return -1;
  }
  if(b->Kz != Variable) {
    return -1;
  }
  
  tVar* v = (tVar*)b->pObj;

  if(/*currentProc->IdxProc*/b->idxProc == 0) {
    code(puAdrVrMain, v->Dspl);
    return 1;
  }

  if(currentProc->IdxProc == b->idxProc) {
    code(puAdrVrLocl, v->Dspl);
    return 1;
  }

  code(puAdrVrGlob, v->Dspl, b->idxProc);

  return 1;
}

// Rechte Seite der Zuweisung
int st2() {
  code(storeVal);
  return 1;
}

int st3() {
  pushLabel(pc + 1);
  code(jnot, 0);
  return 1;
}

int st4() {
  long iJmp = popLabel();
  unsigned short relAdd = pc - iJmp;

  vCode[iJmp] = (unsigned char)((relAdd - 2) & 0xff);
  vCode[iJmp + 1] = (unsigned char)((relAdd - 2) >> 8);
  
  return 1;
}

int st5() {
  pushLabel(pc + 1);
  return 1;
}

int st6() {
  pushLabel(pc + 1);
  code(jnot, 0);
  return 1;
}

int st7() {
  
  long labelJMPN = popLabel();
  long labelJMP = popLabel();
  code(jmp, (short)labelJMP - (pc + 4));
  
  unsigned short relAdd = pc - labelJMPN;
  
  vCode[labelJMPN] = (unsigned char)((relAdd - 2) & 0xff);
  vCode[labelJMPN + 1] = (unsigned char)((relAdd - 2) >> 8);
  
  return 1;
}

// Prozeduraufruf
int st8() {
  /*
  Bezeichner global suchen
    Nicht gefunden -> Fehlerbehandlung
    Gefunden -> ok.
  Bezeichnet der Bezeichner eine Procedure?
    Nein, eine Konstante oder Prozedur -> Fehlerbehandlung
    Ja -> ok
  Codegenerierung call procedurenumber
  */
  tBez* b = searchBezGlobal(Morph.Val.String);
  if(b == NULL) {
    return -1;
  }
  if(b->Kz != Prozedur) {
    return -1;
  }
  tProc* p = (tProc*)b->pObj;
  code(call, p->IdxProc);
  return 1;
}

// Eingabe
int st9() {
  /*
  Bezeichner global suchen:
    nicht gefunden -> Fehlerbehandlung
    gefunden -> ok.
  Bezeichnet der Bezeichner eine Variable?
    Nein, eine Konstante oder Prozedur -> Fehlerbehandlung
    ja -> ok,
  */
  tBez* b = searchBezGlobal(Morph.Val.String);
  if(b == NULL) {
    return -1;
  }
  if(b->Kz != Variable) {
    return -1;
  }
  
  tVar* v = (tVar*)b->pObj;

  if(/*currentProc->IdxProc*/b->idxProc == 0) {
    code(puAdrVrMain, v->Dspl);
    code(getVal);
    return 1;
  }

  if(currentProc->IdxProc == b->idxProc) {
    code(puAdrVrLocl, v->Dspl);
    code(getVal);
    return 1;
  }

  code(puAdrVrGlob, v->Dspl, b->idxProc);
  code(getVal);

  return 1;
}

// Ausgabe
int st10() {
  code(putVal);
  return 1;
}

int ex1() {
  code(vzMinus);
  return 1;
}

int ex2() {
  code(OpAdd);
  return 1;
}

int ex3() {
  code(OpSub);
  return 1;
}

int te1() {
  code(OpMult);
  return 1;
}

int te2() {
  code(OpDiv);
  return 1;
}

int fa1() {
  int32_t value = Morph.Val.Number;
  tConst* cons = searchConst(value);
  if(cons == NULL) {
    tBez* b = createBez(NULL);
    b->Kz = Konstante;
    cons = createConst(value);
    
    // Konstante in Konstantenblock schreiben
    cons->Idx = constCounter;
    addToConstBlock(value);
  }

  code(puConst, cons->Idx);

  return 1;
}

int fa2() {
  tBez* b = searchBezGlobal(Morph.Val.String);
  if(b == NULL) {
    return -1;
  }

  if(b->Kz == Prozedur) {
    return -1;
  }

  if(b->Kz == Variable) {
    tVar* v = (tVar*)b->pObj;
    
      if(b->idxProc == 0) {
        code(puValVrMain, v->Dspl);
        return 1;
      }

      if(currentProc->IdxProc == b->idxProc) {
        code(puValVrLocl, v->Dspl);
        return 1;
      }

      printf("GLOBAL\n");
      code(puValVrGlob, v->Dspl, b->idxProc);
      return 1;
  }

  if(b->Kz == Konstante) {
    tConst* c = (tConst*)b->pObj;
    code(puConst, c->Idx);

    return 1;
  }

  // Sollte nie erreicht werden
  return -1;
}

int co1() {
  code(odd);
  return 1;
}

int co2() {
  compOp = Morph.Val.Symbol;
  return 1;
}

int co3() {
  compOp = Morph.Val.Symbol;
  return 1;
}

int co4() {
  compOp = Morph.Val.Symbol;
  return 1;
}

int co5() {
  compOp = Morph.Val.Symbol;
  return 1;
}

int co6() {
  compOp = Morph.Val.Symbol;
  return 1;
}

int co7() {
  compOp = Morph.Val.Symbol;
  return 1;
}

int co8() {
  switch(compOp) {
    case '=': code(cmpEQ);
              break;
    case '#': code(cmpNE);
              break;
    case '<': code(cmpLT);
              break;
    case 129: code(cmpLE);
              break;
    case '>': code(cmpGT);
              break;
    case 130: code(cmpGE);
              break;
  }
  return 1;
}

void wr2ToCode(short x) {
  vCode[pc++] = (unsigned char)(x & 0xff);
  vCode[pc++] = (unsigned char)(x >> 8);
}

void wr2ToCodeAtP(short x, char* pD) {
  * pD = (unsigned char)(x & 0xff);
  *(pD + 1) = (unsigned char)(x >> 8);
}

int writeCodeToFile() {
  if (pc == fwrite(vCode, sizeof(char), pc, file)) {
    return 1;
  } else return -1;
}

int openCodeFile(char* arg) {
  long i = 0L;  // Reserviere vier Byte für Anzahl Prozeduren & Verarbeitungsbreite
  char vName[128 + 1];

  strcpy(vName, arg);
  if(strstr(vName,".pl0") == NULL) strcat(vName,".cl0");
  else *(strchr(vName, '.') + 1) = 'c';

  if ((file = fopen(vName,"wb+")) != NULL) {
    fwrite(&i, sizeof(int32_t), 1, file);
    return 1;
  }
  else return -1;
}

int closeCodeFile() {
  // Puffer für Anzahl Prozeduren & Verarbeitungsbreite
  char vBuf[4];
  wr2ToCodeAtP(procCounter, vBuf);
  wr2ToCodeAtP(VERARBEITUNGSBREITE, &vBuf[2]);
  
  fseek(file, 0, SEEK_SET);
  if(fwrite(vBuf, 4, 1, file) == 4) {
    fclose(file);
    return 1;
  } else {
    fclose(file);
    return -1;
  }
}

void addToConstBlock(int32_t value) {
    constCounter++;
    constBlock = realloc(constBlock, (constCounter * VERARBEITUNGSBREITE));
    
    constBlock[(constCounter - 1) * VERARBEITUNGSBREITE] = (unsigned char)(value & 0xff);
    for(int v = 1; v < VERARBEITUNGSBREITE; v++) {
      constBlock[(constCounter - 1) * VERARBEITUNGSBREITE + v] = (unsigned char)(value >> v * 8);
    }
}

int and() {
  pushLogic(1);
  return 1;
}

int or() {
  pushLogic(2);
  return 1;
}

int lo() {
  long l;
  while((l = popLogic()) != 0) {
    if(l == 1) code(OpMult);
    else code(OpAdd);
  }
  return st3();
}