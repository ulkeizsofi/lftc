#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include "token.h"
#include "ASIN.h"

#define START_OFFSET 0
#define STATE_FILE "state_descriptor.txt"
#define MAX_STATES 66
#define MAX_NO_CHAR 255

#define NAME_MAX 100
#define BUFF_SIZE_MAX 50001
#define ERROR_ELSE_VALUE_FILE -1
#define ERROR_STATE_ID MAX_STATES
#define INITIAL_STATE_ID 0
#define INPUT_STREAM "first.c"

typedef int (*stateHandler)(Token** tk, char* ch, int length, enum code cod,
		int line);
typedef int stateID;

typedef struct _State {
	stateID id;
	stateID next[MAX_NO_CHAR + 5];
	stateID elseID;
	int cod;
	stateHandler handler;
} State;

//head of Token List
Token *tk;

int readFromFile(FILE* file, char* buffer);
FILE *openFile(char* file_name);
int createInitialStateFile();
int createStates(State *stt);
int initState(State *stt, stateID defaultNext);
void printState(State* stt);

int main(int argc, char** argv) {
	//Declarations
	char input[BUFF_SIZE_MAX]; //input buffer
	int line_nr = 0; //the current line number
	int count = 0;	//the current index in the input buffer
	State states[MAX_STATES + 1]; //all states
	State *currentState;
	int tokenStart = 0; //the origin token
	int nRead; //the number of char already read

	if (argc != 2) {
		printf("Usage: 1 param->file name\n");
		return -1;
	}
	//Create states from file
	createStates(states);

	//Set the head of the token list to NULL
	tk = NULL;
	//Set the current state
	currentState = &states[INITIAL_STATE_ID];
	FILE* file = openFile(argv[1]);

	while ((nRead = readFromFile(file, input)) != 0) {
		//printf("%s\n", input);
		while (count != nRead) {
			if (currentState->handler) {

				currentState->handler(&tk, &input[tokenStart],
						count - tokenStart, currentState->cod, line_nr);
			}

			stateID nextState = currentState->next[input[count] - START_OFFSET];
			if (nextState == ERROR_STATE_ID) {
				nextState = currentState->elseID;
			} else {

				if (input[count] == '\n') {
					//printf("%x %d\n", input[count], line_nr);
					line_nr++;
				}
				count++;
			}
			if (nextState == INITIAL_STATE_ID) {
				tokenStart = count;
			}
			currentState = &states[nextState];
		}
	}
	//Add END Token
	Token *token_end = createToken(C_END, NULL, NULL, NULL, line_nr);
	addToken(&tk, token_end);
	printTokenList(tk);
	printf("%d\n",isUnit(&tk) );

	int ln;
	char* err = checkError(&ln);
	if (err != NULL){
	printf("%s\n", err);
	printf("at line: %d\n", ln);
	}
	else printf("No error\n");
	return 0;
}

FILE *openFile(char* file_name) {
	FILE* file = fopen(file_name, "r");
	if (file == NULL) {
		perror("Cannot open the file to read\n");
	}
	return file;
}

int readFromFile(FILE *file, char* buffer) {
	int nc = fread(buffer, 1, BUFF_SIZE_MAX - 1, file);
	return nc;
}

int initState(State *stt, stateID defaultNext) {
	stt->id = -1;
	stt->handler = NULL;
	stt->elseID = ERROR_STATE_ID;
	for (int i = 0; i < MAX_NO_CHAR; i++) {
		stt->next[i] = defaultNext;
	}
	return 0;
}
int createInitialStateFile() {
	FILE *file;
	file = fopen(STATE_FILE, "w");
	if (file == NULL) {
		perror("Cannot open the file to write\n");
		return -1;
	}

	for (int i = 0; i < MAX_STATES; i++) {
		fprintf(file, "%d\n", i); //id
		fprintf(file, "%d\n", -1); //generate
		for (int j = 0; j < MAX_NO_CHAR - START_OFFSET; j++) {

			fprintf(file, "%c ", j);
		} //vector
		fprintf(file, "\n");
		fprintf(file, "%d\n", ERROR_STATE_ID);
	}
	fclose(file);
	return 0;
}

int createStates(State *stt) {

	FILE *file;
	file = fopen(STATE_FILE, "r");
	if (file == NULL) {
		perror("Cannot open the file to read\n");
	}

	State* state = &stt[0];
	int id, gen, next;
	char ch, start_ch, end_ch;
	while (!feof(file)) {
		fscanf(file, "%d\n", &id);
		state = &stt[id];
		//printf("%d\n", id);
		initState(state, ERROR_STATE_ID);

		state->id = id;
		fscanf(file, "%d\n", &gen);
		state->cod = gen;

		if (gen >= 0) {
			state->handler = addGenericToken;
		}

		do {
			fscanf(file, "%c-%c %d%c", &start_ch, &end_ch, &next, &ch);

			if (start_ch == '^') {
				for (int i = 0; i <= MAX_NO_CHAR; i++) {
					if (i != end_ch)
						state->next[i] = next;
				}
			}
			if (start_ch != 'i' || end_ch != 'i') {
				for (int i = start_ch; i <= end_ch; i++) {
					state->next[i] = next;
				}
			}
		} while (ch != '\n');
		fscanf(file, "%d\n", &(state->elseID));
		if (state->elseID == ERROR_ELSE_VALUE_FILE) {
			state->elseID = ERROR_STATE_ID;
		}

	}
	//Set some of them manually
	stt[0].next['\n'] = 0;
	stt[0].next['\r'] = 0;
	stt[55].next['\\'] = 56;
	stt[60].next['\\'] = 61;
	return 0;
}

void printState(State* stt) {
	printf("id: %d\thandler:%c\t", stt->id, stt->handler == NULL ? 'n' : 'y');
	for (int i = 0; i < MAX_NO_CHAR - START_OFFSET; i++) {
		if (stt->next[i] != ERROR_STATE_ID)
			printf("%c: %d\t", i, stt->next[i]);
	}
	printf("else: %d\n", stt->elseID);
}
