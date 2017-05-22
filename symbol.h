/*
 * symbol.h
 *
 *  Created on: Apr 25, 2017
 *      Author: zsofi
 */

#ifndef SYMBOL_H_
#define SYMBOL_H_

#define INITIAL_SYMBOLS_SIZE 10

enum tb{TB_INT,TB_DOUBLE,TB_CHAR,TB_STRUCT,TB_VOID};
enum cls{CLS_VAR,CLS_FUNC,CLS_EXTFUNC,CLS_STRUCT};
enum mem{MEM_GLOBAL = 0,MEM_ARG,MEM_LOCAL};

typedef struct _Symbol Symbol;

typedef struct _Type{
	int typeBase; // TB_*
	Symbol *s; // struct definition for TB_STRUCT
	int nElements; // >0 array of given size, 0=array without size, <0 non array
}Type;



typedef struct _Symbols{
	Symbol **begin; // the beginning of the symbols, or NULL
	Symbol **end; // the position after the last symbol
	Symbol **after; // the position after the allocated space
}Symbols;

typedef struct _Symbol{
	char *name; // a reference to the name stored in a token
	int cls; // CLS_*
	int mem; // MEM_*
	Type* type;
	int depth; // 0-global, 1-in function, 2... - nested blocks in function
	Symbols *fields; // used only for structs and functions
}Symbol;

Symbol *findSymbol(Symbols *symbols,const char *name);
Symbol *addSymbol(Symbols **symbols,const char *name,int cls,int depth);
void initSymbols(Symbols** symbols);
void deleteSymbolsAfter(Symbols *symbols, Symbol* sym);
void printSymbols(Symbols* symbols);
void freeSymbols(Symbols** symbols);

#endif /* SYMBOL_H_ */
