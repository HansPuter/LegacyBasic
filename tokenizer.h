/*
 * Header for tokenizer
 * (c) 2023 by Hans-Peter Rampp - LGPL 2
 */

int     tokenize (NODE *tokenlist, char *line, int lineno);
TOKEN  *getNextToken (char **pos, int lineno);
