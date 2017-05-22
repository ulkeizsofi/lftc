/*
 * symbol.c

 *
 *  Created on: Apr 25, 2017
 *      Author: zsofi
 */

#include "symbol.h"
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include "token.h"
#include <stdio.h>

void initSymbols(Symbols** symbols) {
	if (symbols == NULL) {
		symbols = (Symbols**) realloc(symbols, sizeof(Symbols*));
	}

	*symbols = (Symbols*) realloc(*symbols, sizeof(Symbols));

	(*symbols)->after = NULL;
	(*symbols)->begin = NULL;
	(*symbols)->end = NULL;
}

void initSymbol(Symbol** sym) {
	*sym = (Symbol*) malloc(sizeof(Symbol));
	if (*sym == NULL) {
		err(-1, "not enough memory");
	}
	(*sym)->fields = NULL;
}

Symbol *addSymbol(Symbols **symbols, const char *nm, int cls, int depth) {
	int count, n = 0;
	Symbol* sym = NULL;
	if (*symbols == NULL) {
		initSymbols(symbols);
	}
	if ((**symbols).end == (**symbols).after) {
		count = (**symbols).end - (**symbols).begin;
		if (count == 0) {
			n = INITIAL_SYMBOLS_SIZE;
		} else {
			n = count * 2;
		}
		(**symbols).begin = (Symbol**) realloc((*symbols)->begin,
				n * sizeof(Symbol*));
		if ((*symbols)->begin == NULL)
			err(-1, "not enough memory");
		(*symbols)->end = (*symbols)->begin + count;
		(*symbols)->after = (*symbols)->begin + n;
	}
	initSymbol(&sym);
	sym->cls = cls;
	sym->name = (char*) malloc(sizeof(char) * (strlen(nm) + 1));
	strncpy(sym->name, nm, strlen(nm));
//	strcat(sym->name,'\0');
	sym->depth = depth;
	*((*symbols)->end) = sym;
	sym->fields = NULL;
	(*symbols)->end++;

	return sym;
}

Symbol *findSymbol(Symbols *symbols, const char *name) {
	Symbol **sym = (Symbol**) malloc(sizeof(Symbol*));
	if (symbols == NULL) {
		return NULL;
	}
	if (symbols->begin == NULL) {
		return NULL;
	}
	for (sym = &(symbols->end[-1]); sym >= symbols->begin; sym--) {
		if (strcmp((*sym)->name, name) == 0)
			return *sym;
	}
	return NULL;
}

void deleteSymbolsAfter(Symbols *symbols, Symbol* sym) {
	Symbol** symHelp;
	Symbol** endSave = symbols->end;
	symHelp = --(symbols->end);

	while (symHelp != symbols->begin && strcmp((*symHelp)->name, sym->name) != 0) {
		(symbols->end)--;
		symHelp--;
	}
	(symbols->end)++;
	if (symHelp == symbols->begin) {
		if (strcmp((*symHelp)->name, sym->name) != 0) {
			symbols->end = endSave;
		} else {
			initSymbols(&symbols);
		}
	}
}

void printSymbols(Symbols* symbols) {
	if (symbols == NULL) {
		printf("Empty\n");
		return;
	}

	Symbol** sym;
	sym = symbols->begin;

	while (sym != symbols->end) {
		for (int j = 0; j < (*sym)->depth; j++) {
			printf("\t");
		}
		if ((*sym)->type == NULL){
			printf("%s\t%d\t%d\t%d\n", (*sym)->name, (*sym)->cls, (*sym)->mem,
							(*sym)->depth);
		}
		else{
		printf("%s\t%d\t%d\t%d\t%d\n", (*sym)->name, (*sym)->cls, (*sym)->mem,
				(*sym)->type->typeBase, (*sym)->depth);
		}
		if ((*sym)->fields != NULL) {

			printSymbols((*sym)->fields);

		}
		sym++;
	}
}

void freeSymbols(Symbols** symbols) {
	if (symbols == NULL)
		return;
	if (*symbols == NULL)
		return;
	Symbol** sym = (*symbols)->begin;
	Symbol** symToFree;
//	if (sym != NULL) {
//		free((*sym)->name);
//		if ((*sym)->fields != NULL) {
//			freeSymbols(&((*sym)->fields));
//		}
//		free(&((*sym)->type));
//		free(*sym);
//		free(sym);
//	}
	while (sym != (*symbols)->end) {
		symToFree = sym;
		sym++;
		free((*symToFree)->name);
		if ((*symToFree)->fields != NULL) {
			freeSymbols(&((*symToFree)->fields));
		}
		if ((*symToFree)->type != NULL)
			free((*symToFree)->type);
		free(*symToFree);
		//free(symToFree);
	}
	free(*symbols);
}
