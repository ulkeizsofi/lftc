#include <stdio.h>
#include <string.h>
#include <errno.h>

#define NAME_MAX 100
#define BUFF_SIZE_MAX 50001

typedef enum code {ID, CT_INT, CT_REAL, CT_CHAR, 
	CT_STRING, COMMA, SEMICOLON, LPAR, 
	RPAR, LBRACKET, RBRACKET, LACC, 
	RACC, ADD, SUB, MUL, DIV, DOT, AND,
	OR, NOT, ASSIGN, EQUAL, NOTEQ, LESS,
	LESSEQ, GREATER, GREATEREQ} codd;

typedef struct _Token{
	enum code cod;
	union{
		char* text;
		int i;
		double r;
	};
	int line_nr;
	struct _Token *next;
}Token;

void readFromFile(char* c_file, char* buffer);
void addToken(struct _Token **first, struct _Token* toAdd);
int createToken(struct _Token* token, enum code cod, char* text, int* i, double* r, int line);
void printToken(struct _Token* token);
void printTokenList(struct _Token *token);

int main(){
	char input[BUFF_SIZE_MAX];
	readFromFile("first.c", input);
	printf("%s\n", input);
	struct _Token *tk;

	struct _Token token, token2;
	createToken(&token, 0, "text", NULL, NULL, 5);
	createToken(&token2, 0, "text2", NULL, NULL, 6);
	addToken(&tk, &token);
	addToken(&tk, &token2);
	//printf("%s  %s\n", token.text, token2.text);
	printTokenList(tk);

}

void readFromFile(char* c_file, char* buffer){
	FILE *file;
	file = fopen(c_file, "r");
	if ( file == NULL){
		perror("Cannot open the file to read\n");
	}
	int nc = fread(buffer, 1, BUFF_SIZE_MAX - 1, file);
	buffer[nc] = '\0';
	fclose(file);
}

// TODO: just a pointerto token instead of the whole struct
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
	printToken(toAdd);
	printf("%p\n", toAdd);
	printf("%p\n", token->next);
	printf("Added\n");
	printTokenList(*first);
	printf("------------------\n");
	return;
}

// TODO: this should return pointer. create inside with malloc or pass pointer to struct
// TODO: should check input type and use only that input (no need to pass as pointer)
int createToken(struct _Token* token, enum code cod, char* text, int* i, double* r, int line){
	token->cod = cod;
	if (text != NULL){
		token->text = text;
	}
	else{
		if (i != NULL){
			token->i = *i;
		}
		else{
			token->r = *r;
		}
	}
	//TODO: check the parameterlist to make sure the union is ok
	token->line_nr = line;
	token->next = NULL;
	return 0;
}

void printToken(struct _Token* token){
	printf("%d\t",token->cod);
	switch(token->cod){
		case ID: CT_STRING: printf("%s\t",token->text);
		break;
		case CT_INT: CT_CHAR: printf("%d\t",token->i);
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