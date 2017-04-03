#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include "token.h"

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
	switch (cod){
	case ID: case CT_STRING: token->text = text;;
		break;
	case CT_INT: token->i = atoi(text);
		break;
	case CT_CHAR: token->i = text[0];
		//TODO: add the appropriate char
		break;
	case CT_REAL: token->r = strtod(text, NULL);
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
	printf("%s\t",names[token->cod]);
	switch(token->cod){
		case ID: case CT_STRING: printf("%s\t",token->text);
		break;
		case CT_INT: case CT_CHAR: printf("%d\t",token->i);
		break;
		case CT_REAL: printf("%f\t",token->r);
		break;
	}
	printf("%d\n", token->line_nr);
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
	Token *token = createToken(cod, str, NULL, NULL, line);
	addToken(tk, token);
}
