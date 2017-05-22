/*
 * type.c

 *
 *  Created on: May 10, 2017
 *      Author: zsofi
 */

#include "symbol.h"
#include "token.h"
#include "ASIN.h"

void createType(Type* type, int typeBase, int nElements) {
	type->typeBase = typeBase;
	type->nElements = nElements;
}

void getArithmeticalType(Type* type1, Type* type2, Type* resoult) {
	int a = type1->typeBase, b = type2->typeBase;
	if (a == b) {
		resoult->typeBase = a;
		return;
	}
	switch (a) {
	case TB_INT:
		switch (b) {
		case TB_CHAR:
			resoult->typeBase = TB_INT;
			break;
		case TB_DOUBLE:
			resoult->typeBase = TB_DOUBLE;
			break;
		}
		break;
	case TB_CHAR:
		switch (b) {
		case TB_INT:
			resoult->typeBase = TB_INT;
			break;
		case TB_DOUBLE:
			resoult->typeBase = TB_DOUBLE;
			break;
		}
		break;

	case TB_DOUBLE:
		switch (b) {
		case TB_INT:
		case TB_CHAR:
			resoult->typeBase = TB_DOUBLE;
			break;
		}
		break;
	}
}

void canCast(Type* dst, Type* src, Token* tk) {
	//if one of them is not an array and the other one is
	if (dst->nElements * src->nElements < 0) {
		if (dst->nElements < 0) {
			addTkError(tk->line_nr, "Cannot convert an array to a non-array");
			return;
		} else {
			addTkError(tk->line_nr, "Cannot convert a non-array to an array");
			return;
		}
	}

	//if both are arrays
	if ((dst->nElements >0) && (src->nElements > 0)) {
		if (dst->typeBase != src->typeBase) {
			addTkError(tk->line_nr,
					"Cannot convert an array to another array with different type");
			return;
		}
	}

	//if the source is struct
	if (src->typeBase == TB_STRUCT) {
		//destination must be also a struct
		if (dst->typeBase != TB_STRUCT) {
			addTkError(tk->line_nr, "Incompatible types");
			return;
		}
		//their struct type must be the same
		if (dst->s != src->s) {
			addTkError(tk->line_nr,
					"Cannot convert a struct to another struct type");
			return;
		}
		return;
	}

	if (dst->typeBase == TB_VOID) {
		addTkError(tk->line_nr, "Cannot convert to void");
		return;
	}
	return;

}

void setRetVal(RetVal* resoult, Type* type, int isLVal, int isConstVal, int i,
		double r, char* text) {
	resoult->type = type;
	resoult->isCtVal = isConstVal;
	resoult->isLVal = isLVal;
	if (resoult->type->typeBase == TB_INT)
		resoult->ctVal.i = i;
	else {
		if (resoult->type->typeBase == TB_DOUBLE)
			resoult->ctVal.d = r;
		else {
			if (resoult->type->typeBase == TB_CHAR) {
				if (resoult->type->nElements < 0) {
					resoult->ctVal.i = i;
				} else {
					resoult->ctVal.str = text;
				}
			}
		}
	}
}

Symbol* addFunc(Symbols** symbols, char* name, Type* type) {
	Symbol* func = addSymbol(symbols, name, CLS_EXTFUNC, 0);
	func->type = (Type*)malloc(sizeof(Type));
	func->type->nElements = type->nElements;
	func->type->s = type->s;
	func->type->typeBase = type->typeBase;
	initSymbols(&(func->fields));
	return func;
}

Symbol *addFuncArg(Symbol *func, const char *name, Type* type) {
	Symbol *a = addSymbol(&(func->fields), name, CLS_VAR, 0);
	a->type = type;
	return a;
}

void addBasicFunctions(Symbols** symbols) {
	Type *funcType = (Type*) malloc(sizeof(Type));
	createType(funcType, TB_VOID, -1);

	Type *argType = (Type*) malloc(sizeof(Type));
	createType(argType, TB_CHAR, 0);
	//void put_s(char s[])
	Symbol *func = addFunc(symbols, "put_s", funcType);
	Symbol *arg = addFuncArg(func, "s", argType);
	//void get_s(char s[])
	func = addFunc(symbols, "get_s", funcType);
	arg = addFuncArg(func, "s", argType);
	//void put_i(int i)
	func = addFunc(symbols, "put_i", funcType);
	createType(argType, TB_INT, -1);
	arg = addFuncArg(func, "i", argType);
	//int get_i();
	createType(funcType, TB_INT, -1);
	func = addFunc(symbols, "get_i", funcType);
	//void put_d(double d)
	createType(funcType, TB_VOID, -1);
	func = addFunc(symbols, "put_d", funcType);
	createType(argType, TB_DOUBLE, -1);
	arg = addFuncArg(func, "d", argType);
	//double get_d()
	createType(funcType, TB_DOUBLE, -1);
	func = addFunc(symbols, "get_d", funcType);
	//void put_c(char c)
	createType(funcType, TB_VOID, -1);
	func = addFunc(symbols, "put_c", funcType);
	createType(argType, TB_CHAR, -1);
	arg = addFuncArg(func, "c", argType);
	//char get_c()
	createType(funcType, TB_CHAR, -1);
	func = addFunc(symbols, "put_d", funcType);
	//double seconds()
	createType(funcType, TB_DOUBLE, -1);
	func = addFunc(symbols, "seconds", funcType);
}

