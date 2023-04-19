/*
 * Evaluate BASIC tokens
 * (c) 2023 by Hans-Peter Rampp - LGPL 2
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "token.h"
#include "list.h"
#include "run.h"

static NODE *scan = NULL;
static int   vars[26];
static int   lineno;

void run(NODE *tokenlist)
{
    int   var;
    NODE *scan = tokenlist;
    lineno = 0;

    if (!scan)
        return;
        
    memset(vars, 0, sizeof(vars));

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
                doGoto(tokenlist);
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

void doGoto(NODE *tokenlist)
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
