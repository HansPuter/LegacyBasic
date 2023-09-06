/*
 * Tokenize source lines
 * (c) 2023 by Hans-Peter Rampp - LGPL 2
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "token.h"
#include "list.h"
#include "tokenizer.h"

int tokenize (NODE **tokenlist, char *line, int lineno)
{
    char *s = line;

    TOKEN *t = getNextToken(&s, lineno);
    if (!t)
        return 0;
        
    if (t->type == NUMBER)
        t->type = LINENO;
    else {
        printf("Error: line doesn't start with a number!");
        exit(1);
    }
    addtoken (tokenlist, t);
    
    while ((t = getNextToken(&s, lineno)))
    {
        if (t->type == REM)
            break;

        addtoken (tokenlist, t);
    }
    
    return 1;
}

const char *delimiters = "+-*/><=(),:;";

/*
 * Scan for next token
 */
TOKEN *getNextToken (char **pos, int lineno)
{
    char *s = *pos;
    while (isspace(*s))
        ++s;
    
    if (*s) {
        // NUMBER or FLOAT
        if (isdigit(*s) /* || (*s == '-' && isdigit(*(s+1))) || (*s == '+' && isdigit(*(s+1))) */ ) {
            TOKEN *token = NULL;
            int minus = 1;
            if (*s == '-') {
                minus = -1;
                ++s;
            }
            
            if (*s == '+') {
                ++s;
            }

            int no = *s - '0';
            char *t = s + 1;
            while (isdigit(*t)) {
                no *= 10;
                no += *t - '0';
                ++t;
            }
            
            if (*t == '.' || toupper(*t) == 'E') {
                float fno = (float)no;
                if (*t == '.') {
                    t++;
                    no = 10;
                    while (isdigit(*t)) {
                        fno += (float)(*t - '0')/(float)no;
                        no *= 10;
                        ++t;
                    }
                }

                if (toupper(*t) == 'E') {
                    int signExp = 0;
                    ++t;
                    if (*t == '-') {
                        signExp = -1;
                        ++t;
                    }
                    
                    if (*t == '+') {
                        ++t;
                    }

                    if (!isdigit(*t)) {
                        printf("error: invalid number in line %d\n", lineno);
                        exit(1);
                    }

                    no = 10;
                    int exp = 0;
                    while (isdigit(*t)) {
                        exp *= 10;
                        exp += *t - '0';
                        ++t;
                    }
                    no = pow(10, exp);
                    if (signExp < 0) {
                        fno /= (float)no;
                    } else {
                        fno *= (float)no;
                    }

                }
                        
                token = (TOKEN*)malloc(sizeof(TOKEN));
                token->type = FLOATNUM;
                if (minus < 0)
                    fno = -fno;
                token->data.fno = fno;
                
            } else {
                if (isalpha(*t)) {
                    printf("error: invalid number in line %d\n", lineno);
                    exit(1);
                }
                
                token = (TOKEN*)malloc(sizeof(TOKEN));
                token->type = NUMBER;
                token->data.no = no * minus;
            }

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
                    if (*(s+1) == '=') {
                        token->type = SMALLEREQUAL;
                        *pos = ++s; 
                    } else if (*(s+1) == '>') {
                        token->type = NOTEQUAL;
                        *pos = ++s;     
                    } else
                        token->type = SMALLER; 
                    
                    break;
                case '>':
                    if (*(s+1) == '=') {
                        token->type = GREATEREQUAL;
                        *pos = ++s;    
                    } else
                        token->type = GREATER; 
                    break;
                case '=':
                    token->type = EQUAL; break;
                case '(':
                    token->type = OPENBRACKET; break;
                case ')':
                    token->type = CLOSEBRACKET; break;
                case ',':
                    token->type = COMMA; break;
                case ':':
                    token->type = COLON; break;
                case ';':
                    token->type = SEMICOLON; break;
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
				if (*t == '$') {
					token->type = STRVAR;
					t++;
				}	
				else 
					token->type = NUMVAR;
					
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
            else if (!strcmp(keyword, "TAB"))
                token->type = TAB;
            else if (!strcmp(keyword, "INPUT")) 
                token->type = INPUT;
            else if (!strcmp(keyword, "LET")) 
                token->type = LET;
            //else if (!strcmp(keyword, "GOSUB")) 
            //    token->type = GOSUB;
            //else if (!strcmp(keyword, "RETURN")) 
            //    token->type = RETURN;
            //else if (!strcmp(keyword, "CLEAR")) 
            //    token->type = CLEAR;
            else if (!strcmp(keyword, "END")) 
                token->type = END;
            else if (!strcmp(keyword, "REM")) 
                token->type = REM;
            else if (!strcmp(keyword, "RND")) {
                token->type = RND;
                token->data.no = NUMFUNC;
            }
            else if (!strcmp(keyword, "INT"))
                token->type = INT;
            else if (!strcmp(keyword, "AND"))
                token->type = AND;
            else if (!strcmp(keyword, "OR"))
                token->type = OR;
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
