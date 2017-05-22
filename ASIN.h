/*
 * ASIN.h
 *
 *  Created on: Mar 22, 2017
 *      Author: zsofi
 */

#ifndef ASIN_H_
#define ASIN_H_

#include "token.h"
#include <stdlib.h>
#include "symbol.h"
#include "type.h"

#define TRUE 1
#define FALSE 0
#define MAX_ERR_NO 100

typedef struct _Token_Error{
	int line;
	char* errorMsg;
}Token_Error;


Symbols* isUnit(Token* tk, RetVal val);
char* checkError(int* line);
void addTkError(int line, const char* errMsg);

#endif /* ASIN_H_ */
