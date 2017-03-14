#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#define START_OFFSET 0
#define STATE_FILE "state_descriptor.txt"
#define MAX_STATES 66
#define MAX_NO_CHAR 255

#define NAME_MAX 100
#define BUFF_SIZE_MAX 50001
#define ERROR_STATE_ID MAX_STATES
#define INITIAL_STATE_ID 0

typedef enum code {ID, CT_INT, CT_REAL, CT_CHAR, 
	CT_STRING, COMMA, SEMICOLON, LPAR, 
	RPAR, LBRACKET, RBRACKET, LACC, 
	RACC, ADD, INC, SUB, 
	DEC, MUL, DIV, DOT, 
	AND, BAND, OR, BOR, 
	NOT, ASSIGN, EQUAL, NOTEQ, 
	LESS, LESSEQ, GREATER, GREATEREQ} codd;

char* names[] = {"ID","CT_INT","CT_REAL","CT_CHAR",
"CT_STRING","COMMA","SEMICOLON","LPAR",
"RPAR","LBRACKET","RBRACKET","LACC",
"RACC","ADD","INC","SUB",
"DEC","MUL","DIV","DOT",
"AND","BAND","OR","BOR",
"NOT","ASSIGN","EQUAL","NOTEQ",
"LESS","LESSEQ","GREATER","GREATEREQ"};

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

typedef int (*stateHandler)(char* ch, int length, enum code cod, int line);
typedef int stateID;


typedef struct _State{
	stateID id;
	stateID next[MAX_NO_CHAR + 5];
	stateID elseID;
	int cod;
	stateHandler handler;
}State;

//ead of Token List
Token *tk;


void readFromFile(char* c_file, char* buffer);
void addToken(struct _Token **first, struct _Token* toAdd);
Token* createToken(enum code cod, char* text, int* i, double* r, int line);
void printToken(struct _Token* token);
void printTokenList(struct _Token *token);
int createInitialStateFile();
int createStates(State *stt);
int initState(State *stt, stateID defaultNext);
int addIntToken(char* ch, int length);
int addStringToken(char* ch, int length);
int addGenericToken(char* ch, int length, enum code cod, int line);
void printState(State* stt);


int main(){
	char input[BUFF_SIZE_MAX];
	readFromFile("first.c", input);
	int line_nr = 0;
	printf("%s\n", input);
	tk = NULL;
	int count = 0;
	State states[MAX_STATES + 1];
	createStates(states);
	State *currentState;
	currentState = &states[INITIAL_STATE_ID];
	int tokenStart = 0;
	while( input[count] != '\0'){
		if (input[count] == '\n'){
			printf("%c %d\n", input[count], line_nr);
			line_nr++;
		}
		if (currentState->handler){
		
			currentState->handler(&input[tokenStart], count - tokenStart, 
				currentState->cod, line_nr);
		}
		
		stateID nextState = currentState->next[input[count] - START_OFFSET];
		if (nextState == ERROR_STATE_ID){
				nextState = currentState->elseID;
		}
		else{
			count++;
		}
		if (nextState == INITIAL_STATE_ID){
			tokenStart = count;
		}
		currentState = &states[nextState];
	}
	printTokenList(tk);
	return 0;
}

void readFromFile(char* c_file, char* buffer){
	FILE *file;
	file = fopen(c_file, "r");
	if ( file == NULL){
		perror("Cannot open the file to read\n");
	}
	//TODO: while
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
	return;
}

// TODO: this should return pointer. create inside with malloc or pass pointer to struct
// TODO: should check input type and use only that input (no need to pass as pointer)
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

int initState(State *stt, stateID defaultNext){
	stt->id = -1;
	stt->handler = NULL;
	stt->elseID = ERROR_STATE_ID;
	for(int i = 0; i < MAX_NO_CHAR; i++){
		stt->next[i] = defaultNext;
	}
	return 0;
}
int createInitialStateFile(){
	FILE *file;
	file = fopen(STATE_FILE, "w");
	if (file == NULL){
		perror("Cannot open the file to write\n");
	}

	for (int i = 0; i < MAX_STATES; i++){
		fprintf(file, "%d\n", i);//id
		fprintf(file, "%d\n", -1);//generate
		for (int j = 0; j < MAX_NO_CHAR - START_OFFSET; j++){
		
			fprintf(file, "%c ", j);
		}//vector
		fprintf(file, "\n");
		fprintf(file, "%d\n", ERROR_STATE_ID);
	}
	fclose(file);
}

int createStates(State *stt){
	
	FILE *file;
	file = fopen(STATE_FILE, "r");
	if (file == NULL){
		perror("Cannot open the file to read\n");
	}

	State* state = &stt[0];
	int id, gen, next;
	char ch, start_ch, end_ch;
	while (!feof(file)){
		fscanf(file, "%d\n", &id);
		state = &stt[id];
		initState(state, ERROR_STATE_ID);
		
		state->id = id;
		fscanf(file, "%d\n", &gen);
		state->cod = gen;
		
		if (gen >= 0){
			state->handler = addGenericToken;
		}
		
		do
		{
			fscanf(file, "%c-%c %d%c", &start_ch, &end_ch, &next, &ch);

			if (start_ch == '^'){
				for (int i = 0; i <= MAX_NO_CHAR; i++){
					if (i != end_ch ) state->next[i] = next;
				}
			}
			if (start_ch != 'i' || end_ch != 'i'){
				for (int i = start_ch; i <= end_ch; i++){
					state->next[i] = next;
				}
			}
		} while (ch != '\n'); 
		fscanf(file, "%d\n", &(state->elseID));

	}
	return 0;

	//State* state = &stt[0];
	//initState(state, 0);
	//state->id = 0;//May be duplicate information

	//for(char i = '0'; i <= '9'; i++ ){
	//	state->next[i] = 1;
	//}
	//for(char i = 'a'; i <= 'z'; i++ ){
	//	state->next[i] = 2;
	//}

	//state = &stt[1];
	//initState(state, ERROR_STATE_ID);
	//state->id = 1;//May be duplicate information
	//state->elseID = 4;
	//for(char i = '0'; i <= '9'; i++ ){
	//	state->next[i] = 1;
	//}

	//state = &stt[2];
	//initState(state, ERROR_STATE_ID);
	//state->id = 2;//May be duplicate information
	//state->elseID = 5;
	//for(char i = 'a' - START_OFFSET; i <= 'z'; i++ ){
	//	state->next[i] = 2;
	//}

	//state = &stt[4];
	//initState(state, ERROR_STATE_ID);
	//state->id = 4;//May be duplicate information
	//state->elseID = INITIAL_STATE_ID;
	//state->handler = addIntToken;

	//state = &stt[5];
	//initState(state, ERROR_STATE_ID);
	//state->id = 5;//May be duplicate information
	//state->elseID = INITIAL_STATE_ID;
	//state->handler = addStringToken;	
}

int addIntToken(char* ch, int length){
	char *str = (char*)malloc(sizeof(char)*(length + 1));
	memcpy(str, ch, length);
	str[length] = 0;
	
	int nr = atoi(str);
	Token *token = createToken(CT_INT, NULL, &nr, NULL, 1);
	addToken(&tk, token);
}


int addStringToken(char* ch, int length){
	char *str = (char*)malloc(sizeof(char)*(length + 1));
	memcpy(str, ch, length);
	str[length] = 0;
	Token *token = createToken(CT_STRING, str, NULL, NULL, 2);
	addToken(&tk, token);
}

int addGenericToken(char* ch, int length, enum code cod, int line){
	char *str = (char*)malloc(sizeof(char)*(length + 1));
	memcpy(str, ch, length);
	str[length] = 0;
	Token *token = createToken(cod, str, NULL, NULL, line);
	addToken(&tk, token);
}

void printState(State* stt){
	printf("id: %d\thandler:%c\t", stt->id, stt->handler == NULL ? 'n' : 'y');
	for (int i = 0; i < MAX_NO_CHAR - START_OFFSET; i++){
		if (stt->next[i] != ERROR_STATE_ID) printf("%c: %d\t", i, stt->next[i]);
	}
	printf("else: %d\n", stt->elseID);
}