/* list.h
 * (c) 2023 by Hans-Peter Rampp - LGPL 2
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "token.h"
#include "list.h"

void addtoken (NODE **root, TOKEN *token)
{
    NODE *scan = NULL;
    NODE *n = (NODE*)malloc(sizeof(NODE));
    n->token = token;
    n->next  = NULL;
    
    if (*root)
    {
        scan = *root;
        while (scan->next != NULL)
        {
            scan = scan->next;
        }
    
        scan->next = n;
    }
    else
        *root = n;
}

void freetokens (NODE **root)
{
    NODE *scan = *root;
    if (!scan)
        return;
        
    do
    {
        if (scan->token->type == STRING)
            free (scan->token->data.str);
        free (scan->token);
        
        NODE *n = scan;
        scan = scan->next;
        free (n);
        
    } while (scan);
    
    *root =  NULL;
}

