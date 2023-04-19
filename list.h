/* list.h
 * (c) 2023 by Hans-Peter Rampp - LGPL 2
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

void addtoken (NODE **root, TOKEN *token);
void freetokens (NODE **root);
