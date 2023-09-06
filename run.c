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

static float getNumericValue();

static NODE *scan = NULL;
static float  vars[26];
static char  *svars[26];
static float *array[26]; 
static int   lineno;

static TOKEN *nextToken() 
{
    if ((scan = scan->next) == NULL) {
        printf("\nEnd of programm w/o END at line %d\n", lineno);
        exit(0);
    }

    return scan->token;
}

static enum TOKENDEF peekToken()
{
    if (scan->next == NULL) 
        return END;

    return scan->next->token->type;
}

static void errorMsg (const char *msg)
{
    printf("Error: %s at line %d\n", msg, lineno);
    exit(1);
}

static enum TOKENDEF getTokenValue()
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
            errorMsg("expecting operator");
            break;;
    }
        
    nextToken();
    return tok;
}

/*
 * Get string expression
 */
char *getStringExpression()
{
	if (scan->token->type == STRVAR) {
		char *s = svars[scan->token->data.no];
		nextToken();
		return s;
	} else if (scan->token->type == STRING) {
		char *s = scan->token->data.str;
		nextToken();
		return s;
	}
	
	errorMsg("string expression expected");
	return NULL;
}

/*
 * Get numerical expression
 */
static float getNumericExpr ()
{
    float result = getNumericValue();
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
        float val = getNumericValue();
        
        switch (tok) {
            case ADD:
                result += val; break;
            case SUB:
                result -= val; break;
            case MUL:
                result *= val; break;
            case DIV:
                result /= val; break;
            case CLOSEBRACKET:
            default:
                break;
        }
    }
    
    return result;
}

static float doRnd()
{
    if (scan->token->type != OPENBRACKET) {
        errorMsg("open bracket expected");
    }
    
    TOKEN *tok = nextToken();
    if (tok->data.no != 1)
        errorMsg("only 1 as parameter for RND implemented!");
    tok = nextToken();
    if (tok->type != CLOSEBRACKET)
        errorMsg("closing bracket expected after RND");

    float retVal = (double)rand() / (double)((unsigned)RAND_MAX + 1);

    return retVal;
}

static float getNumericValue()
{
    float retVal = 0;
    
    switch (scan->token->type) {
        case SUB:
            nextToken();
            retVal = getNumericValue() * -1; 
            break;
        case ADD:
            nextToken();
            retVal = getNumericValue(); 
            break;
        case NUMBER:
            retVal = scan->token->data.no; break;
        case FLOATNUM:
            retVal = scan->token->data.fno; break;
        case NUMVAR:
            retVal = vars[scan->token->data.no]; break;
        case OPENBRACKET:
            nextToken();
            retVal = getNumericExpr(); break;
        case RND:
            nextToken();
            retVal = doRnd(); break;
        case INT: {
            TOKEN *tok = nextToken();
            if (tok->type != OPENBRACKET)
                errorMsg("expecting opening bracket after INT");

            nextToken();
            retVal = getNumericExpr();
            break;;
        }
        default:
            errorMsg("invalid value or expression");
            break;;
    }

    nextToken();
    return retVal;
}

static void doPrint()
{
    int pos =1;         // screen pos 
    enum TOKENDEF lastTok = NONE;
    
    nextToken();
    while (scan->token->type != COLON && scan->token->type != LINENO) { 
        lastTok = scan->token->type;

        switch(scan->token->type) {
            case STRING:            
                pos += printf("%s", scan->token->data.str);
                nextToken();
                break;

            case COMMA:
                while (pos % 14) {
                    printf("%c", ' ');
                    ++pos;
                }
                nextToken();
                break;

            case SEMICOLON:
                nextToken();
                break;

            case TAB: {
                TOKEN *tok = nextToken();
                if (tok->type != OPENBRACKET)
                    errorMsg("expecting opening bracket after TAB");

                tok = nextToken();
                if (tok->type != NUMBER)
                    errorMsg("expecting number after TAB");

                int tabs = tok->data.no;

                tok = nextToken();
                if (tok->type != CLOSEBRACKET)
                    errorMsg("expecting closing bracket after TAB number");

                while (pos <= tabs) {
                    printf("%c", ' ');
                    ++pos;
                    }
                }

                nextToken();
                break;

            default: {
                float f = getNumericExpr();
                if (f == (int)f)
                    pos += printf("% .0f", f);
                else
                    pos += printf("% f", f);
                }
                break;
        }
    }

    if (lastTok != SEMICOLON)
        printf("\n");
}

static void doGoto(NODE *tokenlist)
{
    nextToken();
    int line = scan->token->data.no;    

    NODE *n = tokenlist;
    while (n) {
        if (n->token->type == LINENO && n->token->data.no == line) {
            scan = n;
            return;
        }
        n = n->next;
    }
    
    errorMsg("GOTO destination not found");
}

static void doIf(NODE *tokenlist)
{
    nextToken();

	if (scan->token->type == STRVAR) {
		char *expr1 = getStringExpression();

		enum TOKENDEF tok = scan->token->type;
		if (tok != EQUAL && tok != NOTEQUAL)
			errorMsg("operator expected");

		nextToken();
			
		char *expr2 = getStringExpression();
		if ((tok == EQUAL && !strcmp(expr1,expr2)) || (tok == NOTEQUAL && strcmp(expr1, expr2))) {
			if (peekToken() == NUMBER) {
				doGoto(tokenlist);
			} else
				nextToken();

			return;
		}
		
		errorMsg("invalid string expression ");		
	} else {
		bool evalWhole = false;
		enum TOKENDEF evalTok = NULL;

		for ( ; ; ) {
			int expr1 = getNumericExpr();    
			enum TOKENDEF tok = scan->token->type;
			bool eval = false;
			
			if (tok != GREATER && tok != SMALLER && tok != GREATEREQUAL && tok != SMALLEREQUAL && tok != EQUAL && tok != NOTEQUAL) {
				errorMsg("operator expected");
			}

			nextToken();
			int expr2 = getNumericExpr();    
			
			
			if ((tok == GREATER && expr1 > expr2) || (tok == SMALLER && expr1 < expr2) || (tok == EQUAL && expr1 == expr2)
					|| (tok == GREATEREQUAL && expr1 >= expr2) || (tok == SMALLEREQUAL && expr1 <= expr2) || (tok == NOTEQUAL && expr1 != expr2)) {
				eval = true;
			}
			
			
			
			if (scan->token->type == THEN || scan->token->type == GOTO) {
				if (evalTok == NULL) {
					evalWhole = eval;
				} else {
					if (evalTok == AND) {
						if 
						
				}	
				break;
			}
				
			if (scan->token->type == AND)
				evalTok = AND;
			else if (scan->token->type == OR)
				evalTok = OR;
			else
				errorMsg("AND or OR expected");
			
			nextToken();

				if (peekToken() == NUMBER) {
					doGoto(tokenlist);
				} else
					nextToken();

				return;
		}
			
	}
	
    while (scan && scan->token->type != LINENO)
        nextToken();
}

static void doInput()
{
    TOKEN *tok = nextToken();
    if (tok->type == STRING) {
        char *s = tok->data.str;
        if (nextToken()->type != SEMICOLON)
            errorMsg("semicolon expected after INPUT string");

        printf("%s? ", s);
		tok = nextToken();
    }

    if (tok->type != NUMVAR && tok->type != STRVAR)
        errorMsg("variable name expected after INPUT");

    while (tok->type == NUMVAR || tok->type == STRVAR) { 
        char *line = NULL;
        size_t len = 0;
        getline(&line, &len, stdin);
		if (strlen(line) > 0)
			line[strlen(line)-1] = '\0';
			
		if (tok->type == NUMVAR) {
			vars[tok->data.no] = atoi(line);
			free(line), line = NULL;
		} else
			svars[tok->data.no] = line;	

        tok = nextToken();
		if (tok->type != COMMA && tok->type != COLON && tok->type != LINENO)  
			errorMsg("next var/statement expected after INPUT string");
        
		if (tok->type == COMMA || tok->type == COLON)  
			tok = nextToken();
    }
}

void run(NODE *tokenlist)
{
    int   var;
    scan = tokenlist;
    lineno = 0;

    if (!scan)
        return;
        
    memset(vars, 0, sizeof(vars));
    for (int i=0; i < 26; ++i)
        svars[i] = "";

    do {
        switch (scan->token->type) {
            case LINENO: 
                /*if (lineno >= scan->token->data.no) {
                   errorMsg("line number not increasing");
                }
                */

                lineno = scan->token->data.no;
                nextToken();
                break;
            
            case COLON:
                nextToken();
                break;

            case LET:
                nextToken();
                if (scan->token->type != NUMVAR && scan->token->type != STRVAR) {
                    errorMsg("variable expected");
                }
            case STRVAR:
            case NUMVAR:
                var = scan->token->data.no;
                
                nextToken();
                if (scan->token->type != EQUAL) {
                    errorMsg("assignment expected");
                }
                
                nextToken();
                if (scan->token->type == STRING)
					svars[var] = strdup(scan->token->data.str);
				else 
					vars[var] = getNumericExpr ();
                break;

            case PRINT:
                doPrint();
                break;

            case IF:
                doIf(tokenlist);
                break;
                
            case GOTO:
                doGoto(tokenlist);
                break;
            
            case INPUT:
                doInput();
                break;

            case END:
                exit(0);
                
            default: 
                errorMsg("Unexpected character/command");
                break;;
        }
    } while (scan);
}

