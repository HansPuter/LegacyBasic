/*
 * Header for run
 * (c) 2023 by Hans-Peter Rampp - LGPL 2
 */

void   run(NODE *tokenlist);
int    getNumericExpr ();
int    getNumericValue();
enum TOKENDEF getTokenValue();
void   doPrint();
int    doRnd();
void   doIf();
void   doGoto(NODE *tokenlist);
