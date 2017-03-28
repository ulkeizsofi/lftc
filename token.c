#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include "token.h"

char* names[] = {"ID","CT_INT","CT_REAL","CT_CHAR",
"CT_STRING","COMMA","SEMICOLON","LPAR",
"RPAR","LBRACKET","RBRACKET","LACC",
"RACC","ADD","INC","SUB",
"DEC","MUL","DIV","DOT",
"AND","BAND","OR","BOR",
"NOT","ASSIGN","EQUAL","NOTEQ",
"LESS","LESSEQ","GREATER","GREATEREQ",
"END", "BREAK", "CHAR", "DOUBLE",
"ELSE", "FOR", "IF", "INT",
"RETURN", "STRUCT", "VOID", "WHILE",
"INCLUDE"};

char* ids[] = {"break", "char", "double",
		"else", "for", "if", "int",
		"return", "struct", "void", "while", "include"};


void addToken(struct _Token** first, struct _Token* toAdd){
	struct _Token *token = *first;
	if (*first == NULL){
		*first = toAdd;
		(*first)->next =NULL;
		return;
	}
	while (token->next != NULL){
		token = token->next;
	}
	token->next = toAdd;
	toAdd->next=NULL;
	return;
}

Token* createToken(enum code cod, char* text, int* i, double* r, int line){
	Token *token = (Token*)malloc(sizeof(Token));
	token->cod = cod;
	char* help_text;
	if (text != NULL)
		help_text = (char*)malloc((strlen(text) + 1)* sizeof(char));
	char help_char;
	int help_i = 0;
	switch (cod){
	case ID: token->text = text;
		break;
	case CT_STRING: 
		for (int i = 1; i < strlen(text) - 1; i++){
			if (text[i] == '\\'){
				switch (text[i + 1]){
				case 'n': help_text[help_i] = '\n';
					break;
				case 't': help_text[help_i] = '\t';
					break;
				case 'r': help_text[help_i] = '\r';
					break;
				case 'a': help_text[help_i] = '\a';
					break;
				case 'b': help_text[help_i] = '\b';
					break;
				case 'v': help_text[help_i] = '\v';
					break;
				case '\'':help_text[help_i] = '\'';
					break;
				case '\"':help_text[help_i] = '\"';
					break;
				case '?':help_text[help_i] = '\?';
					break;
				case '\\':help_text[help_i] = '\\';
					break;
				case '0':help_text[help_i] = '\0';
					break;
				}
				i++;
			}
			else{
				help_text[help_i] = text[i];
			}
			help_i++;
		}
		help_text[help_i] = '\0';
		token->text = help_text;
		break;
	case CT_INT: 
		help_i = 10;
		if (text[0] == '0'){
			if( text[1] == 'x'){
				help_i = 16;
			}
			else{
				if(text[1] == '.'){
					help_i = 10;
				}
				else{
					help_i = 8;
				}
			}
		}
		token->i = (int)strtol(text, NULL, help_i);
		break;
	case CT_CHAR: token->i = text[1];
		if (text[1] == '\\'){
			switch (text[2]){
			case 'n': help_char = '\n';
				break;
			case 't': help_char = '\t';
				break;
			case 'r': help_char = '\r';
				break;
			case 'a': help_char = '\a';
				break;
			case 'b': help_char = '\b';
				break;
			case 'v': help_char = '\v';
				break;
			case '\'':help_char = '\'';
				break;
			case '\"':help_char = '\"';
				break;
			case '?':help_char = '\?';
				break;
			case '\\':help_char = '\\';
				break;
			case '0':help_char = '\0';
				break;
			default:
				help_char = 'i';
				break;
			}
		}
		else{
			help_char = text[1];
		}
		break;
	case CT_REAL: token->r = strtod(text, NULL);
		break;
	default:
		break;
	}
	/*if (text != NULL){

		token->text = text;
	}
	else{
		if (i != NULL){
			token->i = *i;
		}
		else{
			token->r = *r;
		}
	}*/
	//TODO: check the parameterlist to make sure the union is ok
	token->line_nr = line;
	token->next = NULL;

	return token;
}

void printToken(struct _Token* token){
	printf("%10s",names[token->cod]);
	switch(token->cod){
		case ID: case CT_STRING: printf(": %5s\t",token->text);
		break;
		case CT_INT: printf(": %5d\t", token->i);
			break;
		case CT_CHAR: printf(": %5c\t",token->i);
		break;
		case CT_REAL: printf(": %5f\t",token->r);
		break;
		default: printf("\t\t");
			break;
	}
	printf(" line: %d\n", token->line_nr);
}

void printTokenList(struct _Token *token){
	if (token == NULL){
		printf("The list is empty\n");
		return;
	}
	struct _Token *tk = token;
	while (tk != NULL){
		printToken(tk);
		tk = tk->next;
	}
}

// int addIntToken(char* ch, int length){
// 	char *str = (char*)malloc(sizeof(char)*(length + 1));
// 	memcpy(str, ch, length);
// 	str[length] = 0;
	
// 	int nr = atoi(str);
// 	Token *token = createToken(CT_INT, NULL, &nr, NULL, 1);
// 	addToken(&tk, token);
// }


// int addStringToken(char* ch, int length){
// 	char *str = (char*)malloc(sizeof(char)*(length + 1));
// 	memcpy(str, ch, length);
// 	str[length] = 0;
// 	Token *token = createToken(CT_STRING, str, NULL, NULL, 2);
// 	addToken(&tk, token);
// }

int addGenericToken(Token** tk, char* ch, int length, enum code cod, int line){
	char *str = (char*)malloc(sizeof(char)*(length + 1));
	memcpy(str, ch, length);
	str[length] = 0;

	if (cod == ID){
		for (int i = 0; i < sizeof(ids)/sizeof(ids[0]); i++){
			if (!strcmp(ids[i], str)){
				cod = C_END + i + 1;
				//TODO: make it less hardcoded
		}
		}
	}
	Token *token = createToken(cod, str, NULL, NULL, line);
	addToken(tk, token);
	return 0;
}
