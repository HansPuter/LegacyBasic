/* list.h
 */
 
typedef struct
{
    enum TOKENDEF type;
    union
    {
        int  no;
        char *str;
    } data;
} TOKEN;
 
typedef struct node
{
    TOKEN *token; 
    struct node *next;
} NODE;

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
            case SMALLER: printf("< "); break;
            case EQUALS: printf("= "); break; 
            case OPENBRACKET: printf("( "); break;
            case CLOSEBRACKET: printf(") "); break;
            case COMMA: printf(", "); break;
            case COLON: printf(": "); break;
            case LINENO: printf("\nLNO(%d) ", scan->token->data.no); break;
            case NUMBER: printf("INT(%d) ", scan->token->data.no); break;
            case STRING: printf("STR(%s) ", scan->token->data.str); break;
            case VARIABLE: printf("VAR(%c) ", scan->token->data.no + 'A'); break;
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
        
            default: printf("Unknown: %d ", scan->token->type); break;
        }
        
        scan = scan->next;
    } while (scan);
    
    printf("\n\n");
}
