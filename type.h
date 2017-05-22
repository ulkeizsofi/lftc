/*
 * type.h
 *
 *  Created on: May 10, 2017
 *      Author: zsofi
 */

#ifndef TYPE_H_
#define TYPE_H_

typedef union {
	long int i; // int, char
	double d; // double
	const char *str; // char[]
} CtVal;

typedef struct {
	Type* type; // type of the result
	int isLVal; // if it is a LVal
	int isCtVal; // if it is a constant value (int, real, char, char[])
	CtVal ctVal; // the constant value
} RetVal;

void getArithmeticalType(Type* type1, Type* type2, Type* resoult);
void canCast(Type* dst, Type* src, Token* tk);
void createType(Type* type, int typeBase, int nElements);
void setRetVal(RetVal* resoult, Type* type, int isLVal, int isConstVal, int i,
		double r, char* text) ;
void addBasicFunctions(Symbols** symbols);

#endif /* TYPE_H_ */
