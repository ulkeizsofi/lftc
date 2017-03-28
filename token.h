#ifndef TOKEN
#define TOKEN

typedef enum code {ID, CT_INT, CT_REAL, CT_CHAR, 
	CT_STRING, COMMA, SEMICOLON, LPAR, 
	RPAR, LBRACKET, RBRACKET, LACC, 
	RACC, ADD, INC, SUB, 
	DEC, MUL, DIV, DOT, 
	AND, BAND, OR, BOR, 
	NOT, ASSIGN, EQUAL, NOTEQ, 
	LESS, LESSEQ, GREATER, GREATEREQ, 
	C_END, BREAK, CHAR, DOUBLE,
	ELSE, FOR, IF, INT,
	RETURN, STRUCT, VOID, WHILE, INCLUDE} codd;


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


void addToken(struct _Token **first, struct _Token* toAdd);
Token* createToken(enum code cod, char* text, int* i, double* r, int line);
void printToken(struct _Token* token);
void printTokenList(struct _Token *token);
int addGenericToken(Token** tk, char* ch, int length, enum code cod, int line);

#endif
