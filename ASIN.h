/*
 * ASIN.h
 *
 *  Created on: Mar 22, 2017
 *      Author: zsofi
 */

#ifndef ASIN_H_
#define ASIN_H_

#include token.h

#define TRUE 1
#define FALSE 0
#define MAX_ERR_NO 100

typedef struct _Token_Error{
	Token* tk;
	char errorMsg[100];
}Token_Error;

int isExprPrimary(Token* tk);
int consume(int code, Token** crtTk);
int isExprPostfix(Token** tk);
int isExpr(Token** tk);
int isExprUnary(Token** tk);
int isExprCast(Token** tk);
int isExprMul(Token** tk);
int isTypeBase(Token** tk);
int isExprAdd(Token** tk);
int isExprRel(Token** tk);
int isExprEq(Token** tk);
int isExprAnd(Token** tk);
int isExprOr(Token** tk);
int isExprAssign(Token** tk);
int isArrayDecl(Token** tk);
int isVarDecl(Token** tk);
int isStructDecl(Token** tk);
int isFuncDecl(Token** tk);

#endif /* ASIN_H_ */
