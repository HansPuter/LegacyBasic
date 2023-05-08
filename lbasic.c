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
#include "tokenizer.h"
#include "run.h"

void dumplist(NODE *root);

int main (int argc, char *argv[])
{
    if (argc <= 1)
    {
        printf("usage: lbasic basic-file\n");
        return 1;
    }
    
    FILE *fin = fopen(argv[1], "r");
    if (NULL == fin)
    {
        printf("error: couldn't open %s\n", argv[1]);
        return 2;
    }
     
    NODE *tokenlist = NULL;
    int   lineno    = 0;
    char line[1024];
    while(fgets(line, sizeof(line)-1, fin))
    {
        printf("%s", line);

        ++lineno;
        if (!tokenize(&tokenlist, line, lineno))
        {
            //fclose(fin), fin=NULL;
            break;
        }
    }

    dumplist(tokenlist);
    fclose(fin), fin=NULL;

    srand(time(0));
    run(tokenlist);
    
    freetokens (&tokenlist);
    return 0;
}


void dumplist(NODE *root)
{
    NODE *scan = root;
    if (!scan)
        return;
        
    do
    {
        switch (scan->token->type)
        {
            case ADD: printf("+ "); break;
            case SUB: printf("- "); break;
            case MUL: printf("* "); break;
            case DIV: printf("/ "); break; 
            case GREATER: printf("> "); break;
            case GREATEREQUAL: printf(">= "); break;
            case SMALLER: printf("< "); break;
            case SMALLEREQUAL: printf("<= "); break;
            case EQUAL: printf("= "); break; 
            case NOTEQUAL: printf("<> "); break;
            case OPENBRACKET: printf("( "); break;
            case CLOSEBRACKET: printf(") "); break;
            case COMMA: printf(", "); break;
            case COLON: printf(": "); break;
            case SEMICOLON: printf("; "); break;
            case LINENO: printf("\nLNO(%d) ", scan->token->data.no); break;
            case NUMBER: printf("NUM(%d) ", scan->token->data.no); break;
            case FLOATNUM: printf("FNO(%f) ", scan->token->data.fno); break;
            case STRING: printf("STR(\"%s\") ", scan->token->data.str); break;
            case NUMVAR: printf("NUMVAR(%c) ", scan->token->data.no + 'A'); break;
            case STRVAR: printf("STRVAR(%c$) ", scan->token->data.no + 'A'); break;
            case PRINT: printf("PRINT "); break;
            case IF: printf("IF "); break;
            case THEN: printf("THEN "); break;
            case GOTO: printf("GOTO "); break;
            case INPUT: printf("INPUT "); break;
            case LET: printf("LET "); break;
            case GOSUB: printf("GOSUB "); break;
            case RETURN: printf("RETURN "); break;
            case CLEAR: printf("CLEAR "); break;
            case END: printf("END "); break;
            case REM: printf("REM "); break;
            case RND: printf("RND "); break;
            case TAB: printf("TAB"); break;
            case INT: printf("INT"); break;
        
            default: printf("Unknown: %d ", scan->token->type); break;
        }
        
        scan = scan->next;
    } while (scan);
    
    printf("\n\n");
}
