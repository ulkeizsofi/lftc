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

#define TRUE 1
#define FALSE 0
#define MAX_ERR_NO 100

typedef struct _Token_Error{
	int line;
	char* errorMsg;
}Token_Error;


int isUnit(Token** tk);
char* checkError(int* line);

#endif /* ASIN_H_ */
