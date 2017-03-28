/*
 * ASIN.c
 *
 *  Created on: Mar 22, 2017
 *      Author: zsofi
 */

#include "ASIN.h"
#include <string.h>

Token_Error* tkError = (Token_Error*) malloc(MAX_ERR_NO * sizeof(Token_Error));
int errNo = 0;

void addTkError(Token* tk, const char* errMsg) {
	tkError[errNo].tk = tk;
	strcpy(tkError[errNo].errorMsg, errMsg);
}

int consume(int code, Token** crtTk) {
	Token* tk = *crtTk;
	if (tk->cod == code) {
		tk = tk->next;
		return TRUE;
	}
	return FALSE;
}

int isExprPrimary(Token** tk) {
	//If the current token is CT_INT
	if (consume(CT_INT))
		return TRUE;

	//If the current token is CT_CHAR
	if (consume(CT_CHAR))
		return TRUE;

	//If the current token is CT_STRING
	if (consume(CT_STRING))
		return TRUE;

	//If the current token is CT_REAL
	if (consume(CT_REAL))
		return TRUE;

	//TODO: the other 2
	return FALSE;
}

int isExprPostfix(Token** tk) {
	//Save the current token in case of returning false
	Token* savedTk = *tk;
	//If it is NOT Primary then FALSE
	if (!isExprPrimary(tk)) {
		tk = &savedTk;
		return FALSE;
	}

	//Postfix + LBRACKET + expr + RBRACKET
	while (consume(LBRACKET, tk)) {
		if (!isExpr(tk)) {
			tk = &savedTk;
			return FALSE;
		}
		if (!consume(RBRACKET, tk)) {
			tk = &savedTk;
			return FALSE;
		}
	}

	//Postfix + DOT + ID
	while (consume(DOT, tk)) {
		if (!consume(ID, tk)) {
			tk = &savedTk;
			return FALSE;
		}
	}

	return TRUE;
}

int isExprUnary(Token** tk) {
	//Save the current token in case of returning false
	Token* savedTk = *tk;
	//(SUB|NOT)* Postfix then TRUE
	while (consume(SUB, tk) || consume(NOT, tk))
		;
	if (isExprPostfix(tk))
		return TRUE;
	tk = &savedTk;
	return FALSE;
}

int isExprCast(Token** tk) {
	//Save the current token in case of returning false
	Token* savedTk = *tk;
	//If (LPAR + expr + RPAR )* Unary then TRUE
	while (consume(LPAR, tk)) {
		if (!isTypeBase(tk)) {
			tk = &savedTk;
			return FALSE;
		}
		if (!consume(RPAR, tk)) {
			addTkError(*tk, "missing ) or syntax error");
			tk = &savedTk;
			return FALSE;
		}
	}

	if (isExprCast(tk))
		return TRUE;
	tk = &savedTk;
	return FALSE;

}

int isExprMul(Token** tk) {
	//Save the current token in case of returning false
	Token* savedTk = *tk;
	//If it is NOT Cast then FALSE
	if (!isExprCast(tk)) {
		tk = &savedTk;
		return FALSE;
	}

	//(MUL|DIV + Cast)* then TRUE
	while (consume(MUL, tk) || consume(DIV, tk)) {
		if (!isExprCast(tk)) {
			tk = &savedTk;
			return FALSE;
		}
	}
	return TRUE;
}

int isExprAdd(Token** tk) {
	//Save the current token in case of returning false
	Token* savedTk = *tk;
	//If it is NOT Mul then FALSE
	if (!isExprMul(tk)) {
		tk = &savedTk;
		return FALSE;
	}

	//(ADD|SUB + Mul)* then TRUE
	while (consume(ADD, tk) || consume(SUB, tk)) {
		if (!isExprMul(tk)) {
			tk = &savedTk;
			return FALSE;
		}
		return TRUE;
	}

	int isExprRel(Token** tk) {
		//Save the current token in case of returning false
		Token* savedTk = *tk;
		//If it is NOT Add then FALSE
		if (!isExprAdd(tk)) {
			tk = &savedTk;
			return FALSE;
		}

		//(LESS|LESSEQ|GREATER|GREATEREQ + Add)* then TRUE
		while (consume(LESS, tk) || consume(LESSEQ, tk) || consume(GREATER, tk)
				|| consume(GREATEREQ, tk)) {
			if (!isExprAdd(tk)) {
				tk = &savedTk;
				return FALSE;
			}
		}
		return TRUE;
	}

	int isExprEq(Token** tk) {
		//Save the current token in case of returning false
		Token* savedTk = *tk;
		//If it is NOT Rel then FALSE
		if (!isExprRel(tk)) {
			tk = &savedTk;
			return FALSE;
		}

		//(EQUAL|NOTEQ + Rel)* then TRUE
		while (consume(EQUAL, tk) || consume(NOTEQ, tk)) {
			if (!isExprRel(tk)) {
				tk = &savedTk;
				return FALSE;
			}
		}
		return TRUE;
	}

	int isExprAnd(Token** tk) {
		//Save the current token in case of returning false
		Token* savedTk = *tk;
		//If it is NOT Eq then FALSE
		if (!isExprEq(tk)) {
			tk = &savedTk;
			return FALSE;
		}

		//(AND + Eq)* then TRUE
		while (consume(AND, tk)) {
			if (!isExprEq(tk)) {
				tk = &savedTk;
				return FALSE;
			}
		}
		return TRUE;
	}

	int isExprOr(Token** tk) {
		//Save the current token in case of returning false
		Token* savedTk = *tk;
		//If it is NOT And then FALSE
		if (!isExprAnd(tk)) {
			tk = &savedTk;
			return FALSE;
		}

		//(AND + Eq)* then TRUE
		while (consume(OR, tk)) {
			if (!isExprAnd(tk)) {
				tk = &savedTk;
				return FALSE;
			}
		}
		return TRUE;
	}

	int isExprAssign(Token** tk) {
		//Save the current token in case of returning false
		Token* savedTk = *tk;
		//Unary ASSIGN Assign | Or
		while (isExprUnary(tk)) {
			if (!consume(ASSIGN, tk))
				return TRUE; //This Assign is just a Unary
		}
		//If its Or then TRUE
		if (isExprOr(tk))
			return TRUE;
		tk = &savedTk;
		return FALSE;
	}

	int isExpr(Token** tk) {
		return isExprAssign(tk);
	}

	int isTypeBase(Token** tk) {
		//INT | DOUBLE | CHAR | STRUCT ID
		if (consume(INT, tk))
			return TRUE;
		if (consume(DOUBLE, tk))
			return TRUE;
		if (consume(CHAR, tk))
			return TRUE;
		//if (consume(STRUCT, tk) ) return TRUE;
		return FALSE;
	}

	int isArrayDecl(Token** tk) {
		//Save the current token in case of returning false
		Token* savedTk = *tk;
		//LBRACKET + expr? + RBRACKET
		if (!consume(LBRACKET, tk))
			return FALSE;
		isExpr(tk); //It's return value does not matter
		return consume(RBRACKET, tk);
	}

	int isVarDecl(Token** tk) {
		//Save the current token in case of returning false
		Token* savedTk = *tk;
		if (!isTypeBase(tk))
			return FALSE;
		while (consume(MUL, tk))
			;
		if (!consume(ID, tk)) {
			tk = &savedTk;
			return FALSE;
		}
		isArrayDecl(tk);
		while (consume(COMMA, tk)) {
			if (!consume(ID, tk)) {
				tk = &savedTk;
				return FALSE;
			}
			isArrayDecl(tk);
		}
		if (consume(SEMICOLON, tk))
			return TRUE;
		tk = &savedTk;
		return FALSE;
	}

	int isStructDecl(Token** tk) {
		//Save the current token in case of returning false
		Token* savedTk = *tk;
		if (!consume(STRUCT, tk))
			return FALSE;
		if (!consume(ID, tk)) {
			tk = &savedTk;
			return FALSE;
		}
		if (!consume(LACC, tk)) {
			tk = &savedTk;
			return FALSE;
		}
		while (isVarDecl(tk))
			;
		if (!consume(RACC, tk)) {
			tk = &savedTk;
			return FALSE;
		}
		if (consume(SEMICOLON, tk))
			return TRUE;
		tk = &savedTk;
		return FALSE;
	}

	int isFuncDecl(Token** tk) {
		//Save the current token in case of returning false
		Token* savedTk = *tk;
		//TypeBase MUL | VOID ID LPAR TypeBase ID Array? COMMA (..) RPAR stmCompound
		if (!isTypeBase(tk))
			return FALSE;
		while (consume(MUL, tk))
			;

	}
