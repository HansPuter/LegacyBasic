/* LBasic  (LegacyBasic)
 *
 * (c) 2023 by Hans-Peter Rampp - LGPL 2
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "token.h"
#include "list.h"

NODE *tokenlist = NULL;
NODE *scan      = NULL;
int   lineno    = 0;
int   vars[26];

TOKEN *getNextToken (char **pos);
int tokenize (char *line);
void   run();
int    getNumericExpr ();
int    getNumericValue();
enum TOKENDEF getTokenValue();
void   doPrint();
int    doRnd();
void   doIf();
void   doGoto();

int main (int argc, char *argv[])
{
    if (argc <= 1)
    {
        printf("usage: tbas basic-file\n");
        return 1;
    }
    
    FILE *fin = fopen(argv[1], "r");
    if (NULL == fin)
    {
        printf("error: couldn't open %s\n", argv[1]);
        return 2;
    }
     
    char line[1024];
    while(fgets(line, sizeof(line)-1, fin))
    {
        printf(line);

        ++lineno;
        if (!tokenize(line))
        {
            //fclose(fin), fin=NULL;
            break;
        }
    }

    dumplist(tokenlist);
    fclose(fin), fin=NULL;

    memset(vars, 0, sizeof(vars));
    srand(time(0));
    run();
    
    freetokens (&tokenlist);
    return 0;
}

/*
 * Tokenize source line
 */
int tokenize (char *line)
{
    char *s = line;
    
    TOKEN *t = getNextToken(&s);
    if (!t)
        return 0;
        
    if (t->type == NUMBER)
        t->type = LINENO;
    addtoken (&tokenlist, t);
    
    while ((t = getNextToken(&s)))
    {
        if (t->type == REM)
            break;

        addtoken (&tokenlist, t);
    }
    
    return 1;
}

const char *delimiters = "+-*/><=(),:";

/*
 * Scan for next token
 */
TOKEN *getNextToken (char **pos)
{
    char *s = *pos;
    while (isspace(*s))
        ++s;
    
    if (*s) {
        if (isdigit(*s) || (*s == '-' && isdigit(*(s+1)))) {
            int minus = 1;
            if (*s == '-') {
                minus = -1;
                ++s;
            }
            
            int no = *s - '0';
            char *t = s + 1;
            while (isdigit(*t)) {
                no *= 10;
                no += *t - '0';
                ++t;
            }
            
            if (isalpha(*t)) {
                printf("error: invalid number in line %d\n", lineno);
                exit(1);
            }
            
            TOKEN *token = (TOKEN*)malloc(sizeof(TOKEN));
            token->type = NUMBER;
            token->data.no = no * minus;
            *pos = t;
            return token;
        }
        
        if (*s == '"') {
            char str[1024];
            int  idx=0;
            char *t = s + 1;
            for ( ; *t != 0; ++t) {
                if (*t == '"') {
                    if  (*(t+1) == '"') {
                        str[idx++] = *t;
                        ++t;
                    }
                    else {
                        str[idx++] = 0;
                        ++t;
                        break;
                    }
                    
                    continue;
                }

                str[idx++] = *t;
            }
            
            TOKEN *token = (TOKEN*)malloc(sizeof(TOKEN));
            token->type = STRING;
            token->data.str = strdup(str);
            *pos = t;
            return token;
        }
        
        if (strchr(delimiters, *s)) {
            TOKEN *token = (TOKEN*)malloc(sizeof(TOKEN));
            token->data.no = 0;

            switch (*s) {
                case '+':
                    token->type = ADD; break;
                case '-':
                    token->type = SUB; break;
                case '*':
                    token->type = MUL; break;
                case '/':
                    token->type = DIV; break;
                case '<':
                    token->type = SMALLER; break;
                case '>':
                    token->type = GREATER; break;
                case '=':
                    token->type = EQUALS; break;
                case '(':
                    token->type = OPENBRACKET; break;
                case ')':
                    token->type = CLOSEBRACKET; break;
                case ',':
                    token->type = COMMA; break;
                case ':':
                    token->type = COLON; break;
                default:
                    free(token); return NULL;
            }
            *pos = s+1;
            return token;
        }
        
        if (isalpha(*s)) {
            char keyword[16];
            int  idx=0;
            char *t = s;
            while (isalpha(*t)) {
                keyword[idx++] = islower(*t) ? toupper(*t) : *t;
                ++t;
            }
            keyword[idx] = 0;
            
            size_t len = strlen(keyword);
            if (len == 1) {
                TOKEN *token = (TOKEN*)malloc(sizeof(TOKEN));
                token->type = VARIABLE;
                token->data.no = (islower(*s) ? toupper(*s) : *s) - 'A';
                *pos = t;
                return token;
            }
            
            TOKEN *token = (TOKEN*)malloc(sizeof(TOKEN));
            if (!strcmp(keyword, "PRINT")) 
                token->type = PRINT;
            else if (!strcmp(keyword, "IF")) 
                token->type = IF;
            else if (!strcmp(keyword, "THEN")) 
                token->type = THEN;
            else if (!strcmp(keyword, "GOTO")) 
                token->type = GOTO;
            else if (!strcmp(keyword, "INPUT")) 
                token->type = INPUT;
            else if (!strcmp(keyword, "LET")) 
                token->type = LET;
            else if (!strcmp(keyword, "GOSUB")) 
                token->type = GOSUB;
            else if (!strcmp(keyword, "RETURN")) 
                token->type = RETURN;
            else if (!strcmp(keyword, "CLEAR")) 
                token->type = CLEAR;
            else if (!strcmp(keyword, "END")) 
                token->type = END;
            else if (!strcmp(keyword, "REM")) 
                token->type = REM;
            else if (!strcmp(keyword, "RND")) {
                token->type = RND;
                token->data.no = NUMFUNC;
            }
            else {
                printf("error: unknown token: %s at line %d\n", keyword, lineno);
                exit(1);
            }
            
            *pos = t;
            return token;
        }
    }
    
    return NULL;
}

/*
 * Evaluate BASIC tokens
 */
void run()
{
    int   var;
    scan = tokenlist;
    lineno = 0;
    
    if (!scan)
        return;
        
    do {
        switch (scan->token->type) {
            case LINENO: 
                lineno = scan->token->data.no;
                scan = scan->next;
                break;
            
            case LET:
                scan = scan->next;
                if (scan->token->type != VARIABLE) {
                    printf("Error: variable expected at line: %d ", lineno); 
                }
            case VARIABLE:
                var = scan->token->data.no;
                
                scan = scan->next;
                if (scan->token->type != EQUALS) {
                    printf("Error: assignment expected at line: %d ", lineno); 
                }
                
                scan = scan->next;
                vars[var] = getNumericExpr ();
                break;

            case PRINT:
                doPrint();
                break;

            case IF:
                doIf();
                break;
                
            case GOTO:
                doGoto();
                break;
            
            case END:
                exit(0);
                
            default: 
                printf("Unknown token: %d at %d", scan->token->type, lineno); 
                exit(1);
        }
    } while (scan);
}

/*
 * Get numerical expression
 */
int getNumericExpr ()
{
    int result = getNumericValue();
    enum TOKENDEF tok = NONE;

/*
    if (scan->token->type == OPENBRACKET) {
        scan = scan->next;
        return getNumericExpr();
    }
*/    
   
    while (scan->token->type == ADD || scan->token->type == SUB || scan->token->type == MUL ||
             scan->token->type == DIV) {
        tok = getTokenValue();
        int val = getNumericValue();
        
        switch (tok) {
            case ADD:
                result += val; break;
            case SUB:
                result -= val; break;
            case MUL:
                result *= val; break;
            case DIV:
                result /= val; break;
            default:
                break;
        }
    }
    
    return result;
}

int getNumericValue()
{
    int retVal = 0;
    
    switch (scan->token->type) {
        case NUMBER:
            retVal = scan->token->data.no; break;
        case VARIABLE:
            retVal = vars[scan->token->data.no]; break;
        case OPENBRACKET:
            scan = scan->next;
            retVal = getNumericExpr(); break;
        case RND:
            scan = scan->next;
            retVal = doRnd(); break;
        default:
            printf("Error: invalid value or expression at line: %d ", lineno);
            exit(1);
    }
        
    scan = scan->next;
    return retVal;
}

enum TOKENDEF getTokenValue()
{
    enum TOKENDEF tok;
    
    switch (scan->token->type) {
        case ADD:
        case SUB:
        case MUL:
        case DIV:
            tok = scan->token->type;
            break;
            
        default:
            printf("Error: expecting operator at line: %d ", lineno);
            exit(1);
    }
        
    scan = scan->next;
    return tok;
}

void doPrint()
{
    scan = scan->next;
    if (scan->token->type == STRING) {
        printf("%s", scan->token->data.str);
        scan = scan->next;
    }
    else if (scan->token->type != COLON && scan->token->type != LINENO)
        printf("%d ", getNumericExpr());
        
    if (scan->token->type == COMMA)
        scan = scan->next;
    else
        printf("\n");
}

int doRnd()
{
    if (scan->token->type != OPENBRACKET) {
        printf("Error: open bracket expected at line: %d ", lineno);
        exit(1);
    }
    scan = scan->next;
    
    int retVal = ((long)rand() * (long)getNumericExpr()) / RAND_MAX;

    while (scan && scan->token->type != CLOSEBRACKET) 
        scan = scan->next;

    return retVal;
}

void doIf()
{
    scan = scan->next;
    int expr1 = getNumericExpr();    

    enum TOKENDEF tok = scan->token->type;
    if (tok != GREATER && tok != SMALLER && tok != EQUALS) {
        printf("Error: operator expected at line: %d ", lineno);
        exit(1);
    }

    scan = scan->next;
    int expr2 = getNumericExpr();    
    
    if (scan->token->type != THEN && scan->token->type != GOTO) {
        printf("Error: THEN expected at line: %d ", lineno);
        exit(1);
    }
    
    scan = scan->next;
    if ((tok == GREATER && expr1 < expr2) || (tok == SMALLER && expr1 > expr2) || (tok == EQUALS && expr1 != expr2)) {
        while (scan && scan->token->type != LINENO)
            scan = scan->next;
    }
}

void doGoto()
{
    scan = scan->next;
    int line = getNumericExpr();    

    NODE *n = tokenlist;
    while (n) {
        if (n->token->type == LINENO && n->token->data.no == line) {
            scan = n;
            return;
        }
        n = n->next;
    }
    
    printf("Error: goto destination not found at line: %d ", lineno);
    exit(1);
}

