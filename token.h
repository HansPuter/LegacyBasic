/* Token definitions
 * (c) 2023 by Hans-Peter Rampp - LGPL 2
*/

/*********************************************
"+"      { return ADD; }
"-"      { return SUB; }
"*"      { return MUL; }
"/"      { return DIV; }
">"      { return GREATER; }
"<"      { return SMALLER; }
"="      { return EQUALS; }
"("      { return OPENBRACKET; }
")"      { return CLOSEBRACKET; }
^[1-9][0-9]+ { return LINENO; };
[0-9]+   { return NUMBER; }
[a-zA-Z] { return VARIABLE; }
"PRINT"  { return PRINT; }
"IF"     { return IF; }
"GOTO"   { return GOTO; }
"INPUT"  { return INPUT; }
"LET"    { return LET; }
"GOSUB"  { return GOSUB; }
"RETURN" { return RETURN; }
"CLEAR"  { return CLEAR; }
"LIST"   { return LIST; }
"RUN"    { return RUN; }
"END"    { return END; }
"REM"    { return REM; }
"RND"    { return RND; }
********************************************/

enum TOKENDEF { NONE=0, ADD, SUB, MUL, DIV, 
    GREATER=10, SMALLER, EQUALS, 
    OPENBRACKET=20, CLOSEBRACKET, COMMA, COLON,
    LINENO=30, NUMBER, STRING, VARIABLE,
    PRINT=40, IF, THEN, GOTO, INPUT, LET, GOSUB, RETURN, 
    CLEAR, END, REM, 
    RND=100 
};

enum FUNCTYPE {
    NUMFUNC = 1,
    STRINGFUNC
};

