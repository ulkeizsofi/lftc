/*
 * ASIN.c
 *
 *  Created on: Mar 22, 2017
 *      Author: zsofi
 */

#include "ASIN.h"
#include <string.h>

Token_Error tkError;

int isExprPrimary(Token** tk);
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
int isTypeName(Token** tk);
int isFuncArg(Token** tk);

int isStm(Token** tk);
int isStmCompound(Token** tk);

void addTkError(int line, const char* errMsg) {
	if (!isError()) {
		tkError.line = line;
		tkError.errorMsg = (char*) malloc(100 * sizeof(char));
		strcpy(tkError.errorMsg, errMsg);
	}
}

int isError() {
	if (tkError.errorMsg == NULL)
		return FALSE;
	return TRUE;
}

char* checkError(int* line) {
	if (isError()) {
		*line = tkError.line;
	}
	return tkError.errorMsg;
}

int consume(int code, Token** crtTk) {
	Token* tk = *crtTk;
	if (tk->cod == code) {
		tk = tk->next;
		*crtTk = tk;
		return TRUE;
	}
	return FALSE;
}

int isExprPrimary(Token** tk) {
	//Save the current token in case of returning false
	Token* savedTk = *tk;
	//If there is Error then FALSE
	if (isError())
		return FALSE;
	//If the current token is CT_INT
	if (consume(CT_INT, tk))
		return TRUE;

	//If the current token is CT_CHAR
	if (consume(CT_CHAR, tk))
		return TRUE;

	//If the current token is CT_STRING
	if (consume(CT_STRING, tk))
		return TRUE;

	//If the current token is CT_REAL
	if (consume(CT_REAL, tk))
		return TRUE;

	if (consume(LPAR, tk)) {
		if (isExpr(tk)) {
			if (consume(RPAR, tk))
				return TRUE;
			addTkError((*tk)->line_nr, "missing ) or sintax error");
			*tk = savedTk;
			return FALSE;
		}
		*tk = savedTk;
		return FALSE;
	}

	if (consume(ID, tk)) {
		if (consume(LPAR, tk)) {
			if (isExpr(tk)) {
				while (consume(COMMA, tk)) {
					if (!isExpr(tk)) {
						addTkError((*tk)->line_nr,
								"missing expression or sintax error");
						*tk = savedTk;
						return FALSE;
					}
				}
			}
			if (consume(RPAR, tk))
				return TRUE;
			else {
				addTkError((*tk)->line_nr, "missing ) or sintax error");
				*tk = savedTk;
				return FALSE;
			}
		} else {
			return TRUE;
		}
	}
	*tk = savedTk;
	return FALSE;
}

int isExprPostfix(Token** tk) {
	//If there is Error then FALSE
	if (isError())
		return FALSE;
	//Save the current token in case of returning false
	Token* savedTk = *tk;
	//If it is NOT Primary then FALSE
	if (!isExprPrimary(tk)) {
		*tk = savedTk;
		;
		return FALSE;
	}
	int brack = 0, dot = 0;
	while ((brack = consume(LBRACKET, tk)) || (dot = consume(DOT, tk))) {
		//Postfix + LBRACKET + expr + RBRACKET
		if (brack) {
			if (!isExpr(tk)) {
				*tk = savedTk;
				return FALSE;
			}
			if (!consume(RBRACKET, tk)) {
				addTkError((*tk)->line_nr, "missing }");
				*tk = savedTk;
				return FALSE;
			}
		} else if (dot) { //Postfix + DOT + ID
			if (!consume(ID, tk)) {
				addTkError((*tk)->line_nr, "missing ID");
				*tk = savedTk;
				return FALSE;
			}
		}
		brack = 0;
		dot = 0;
	}
	return TRUE;
}

int isExprUnary(Token** tk) {
	//If there is Error then FALSE
	if (isError())
		return FALSE;

	//Save the current token in case of returning false
	Token* savedTk = *tk;
	//(SUB|NOT)* Postfix then TRUE
	while (consume(SUB, tk) || consume(NOT, tk))
		;
	if (isExprPostfix(tk))
		return TRUE;
	*tk = savedTk;
	;
	return FALSE;
}

int isExprCast(Token** tk) {
	//If there is Error then FALSE
	if (isError())
		return FALSE;
	//Save the current token in case of returning false
	Token* savedTk = *tk;
	//If (LPAR + expr + RPAR )* Unary then TRUE
	while (consume(LPAR, tk)) {
		if (!isTypeName(tk)) {
			*tk = savedTk;
			;
			return FALSE;
		}
		if (!consume(RPAR, tk)) {
			addTkError((*tk)->line_nr, "missing ) or syntax error");
			*tk = savedTk;
			;
			return FALSE;
		}
	}

	if (isExprUnary(tk))
		return TRUE;
	*tk = savedTk;
	;
	return FALSE;

}

int isExprMul(Token** tk) {
	//If there is Error then FALSE
	if (isError())
		return FALSE;

	//Save the current token in case of returning false
	Token* savedTk = *tk;
	//If it is NOT Cast then FALSE
	if (!isExprCast(tk)) {
		*tk = savedTk;
		;
		return FALSE;
	}

	//(MUL|DIV + Cast)* then TRUE
	while (consume(MUL, tk) || consume(DIV, tk)) {
		if (!isExprCast(tk)) {
			addTkError((*tk)->line_nr, "missing expression");
			*tk = savedTk;
			return FALSE;
		}
	}
	return TRUE;
}

int isExprAdd(Token** tk) {
	//If there is Error then FALSE
	if (isError())
		return FALSE;

	//Save the current token in case of returning false
	Token* savedTk = *tk;
	//If it is NOT Mul then FALSE
	if (!isExprMul(tk)) {
		*tk = savedTk;
		;
		return FALSE;
	}

	//(ADD|SUB + Mul)* then TRUE
	while (consume(ADD, tk) || consume(SUB, tk)) {
		if (!isExprMul(tk)) {
			addTkError((*tk)->line_nr, "missing expression");
			*tk = savedTk;
			return FALSE;
		}
	}
	return TRUE;
}

int isExprRel(Token** tk) {
	//If there is Error then FALSE
	if (isError())
		return FALSE;

	//Save the current token in case of returning false
	Token* savedTk = *tk;
	//If it is NOT Add then FALSE
	if (!isExprAdd(tk)) {
		*tk = savedTk;
		;
		return FALSE;
	}

	//(LESS|LESSEQ|GREATER|GREATEREQ + Add)* then TRUE
	while (consume(LESS, tk) || consume(LESSEQ, tk) || consume(GREATER, tk)
			|| consume(GREATEREQ, tk)) {
		if (!isExprAdd(tk)) {
			addTkError((*tk)->line_nr, "missing expression");
			*tk = savedTk;
			return FALSE;
		}
	}
	return TRUE;
}

int isExprEq(Token** tk) {
	//If there is Error then FALSE
	if (isError())
		return FALSE;

	//Save the current token in case of returning false
	Token* savedTk = *tk;
	//If it is NOT Rel then FALSE
	if (!isExprRel(tk)) {
		*tk = savedTk;
		return FALSE;
	}

	//(EQUAL|NOTEQ + Rel)* then TRUE
	while (consume(EQUAL, tk) || consume(NOTEQ, tk)) {
		if (!isExprRel(tk)) {
			addTkError((*tk)->line_nr, "missing expression");
			*tk = savedTk;
			return FALSE;
		}
	}
	return TRUE;
}

int isExprAnd(Token** tk) {
	//If there is Error then FALSE
	if (isError())
		return FALSE;

	//Save the current token in case of returning false
	Token* savedTk = *tk;
	//If it is NOT Eq then FALSE
	if (!isExprEq(tk)) {
		*tk = savedTk;
		return FALSE;
	}

	//(AND + Eq)* then TRUE
	while (consume(AND, tk)) {
		if (!isExprEq(tk)) {
			addTkError((*tk)->line_nr, "missing expression");

			*tk = savedTk;
			return FALSE;
		}
	}
	return TRUE;
}

int isExprOr(Token** tk) {
	//If there is Error then FALSE
	if (isError())
		return FALSE;

	//Save the current token in case of returning false
	Token* savedTk = *tk;
	//If it is NOT And then FALSE
	if (!isExprAnd(tk)) {
		*tk = savedTk;
		return FALSE;
	}

	//(AND + Eq)* then TRUE
	while (consume(OR, tk)) {
		if (!isExprAnd(tk)) {
			addTkError((*tk)->line_nr, "missing expression");
			*tk = savedTk;
			return FALSE;
		}
	}
	return TRUE;
}

int isExprAssign(Token** tk) {
	//If there is Error then FALSE
	if (isError())
		return FALSE;

	//Save the current token in case of returning false
	Token* savedTk = *tk;
	//Unary ASSIGN Assign | Or
	if (isExprUnary(tk)) {
		if (consume(ASSIGN, tk)) {
			int isAssign = isExprAssign(tk);
			if (!isAssign) {
				addTkError((*tk)->line_nr, "missing expression");
				*tk = savedTk;
			}
			return isAssign; //This Assign is just a Unary
		} else
			*tk = savedTk;
	}
	//If it's Or then TRUE
	if (isExprOr(tk))
		return TRUE;
	*tk = savedTk;
	return FALSE;
}

int isExpr(Token** tk) {
	//If there is Error then FALSE
	if (isError())
		return FALSE;

	return isExprAssign(tk);
}

int isTypeBase(Token** tk) {
	//If there is Error then FALSE
	if (isError())
		return FALSE;

	//Save the current token in case of returning false
	Token* savedTk = *tk;

	//INT | DOUBLE | CHAR | STRUCT ID
	if (consume(INT, tk))
		return TRUE;
	if (consume(DOUBLE, tk))
		return TRUE;
	if (consume(CHAR, tk))
		return TRUE;
	if (consume(STRUCT, tk)) {
		if (consume(ID, tk))
			return TRUE;
		*tk = savedTk;
		return FALSE;
	}

	return FALSE;
}

int isArrayDecl(Token** tk) {

	//If there is Error then FALSE
	if (isError())
		return FALSE;
	//Save the current token in case of returning false
	Token* savedTk = *tk;
	//LBRACKET + expr? + RBRACKET
	if (!consume(LBRACKET, tk))
		return FALSE;
	isExpr(tk); //It's return value does not matter
	if (consume(RBRACKET, tk))
		return TRUE;
	addTkError((*tk)->line_nr, "missing ] or syntax error");
	*tk = savedTk;
	return FALSE;
}

int isVarDecl(Token** tk) {
	//If there is Error then FALSE
	if (isError())
		return FALSE;

	//Save the current token in case of returning false
	Token* savedTk = *tk;
	if (!isTypeBase(tk))
		return FALSE;
	while (consume(MUL, tk))
		;
	if (!consume(ID, tk)) {
		*tk = savedTk;
		return FALSE;
	}
	isArrayDecl(tk);
	while (consume(COMMA, tk)) {
		if (!consume(ID, tk)) {
			addTkError((*tk)->line_nr, "missing ID");
			*tk = savedTk;
			return FALSE;
		}
		isArrayDecl(tk);
	}
	if (consume(SEMICOLON, tk))
		return TRUE;
	addTkError((*tk)->line_nr, "missing ;");
	*tk = savedTk;
	return FALSE;
}

int isStructDecl(Token** tk) {
	//If there is Error then FALSE
	if (isError())
		return FALSE;

	//Save the current token in case of returning false
	Token* savedTk = *tk;
	if (!consume(STRUCT, tk))
		return FALSE;
	if (!consume(ID, tk)) {
		addTkError((*tk)->line_nr, "ID is missing in STRUCT statement");
		*tk = savedTk;
		return FALSE;
	}
	if (!consume(LACC, tk)) {
		*tk = savedTk;
		return FALSE;
	}
	while (isVarDecl(tk))
		;
	if (!consume(RACC, tk)) {
		addTkError((*tk)->line_nr, "} is missing in STRUCT statement");
		*tk = savedTk;
		return FALSE;
	}
	if (consume(SEMICOLON, tk))
		return TRUE;
	addTkError((*tk)->line_nr, "; is missing in STRUCT statement");
	*tk = savedTk;
	return FALSE;
}

int isFuncDecl(Token** tk) {
	//If there is Error then FALSE
	if (isError())
		return FALSE;

	//Save the current token in case of returning false
	Token* savedTk = *tk;
	//TypeBase MUL | VOID ID LPAR TypeBase ID Array? COMMA (..) RPAR stmCompound
	if (isTypeBase(tk)) {
		while (consume(MUL, tk))
			;
	} else {
		if (!consume(VOID, tk))
			return FALSE;
	}
	if (consume(ID, tk)) {
		if (consume(LPAR, tk)) {
			if (isFuncArg(tk)) {
				while (consume(COMMA, tk)) {
					if (!isFuncArg(tk)) {
						addTkError((*tk)->line_nr, "missing argument");
						*tk = savedTk;
						return FALSE;
					}
				}
			}
			if (consume(RPAR, tk)) {
				if (isStmCompound(tk))
					return TRUE;
				addTkError((*tk)->line_nr, "missing statement");
			} else {
				addTkError((*tk)->line_nr, "missing ) or syntax error");
			}
		}
	}
	*tk = savedTk;
	return FALSE;
}

int isStmCompound(Token** tk) {
	//If there is Error then FALSE
	if (isError())
		return FALSE;

	//Save the current token in case of returning false
	Token* savedTk = *tk;

	if (!consume(LACC, tk))
		return FALSE;
	while (isVarDecl(tk) || isStm(tk))
		;
	if (consume(RACC, tk))
		return TRUE;
	addTkError((*tk)->line_nr, "missing } or syntax error");
	*tk = savedTk;
	return FALSE;
}

int isIF(Token** tk) {
	//If there is Error then FALSE
	if (isError())
		return FALSE;

	//Save the current token in case of returning false
	Token* savedTk = *tk;
	//IF + LPAR + expr + RPAR + stm + ELSE stm
	if (consume(IF, tk)) {
		if (consume(LPAR, tk)) {
			if (isExpr(tk)) {
				if (consume(RPAR, tk)) {
					if (isStm(tk)) {
						//ELSE + stm not mandatory
						if (consume(ELSE, tk)) {
							if (isStm(tk))
								return TRUE;
							addTkError((*tk)->line_nr,
									"missing statement after else or syntax error");
							*tk = savedTk;
							return FALSE;
						}
						return TRUE; //without else
					} else
						addTkError((*tk)->line_nr,
								"missing statement after if or syntax error");
				} else
					addTkError((*tk)->line_nr, "missing ) or syntax error");
			} else
				addTkError((*tk)->line_nr,
						"missing expression or syntax error");
		} else
			addTkError((*tk)->line_nr, "missing ( after if or syntax error");
	}
	*tk = savedTk;
	return FALSE;
}

int isWhile(Token** tk) {
	//If there is Error then FALSE
	if (isError())
		return FALSE;

	//Save the current token in case of returning false
	Token* savedTk = *tk;

	if (!consume(WHILE, tk))
		return FALSE;
	if (consume(LPAR, tk)) {
		if (isExpr(tk)) {
			if (consume(RPAR, tk)) {
				if (isStm(tk)) {
					return TRUE;
				} else {
					addTkError((*tk)->line_nr,
							"missing statement after while or syntax error");
				}
			} else {
				addTkError((*tk)->line_nr, "missing ) or syntax error");
			}
		} else {
			addTkError((*tk)->line_nr, "missing expression or syntax error");
		}
	} else {
		addTkError((*tk)->line_nr, "missing ( or syntax error");
	}

	*tk = savedTk;
	return FALSE;
}

int isFor(Token** tk) {
	//If there is Error then FALSE
	if (isError())
		return FALSE;

	//Save the current token in case of returning false
	Token* savedTk = *tk;

	//for (e;e;e) stm
	if (!consume(FOR, tk))
		return FALSE;
	if (consume(LPAR, tk)) {
		isExpr(tk);
		if (consume(SEMICOLON, tk)) {
			isExpr(tk);
			if (consume(SEMICOLON, tk)) {
				isExpr(tk);
				if (consume(RPAR, tk)) {
					if (isStm(tk)) {
						return TRUE;
					} else {
						addTkError((*tk)->line_nr,
								"missing statement or syntax error");
					}
				} else {
					addTkError((*tk)->line_nr, "missing ) or syntax error");
				}
			} else {
				addTkError((*tk)->line_nr, "missing ; or syntax error");
			}
		} else {
			addTkError((*tk)->line_nr, "missing ; or syntax error");
		}

	} else {
		addTkError((*tk)->line_nr, "missing ( or syntax error");
	}

	*tk = savedTk;
	return FALSE;
}

int isBreak(Token** tk) {
	//If there is Error then FALSE
	if (isError())
		return FALSE;

	//Save the current token in case of returning false
	Token* savedTk = *tk;
	if (!consume(BREAK, tk))
		return FALSE;
	if (consume(SEMICOLON, tk))
		return TRUE;

	addTkError((*tk)->line_nr, "missing ; or syntax error");
	*tk = savedTk;
	return FALSE;
}

int isReturn(Token** tk) {
	//If there is Error then FALSE
	if (isError())
		return FALSE;

	//Save the current token in case of returning false
	Token* savedTk = *tk;
	if (!consume(RETURN, tk))
		return FALSE;
	isExpr(tk);
	if (consume(SEMICOLON, tk))
		return TRUE;
	addTkError((*tk)->line_nr, "missing ; after return or syntax error");
	*tk = savedTk;
	return FALSE;
}

int isStm(Token** tk) {
	//If there is Error then FALSE
	if (isError())
		return FALSE;

	//Save the current token in case of returning false
	Token* savedTk = *tk;

	if (isStmCompound(tk))
		return TRUE;
	if (isIF(tk))
		return TRUE;
	if (isWhile(tk))
		return TRUE;
	if (isFor(tk))
		return TRUE;
	if (isBreak(tk))
		return TRUE;
	if (isReturn(tk))
		return TRUE;

	//expr? + SEMICOLON
	isExpr(tk);
	if (consume(SEMICOLON, tk))
		return TRUE;
	*tk = savedTk;
	return FALSE;
}

int isTypeName(Token** tk) {
	//If there is Error then FALSE
	if (isError())
		return FALSE;

	if (!isTypeBase(tk))
		return FALSE;
	isArrayDecl(tk);
	return TRUE;
}

int isFuncArg(Token** tk) {
	//If there is Error then FALSE
	if (isError())
		return FALSE;

	//Save the current token in case of returning false
	Token* savedTk = *tk;

	if (!isTypeBase(tk))
		return FALSE;
	if (consume(ID, tk)) {
		isArrayDecl(tk);
		return TRUE;
	}
	addTkError((*tk)->line_nr, "missing ID");
	*tk = savedTk;
	return FALSE;
}

int isUnit(Token** tk) {
	//set Error to FALSE
	tkError.errorMsg = NULL;

	while (isStructDecl(tk) || isFuncDecl(tk) || isVarDecl(tk))
		;
	if (consume(C_END, tk))
		return TRUE;
	return FALSE;
}
