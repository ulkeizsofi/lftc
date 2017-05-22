/*
 * ASIN.c
 *
 *  Created on: Mar 22, 2017
 *      Author: zsofi
 */

#include "ASIN.h"
#include <string.h>
#include "symbol.h"
#include "type.h"

Token_Error tkError;
Symbols* symbols;
Symbol* crtStruct;
Symbol* crtFunc;
int crtDepth;

int isExprPrimary(Token** tk, RetVal *val);
int consume(int code, Token** crtTk);
int isExprPostfix(Token** tk, RetVal* val);
int isExpr(Token** tk, RetVal* val);
int isExprUnary(Token** tk, RetVal* val);
int isExprCast(Token** tk, RetVal* val);
int isExprMul(Token** tk, RetVal* val);
Type* isTypeBase(Token** tk);
int isExprAdd(Token** tk, RetVal* val);
int isExprRel(Token** tk, RetVal* val);
int isExprEq(Token** tk, RetVal* val);
int isExprAnd(Token** tk, RetVal* val);
int isExprOr(Token** tk, RetVal* val);
int isExprAssign(Token** tk, RetVal* val);
int isArrayDecl(Token** tk, RetVal* val, Type* type);
int isVarDecl(Token** tk, RetVal* val);
int isStructDecl(Token** tk, RetVal* val);
int isFuncDecl(Token** tk, RetVal* val);
Type* isTypeName(Token** tk, RetVal* val);
int isFuncArg(Token** tk, RetVal* val);
int addVar(Token* tken, Type* type);
int isStm(Token** tk, RetVal* val);
int isStmCompound(Token** tk, RetVal* val);
int isError();

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

int isExprPrimary(Token** tk, RetVal *val) {
	//Save the current token in case of returning false
	Token* savedTk = *tk;
	//If there is Error then FALSE
	if (isError())
		return FALSE;
	Token* conservedTk = *tk;
	Symbol* argSym;
	Symbol** crtArg;
	//If the current token is CT_INT
	if (consume(CT_INT, tk)) {
		Type* type = (Type*) malloc(sizeof(Type));
		createType(type, TB_INT, -1);
		setRetVal(val, type, 0, 1, conservedTk->i, conservedTk->r,
				conservedTk->text);
		return TRUE;
	}

	//If the current token is CT_CHAR
	if (consume(CT_CHAR, tk)) {
		Type* type = (Type*) malloc(sizeof(Type));
		createType(type, TB_CHAR, -1);
		setRetVal(val, type, 0, 1, conservedTk->i, conservedTk->r,
				conservedTk->text);
		return TRUE;
	}

	//If the current token is CT_STRING
	if (consume(CT_STRING, tk)) {
		Type* type = (Type*) malloc(sizeof(Type));
		createType(type, TB_CHAR, strlen(conservedTk->text));
		setRetVal(val, type, 0, 1, conservedTk->i, conservedTk->r,
				conservedTk->text);
		return TRUE;
	}

	//If the current token is CT_REAL
	if (consume(CT_REAL, tk)) {
		Type* type = (Type*) malloc(sizeof(Type));
		createType(type, TB_DOUBLE, -1);
		setRetVal(val, type, 0, 1, conservedTk->i, conservedTk->r,
				conservedTk->text);
		return TRUE;
	}

	if (consume(LPAR, tk)) {
		if (isExpr(tk, val)) {
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
		//search for the ID in the symbols list
		Symbol* sym = findSymbol(symbols, conservedTk->text);

		if (sym == NULL) {
			addTkError((conservedTk)->line_nr, "undefined symbol");
			*tk = savedTk;
			return FALSE;
		}
		if (sym->type == NULL) {
			addTkError((conservedTk)->line_nr, "syntax error");
			*tk = savedTk;
			return FALSE;
		}
		setRetVal(val, sym->type, 1, 0, conservedTk->i, conservedTk->r,
				conservedTk->text);

		if (consume(LPAR, tk)) {
			//if it is not a function
			if (sym->cls != CLS_FUNC && sym->cls != CLS_EXTFUNC) {
				addTkError((conservedTk)->line_nr, "not a function");
				*tk = savedTk;
				return FALSE;
			}
			crtArg = sym->fields->begin;
			conservedTk = *tk;
			//check the parameter list
			if (isExpr(tk, val)) {
				//if it is a no-arg fucntion
				if (sym->fields->begin == sym->fields->end) {
					addTkError((conservedTk)->line_nr,
							"too many arguments in function call");
					*tk = savedTk;
					return FALSE;
				}
				//check if it exists in the symbols list
				if (val->type->typeBase == TB_CHAR) {
					argSym = findSymbol(symbols, conservedTk->text);

					if (argSym == NULL) {
						addTkError(conservedTk->line_nr, "undefined symbol");
						*tk = savedTk;
						return FALSE;
					}
				}
				//if the parameter can be casted to the original type
				canCast((*crtArg)->type, val->type, conservedTk);
				crtArg++;
				while (consume(COMMA, tk)) {
					conservedTk = *tk;
					if (!isExpr(tk, val)) {
						addTkError((*tk)->line_nr,
								"missing expression or sintax error");
						*tk = savedTk;
						return FALSE;
					}
					if (crtArg == sym->fields->end) {
						addTkError((conservedTk)->line_nr,
								"too many arguments in function call");
						*tk = savedTk;
						return FALSE;
					}
					//check if it exists in the symbols list
					if (val->type->typeBase == TB_CHAR) {
						argSym = findSymbol(symbols, conservedTk->text);

						if (argSym == NULL) {
							addTkError(conservedTk->line_nr,
									"undefined symbol");
							*tk = savedTk;
							return FALSE;
						}
					}
					//if the parameter can be casted to the original type
					canCast((*crtArg)->type, val->type, conservedTk);
					crtArg++;
				}
			}
			if (consume(RPAR, tk)) {
				//if the argument list hasn`t reached the end of parameter list yet
				if (crtArg != (sym->fields->end)) {
					addTkError(conservedTk->line_nr,
							"too few arguments in function call");
					*tk = savedTk;
					return FALSE;
				}
				setRetVal(val, sym->type, 0, 0, conservedTk->i, conservedTk->r,
						conservedTk->text);
				return TRUE;
			} else {
				addTkError((*tk)->line_nr, "missing ) or sintax error");
				*tk = savedTk;
				return FALSE;
			}
		} else {
			Symbol * sym = findSymbol(symbols, conservedTk->text);
			{
				if (sym != NULL) {
					if (sym->cls == CLS_EXTFUNC || sym->cls == CLS_FUNC) {
						addTkError(conservedTk->line_nr,
								"missing call for function");
						*tk = savedTk;
						return FALSE;
					}
					setRetVal(val, sym->type, 1, 0, conservedTk->i,
							conservedTk->r, conservedTk->text);
				}
			}
			return TRUE;
		}
	}
	*tk = savedTk;
	return FALSE;
}

int isExprPostfix(Token** tk, RetVal* val) {
//If there is Error then FALSE
	if (isError())
		return FALSE;
//Save the current token in case of returning false
	Token* savedTk = *tk;

//Save the token to check in the symbols list and to do other ops
	Token* conservedTk = *tk;

//If it is NOT Primary then FALSE
	if (!isExprPrimary(tk, val)) {
		*tk = savedTk;
		;
		return FALSE;
	}
	int brack = 0, dot = 0;
	while ((brack = consume(LBRACKET, tk)) || (dot = consume(DOT, tk))) {
		//search for the token in the symbols list
		Symbol* sym = findSymbol(symbols, conservedTk->text);
		//if it was not found then it can only be declaration
		//dot is not allowed
		if (sym == NULL) {
			addTkError((*conservedTk).line_nr, "undefined symbol");
			*tk = savedTk;
			return FALSE;
		}

		//Postfix + LBRACKET + expr + RBRACKET
		if (brack) {
			conservedTk = *tk;
			//TODO: check if it was a vector, check if expr ok. [expr]
			if (!isExpr(tk, val)) {
				*tk = savedTk;
				return FALSE;
			}
			//check if it was previously declared
			if (sym != NULL) {
				//check if it was an array
				if (sym->type->nElements < 0) {
					addTkError((*conservedTk).line_nr,
							"Expression is not an array");
					*tk = savedTk;
					return FALSE;
				}

			}
			//check if the expression can be casted to an int
			//create a type INT
			Type* typeInt = (Type*) malloc(sizeof(Type));
			createType(typeInt, TB_INT, -1);
			//check the cast
			canCast(typeInt, val->type, conservedTk);

			setRetVal(val, sym->type, 1, 0, conservedTk->i, conservedTk->r,
					conservedTk->text);

			if (!consume(RBRACKET, tk)) {
				addTkError((*tk)->line_nr, "missing }");
				*tk = savedTk;
				return FALSE;
			}
		} else if (dot) { //Postfix + DOT + ID
			//check if it was a struct, check if the field exist
			conservedTk = *tk;
			if (!consume(ID, tk)) {
				addTkError((*tk)->line_nr, "missing ID");
				*tk = savedTk;
				return FALSE;
			}
			Symbol* symStruct = val->type->s;
			Symbol *symField = findSymbol(symStruct->fields, conservedTk->text);

			if (symField == NULL) {
				addTkError((conservedTk)->line_nr, "undefined member");
				*tk = savedTk;
				return FALSE;
			}

			setRetVal(val, symField->type, 1, 0, conservedTk->i, conservedTk->r,
					conservedTk->text);
		}
		brack = 0;
		dot = 0;
	}
	return TRUE;
}

int isExprUnary(Token** tk, RetVal* val) {
//If there is Error then FALSE
	if (isError())
		return FALSE;
	Token* conservedTk;
//Save the current token in case of returning false
	Token* savedTk = *tk;
//(SUB|NOT)* Postfix then TRUE
	int sub = 0, not = 0;
	int sub_save = 0, not_save = 0;
	do {
		sub_save = sub;
		not_save = not;
		sub = consume(SUB, tk);
		not = consume(NOT, tk);
	} while (sub || not);
	sub = sub_save;
	not = not_save;
	conservedTk = *tk;
	if (isExprPostfix(tk, val)) {
		if (sub) {
			//if it is an array then error
			if (val->type->nElements >= 0) {
				addTkError(conservedTk->line_nr,
						"operation - cannot be applied to an array");
				*tk = savedTk;
				;
				return FALSE;
			}
			//if it is a struct then error
			if (val->type->typeBase == TB_STRUCT) {
				addTkError(conservedTk->line_nr,
						"operation - cannot be applied to a struct");
				*tk = savedTk;
				;
				return FALSE;
			}
		}
		if (not) {
			//struct -> error
			if (val->type->typeBase == TB_STRUCT) {
				addTkError(conservedTk->line_nr,
						"operation ! cannot be applied to a struct");
				*tk = savedTk;
				;
				return FALSE;
			}
			createType(val->type, TB_INT, -1);
			val->isCtVal = 0;
			val->isLVal = 0;
		}

		return TRUE;
	}
	*tk = savedTk;
	;
	return FALSE;
}

int isExprCast(Token** tk, RetVal* val) {
//If there is Error then FALSE
	if (isError())
		return FALSE;
//Save the current token in case of returning false
	Token* savedTk = *tk;
	Token* conservedTk = *tk;
	Type* type = (Type*) malloc(sizeof(Type));
//If (LPAR + expr + RPAR )* Unary then TRUE
	while (consume(LPAR, tk)) {
		conservedTk = *tk;
		if (!isTypeName(tk, val)) {
			*tk = savedTk;
			;
			return FALSE;
		}
		//create a type with the val value

		createType(type, val->type->typeBase, -1);
		if (!consume(RPAR, tk)) {
			addTkError((*tk)->line_nr, "missing ) or syntax error");
			*tk = savedTk;
			;
			return FALSE;
		}
	}

	conservedTk = *tk;
	if (isExprUnary(tk, val)) {
		if (type != NULL)
			canCast(type, val->type, conservedTk);
		return TRUE;
	}
	*tk = savedTk;
	;
	return FALSE;

}

int isExprMul(Token** tk, RetVal* val) {
//If there is Error then FALSE
	if (isError())
		return FALSE;

//Save the current token in case of returning false
	Token* savedTk = *tk;
//If it is NOT Cast then FALSE
	Token* conservedTk = *tk;
	if (!isExprCast(tk, val)) {
		*tk = savedTk;
		;
		return FALSE;
	}

//(MUL|DIV + Cast)* then TRUE
	while (consume(MUL, tk) || consume(DIV, tk)) {
		//check if before MUL|DIV there was an array
		if (val->type->nElements >= 0) {
			addTkError((conservedTk)->line_nr,
					"operation * or / cannot be applied on an array");
			*tk = savedTk;
			return FALSE;
		}
		//check if there was a struct
		if (val->type->typeBase == TB_STRUCT) {
			addTkError((conservedTk)->line_nr,
					"operation * or / cannot be applied on a struct");
			*tk = savedTk;
			return FALSE;
		}
		//save the type
		Type type1 = *(val->type);
		if (!isExprCast(tk, val)) {
			addTkError((*tk)->line_nr, "missing expression after * OR /");
			*tk = savedTk;
			return FALSE;
		}
		//check if after MUL|DIV there was an array
		if (val->type->nElements >= 0) {
			addTkError((conservedTk)->line_nr,
					"operation * or / cannot be applied on an array");
			*tk = savedTk;
			return FALSE;
		}
		//check if there was a struct after MUL|DIV
		if (val->type->typeBase == TB_STRUCT) {
			addTkError((conservedTk)->line_nr,
					"operation * or / cannot be applied on a struct");
			*tk = savedTk;
			return FALSE;
		}
		getArithmeticalType(&type1, val->type, val->type);
		val->isCtVal = 0;
		val->isLVal = 0;
	}
	return TRUE;
}

int isExprAdd(Token** tk, RetVal* val) {
//If there is Error then FALSE
	if (isError())
		return FALSE;

//Save the current token in case of returning false
	Token* savedTk = *tk;
//If it is NOT Mul then FALSE
	Token* conservedTk = *tk;
	if (!isExprMul(tk, val)) {
		*tk = savedTk;
		;
		return FALSE;
	}

//(ADD|SUB + Mul)* then TRUE
	while (consume(ADD, tk) || consume(SUB, tk)) {
		//check if before ADD|SUB there was an array
		if (val->type->nElements >= 0) {
			addTkError((conservedTk)->line_nr,
					"operation + or - cannot be applied on an array");
			*tk = savedTk;
			return FALSE;
		}
		//check if there was a struct
		if (val->type->typeBase == TB_STRUCT) {
			addTkError((conservedTk)->line_nr,
					"operation + or - cannot be applied on a struct");
			*tk = savedTk;
			return FALSE;
		}
		//save the type
		Type type1 = *(val->type);
		if (!isExprMul(tk, val)) {
			addTkError((*tk)->line_nr, "missing expression after +/-");
			*tk = savedTk;
			return FALSE;
		}
		//check if after ADD|SUB there was an array
		if (val->type->nElements >= 0) {
			addTkError((conservedTk)->line_nr,
					"operation + or - cannot be applied on an array");
			*tk = savedTk;
			return FALSE;
		}
		//check if there was a struct
		if (val->type->typeBase == TB_STRUCT) {
			addTkError((conservedTk)->line_nr,
					"operation + or - cannot be applied on a struct");
			*tk = savedTk;
			return FALSE;
		}
		getArithmeticalType(&type1, val->type, val->type);
		val->isCtVal = 0;
		val->isLVal = 0;
	}
	return TRUE;
}

int isExprRel(Token** tk, RetVal* val) {
//If there is Error then FALSE
	if (isError())
		return FALSE;

//Save the current token in case of returning false
	Token* savedTk = *tk;
//If it is NOT Add then FALSE
	Token* conservedTk = *tk;
	if (!isExprAdd(tk, val)) {
		*tk = savedTk;
		;
		return FALSE;
	}

//(LESS|LESSEQ|GREATER|GREATEREQ + Add)* then TRUE
	while (consume(LESS, tk) || consume(LESSEQ, tk) || consume(GREATER, tk)
			|| consume(GREATEREQ, tk)) {
		//check if there was an array before
		if (val->type->nElements >= 0) {
			addTkError((conservedTk)->line_nr,
					"operation <,<=,>,>= cannot be applied on an array");
			*tk = savedTk;
			return FALSE;
		}
		//check if there was a struct
		if (val->type->typeBase == TB_STRUCT) {
			addTkError((conservedTk)->line_nr,
					"operation <,<=,>,>= cannot be applied on a struct");
			*tk = savedTk;
			return FALSE;
		}
		if (!isExprAdd(tk, val)) {
			addTkError((*tk)->line_nr, "missing expression");
			*tk = savedTk;
			return FALSE;
		}
		//check if there was an array after
		if (val->type->nElements >= 0) {
			addTkError((conservedTk)->line_nr,
					"operation <,<=,>,>= cannot be applied on an array");
			*tk = savedTk;
			return FALSE;
		}
		//check if there was a struct
		if (val->type->typeBase == TB_STRUCT) {
			addTkError((conservedTk)->line_nr,
					"operation <,<=,>,>= cannot be applied on a struct");
			*tk = savedTk;
			return FALSE;
		}
		val->type->typeBase = TB_INT;
		val->type->nElements = -1;
		val->isCtVal = 0;
		val->isLVal = 0;
	}
	return TRUE;
}

int isExprEq(Token** tk, RetVal* val) {
//If there is Error then FALSE
	if (isError())
		return FALSE;

//Save the current token in case of returning false
	Token* savedTk = *tk;
//If it is NOT Rel then FALSE
	Token* conservedTk = *tk;
	if (!isExprRel(tk, val)) {
		*tk = savedTk;
		return FALSE;
	}

//(EQUAL|NOTEQ + Rel)* then TRUE
	while (consume(EQUAL, tk) || consume(NOTEQ, tk)) {
		//check if there was a struct
		if (val->type->typeBase == TB_STRUCT) {
			addTkError((conservedTk)->line_nr,
					"operation == or != cannot be applied on a struct");
			*tk = savedTk;
			return FALSE;
		}
		if (!isExprRel(tk, val)) {
			addTkError((*tk)->line_nr, "missing expression");
			*tk = savedTk;
			return FALSE;
		}
		//check if there was a struct
		if (val->type->typeBase == TB_STRUCT) {
			addTkError((conservedTk)->line_nr,
					"operation == or != cannot be applied on a struct");
			*tk = savedTk;
			return FALSE;
		}
		val->type->typeBase = TB_INT;
		val->type->nElements = -1;
		val->isCtVal = 0;
		val->isLVal = 0;
	}
	return TRUE;
}

int isExprAnd(Token** tk, RetVal* val) {
//If there is Error then FALSE
	if (isError())
		return FALSE;

//Save the current token in case of returning false
	Token* savedTk = *tk;
//If it is NOT Eq then FALSE
	Token* conservedTk = *tk;
	if (!isExprEq(tk, val)) {
		*tk = savedTk;
		return FALSE;
	}

//(AND + Eq)* then TRUE
	while (consume(AND, tk)) {
		//check if there was a struct
		if (val->type->typeBase == TB_STRUCT) {
			addTkError((conservedTk)->line_nr,
					"operation && cannot be applied on a struct");
			*tk = savedTk;
			return FALSE;
		}
		if (!isExprEq(tk, val)) {
			addTkError((*tk)->line_nr, "missing expression");

			*tk = savedTk;
			return FALSE;
		}
		//check if there was a struct
		if (val->type->typeBase == TB_STRUCT) {
			addTkError((conservedTk)->line_nr,
					"operation && cannot be applied on a struct");
			*tk = savedTk;
			return FALSE;
		}
		val->type->typeBase = TB_INT;
		val->type->nElements = -1;
		val->isCtVal = 0;
		val->isLVal = 0;
	}
	return TRUE;
}

int isExprOr(Token** tk, RetVal* val) {
//If there is Error then FALSE
	if (isError())
		return FALSE;

//Save the current token in case of returning false
	Token* savedTk = *tk;
//If it is NOT And then FALSE
	Token* conservedTk = *tk;
	if (!isExprAnd(tk, val)) {
		*tk = savedTk;
		return FALSE;
	}

//(OR + Eq)* then TRUE
	while (consume(OR, tk)) {
		if (val->type->typeBase == TB_STRUCT) {
			addTkError((conservedTk)->line_nr,
					"operation || cannot be applied on a struct");
			*tk = savedTk;
			return FALSE;
		}
		if (!isExprAnd(tk, val)) {
			addTkError((*tk)->line_nr, "missing expression");
			*tk = savedTk;
			return FALSE;
		}
		if (val->type->typeBase == TB_STRUCT) {
			addTkError((conservedTk)->line_nr,
					"operation || cannot be applied on a struct");
			*tk = savedTk;
			return FALSE;
		}
		val->type->typeBase = TB_INT;
		val->type->nElements = -1;
		val->isCtVal = 0;
		val->isLVal = 0;
	}
	return TRUE;
}

int isExprAssign(Token** tk, RetVal* val) {
//If there is Error then FALSE
	if (isError())
		return FALSE;

//Save the current token in case of returning false
	Token* savedTk = *tk;
//Unary ASSIGN Assign | Or
	Token* conservedTk = *tk;
	if (isExprUnary(tk, val)) {
		if (consume(ASSIGN, tk)) {
			if (!(val->isLVal)) {
				addTkError((conservedTk)->line_nr,
						"cannot assign to a non-left value");
				*tk = savedTk;
				return FALSE;
			}
			if (val->type->nElements >= 0) {
				addTkError((conservedTk)->line_nr, "cannot assign to an array");
				*tk = savedTk;
				return FALSE;
			}
			Type type1 = *(val->type);
			if (!isExprAssign(tk, val)) {
				addTkError((*tk)->line_nr, "missing expression");
				*tk = savedTk;
				return FALSE;
			}
			if (val->type->nElements >= 0) {
				addTkError((conservedTk)->line_nr,
						"cannot use assign operation to an array");
				*tk = savedTk;
				return FALSE;
			}
			canCast(&type1, val->type, conservedTk);
			return TRUE;
		} else
			*tk = savedTk;
	}
//If it's Or then TRUE
	if (isExprOr(tk, val))
		return TRUE;
	*tk = savedTk;
	return FALSE;
}

int isExpr(Token** tk, RetVal* val) {
//If there is Error then FALSE
	if (isError())
		return FALSE;

	return isExprAssign(tk, val);
}

Type* isTypeBase(Token** tk) {
//If there is Error then FALSE
	if (isError())
		return FALSE;

//Save the current token in case of returning false
	Token* savedTk = *tk;

	Type* type = (Type*) malloc(sizeof(Type));

//INT | DOUBLE | CHAR | STRUCT ID
	if (consume(INT, tk)) {
		type->typeBase = TB_INT;
		return type;
	}
	if (consume(DOUBLE, tk)) {
		type->typeBase = TB_DOUBLE;
		return type;
	}
	if (consume(CHAR, tk)) {
		type->typeBase = TB_CHAR;
		return type;
	}
	if (consume(STRUCT, tk)) {
		//Save the text of the token
		Token * checkTk = *tk;
		if (consume(ID, tk)) {
			Symbol* sym = findSymbol(symbols, checkTk->text);
			if (sym == NULL) {
				addTkError(checkTk->line_nr, "undefined struct ID");
				free(type);
				return NULL;
			}
			if (sym->cls != CLS_STRUCT) {
				addTkError((*tk)->line_nr, "not a struct");
				free(type);
				return NULL;
			}
			type->s = sym;
			type->typeBase = TB_STRUCT;
			return type;
		}
		*tk = savedTk;
		return NULL;
	}
	return NULL;
}

int isArrayDecl(Token** tk, RetVal* val, Type* type) {

//If there is Error then FALSE
	if (isError())
		return FALSE;
//Save the current token in case of returning false
	Token* savedTk = *tk;
//LBRACKET + expr? + RBRACKET
	if (!consume(LBRACKET, tk)) {
		type->nElements = -1;
		return FALSE;
	}
	Token* conservedTk = *tk;
	if (isExpr(tk, val)) {
		//check if the array size is a constant value
		if (!(val->isCtVal)) {
			addTkError((conservedTk)->line_nr, "array size is not constant");
			*tk = savedTk;
			return FALSE;

		}
		//check if the array size is an int
		if (val->type->typeBase != TB_INT) {
			addTkError((conservedTk)->line_nr, "array size is not an integer");
			*tk = savedTk;
			return FALSE;
		}
		type->nElements = val->ctVal.i;
	} else {
		type->nElements = 0;
	}
	if (consume(RBRACKET, tk))
		return TRUE;
	addTkError((*tk)->line_nr, "missing ] or syntax error");
	*tk = savedTk;
	return FALSE;
}

int isVarDecl(Token** tk, RetVal* val) {

//If there is Error then FALSE
	if (isError())
		return FALSE;

//Save the current token in case of returning false
	Token* savedTk = *tk;
	Type* type;
	type = isTypeBase(tk);
	if (type == NULL)
		return FALSE;
	while (consume(MUL, tk))
		;
//Save the token in case the consume succeeds
	Token* tken = *tk;

	if (!consume(ID, tk)) {
		*tk = savedTk;
		return FALSE;
	}

	if (!addVar(tken, type)) {
		*tk = savedTk;
		return FALSE;
	}
	isArrayDecl(tk, val, type);
	while (consume(COMMA, tk)) {
		tken = *tk;
		if (!consume(ID, tk)) {
			addTkError((*tk)->line_nr, "missing ID");
			*tk = savedTk;
			return FALSE;
		}
		if (!addVar(tken, type)) {
			*tk = savedTk;
			return FALSE;
		}
		isArrayDecl(tk, val, type);
	}
	if (consume(SEMICOLON, tk)) {
		return TRUE;
	}
	addTkError((*tk)->line_nr, "missing ;");
	*tk = savedTk;
	return FALSE;
}

int addVar(Token* tken, Type* type) {
	Symbol* sym;
	if (crtStruct) {
		if (findSymbol(crtStruct->fields, tken->text)) {
			addTkError(tken->line_nr, "redefined identifier");
			return FALSE;
		}
		sym = addSymbol(&(crtStruct->fields), tken->text, CLS_VAR, crtDepth);
	} else if (crtFunc) {
		sym = findSymbol(symbols, tken->text);
		if (sym != NULL) {
			if (sym->depth == crtDepth) {
				addTkError(tken->line_nr, "redefined identifier");
				return FALSE;
			}
		}
		sym = addSymbol(&symbols, tken->text, CLS_VAR, crtDepth);
		sym->mem = MEM_LOCAL;
	} else {
		sym = findSymbol(symbols, tken->text);
		if (sym != NULL) {
			addTkError(tken->line_nr, "redefined identifier");
			return FALSE;
		}
		sym = addSymbol(&symbols, tken->text, CLS_VAR, crtDepth);
		sym->mem = MEM_GLOBAL;
	}
	sym->type = type;
	return TRUE;
}

int isStructDecl(Token** tk, RetVal* val) {
//If there is Error then FALSE
	if (isError())
		return FALSE;
//Save the current token in case of returning false
	Token* savedTk = *tk;
	if (!consume(STRUCT, tk))
		return FALSE;

//If consume ID returns with true, we'll need the ID to create the symbol
	Token* checkTk = *tk;
	if (!consume(ID, tk)) {
		addTkError((*tk)->line_nr, "ID is missing in STRUCT statement");
		*tk = savedTk;

		return FALSE;
	}

//If the next token is not LACC then it could be a declaration of that struct type
	if (!consume(LACC, tk)) {
		//Searching for the id in the Symbol table
		if (!findSymbol(symbols, checkTk->text)) {
			addTkError(checkTk->line_nr, "symbol undefined: struct ID");

			*tk = savedTk;

			return FALSE;
		}
		*tk = savedTk;
		return FALSE;
	}

//If the struct ID was not found add it
	crtStruct = addSymbol(&symbols, checkTk->text, CLS_STRUCT, crtDepth);
//Initialize the fields of the struct
	initSymbols(&(crtStruct->fields));

	while (isVarDecl(tk, val))
		;
	if (!consume(RACC, tk)) {
		addTkError((*tk)->line_nr, "} is missing in STRUCT statement");
		*tk = savedTk;
		return FALSE;
	}
	if (consume(SEMICOLON, tk)) {

		crtStruct = NULL;

		return TRUE;
	}

	crtStruct = NULL;
	addTkError((*tk)->line_nr, "; is missing in STRUCT statement");
	*tk = savedTk;
	return FALSE;
}

int isFuncDecl(Token** tk, RetVal* val) {
//If there is Error then FALSE
	if (isError())
		return FALSE;

//Save the current token in case of returning false
	Token* savedTk = *tk;
//TypeBase MUL | VOID ID LPAR TypeBase ID Array? COMMA (..) RPAR stmCompound
	Type* type = isTypeBase(tk);
	if (type != NULL) {
		if (consume(MUL, tk)) {
			type->nElements = 0;
			while (consume(MUL, tk))
				;
		} else {
			type->nElements = -1;
		}

	} else {
		if (!consume(VOID, tk))
			return FALSE;
		type = (Type*) malloc(sizeof(Type));
		type->typeBase = TB_VOID;
	}
//Saving the token
	Token* checkTk = *tk;

	if (consume(ID, tk)) {
		if (consume(LPAR, tk)) {
			//Check if the function ID is ok
			if (findSymbol(symbols, checkTk->text)) {
				addTkError(checkTk->line_nr, "symbol redefinition in function");
				*tk = savedTk;
				return FALSE;
			}
			//Set all needed values for a function
			crtFunc = addSymbol(&symbols, checkTk->text, CLS_FUNC, crtDepth);
			initSymbols(&(crtFunc->fields));
			crtFunc->type = type;
			crtDepth++;

			if (isFuncArg(tk, val)) {
				while (consume(COMMA, tk)) {
					if (!isFuncArg(tk, val)) {
						addTkError((*tk)->line_nr, "missing argument");
						*tk = savedTk;
						return FALSE;
					}
				}
			}
			if (consume(RPAR, tk)) {
				crtDepth--;
				if (isStmCompound(tk, val)) {
					deleteSymbolsAfter(symbols, crtFunc);
					crtFunc = NULL;
					return TRUE;
				}
				addTkError((*tk)->line_nr, "missing statement");
			} else {
				addTkError((*tk)->line_nr, "missing ) or syntax error");
			}
		}
	}
	*tk = savedTk;
	return FALSE;
}

int isStmCompound(Token** tk, RetVal* val) {
//If there is Error then FALSE
	if (isError())
		return FALSE;

//Save the current token in case of returning false
	Token* savedTk = *tk;

//Save the current position in the symbols list
	Symbol *start = symbols->end[-1];

	if (!consume(LACC, tk))
		return FALSE;
	crtDepth++;
	while (isVarDecl(tk, val) || isStm(tk, val))
		;
	if (consume(RACC, tk)) {
		crtDepth--;
		deleteSymbolsAfter(symbols, start);
		return TRUE;
	}
	addTkError((*tk)->line_nr, "missing } or syntax error");
	*tk = savedTk;
	return FALSE;
}

int isIF(Token** tk, RetVal* val) {
//If there is Error then FALSE
	if (isError())
		return FALSE;

//Save the current token in case of returning false
	Token* savedTk = *tk;
//IF + LPAR + expr + RPAR + stm + ELSE stm
	if (consume(IF, tk)) {
		if (consume(LPAR, tk)) {
			Token* conservedTk = *tk;
			if (isExpr(tk, val)) {
				if (val->type->typeBase == TB_STRUCT) {
					addTkError(conservedTk->line_nr,
							"a structure cannot be logically tested");
					*tk = savedTk;
					return FALSE;
				}
				if (consume(RPAR, tk)) {
					if (isStm(tk, val)) {
						//ELSE + stm not mandatory
						if (consume(ELSE, tk)) {
							if (isStm(tk, val))
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

int isWhile(Token** tk, RetVal* val) {
//If there is Error then FALSE
	if (isError())
		return FALSE;

//Save the current token in case of returning false
	Token* savedTk = *tk;

	if (!consume(WHILE, tk))
		return FALSE;
	if (consume(LPAR, tk)) {
		Token* conservedTk = *tk;
		if (isExpr(tk, val)) {
			if (val->type->typeBase == TB_STRUCT) {
				addTkError(conservedTk->line_nr,
						"a structure cannot be logically tested");
				*tk = savedTk;
				return FALSE;
			}
			if (consume(RPAR, tk)) {
				if (isStm(tk, val)) {
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

int isFor(Token** tk, RetVal* val) {
//If there is Error then FALSE
	if (isError())
		return FALSE;

//Save the current token in case of returning false
	Token* savedTk = *tk;

//for (e;e;e) stm
	if (!consume(FOR, tk))
		return FALSE;
	if (consume(LPAR, tk)) {
		isExpr(tk, val);
		if (consume(SEMICOLON, tk)) {
			Token* conservedTk = *tk;
			if (isExpr(tk, val)) {
				if (val->type->typeBase == TB_STRUCT) {
					addTkError(conservedTk->line_nr,
							"a structure cannot be logically tested");
					*tk = savedTk;
					return FALSE;
				}
			}
			if (consume(SEMICOLON, tk)) {
				isExpr(tk, val);
				if (consume(RPAR, tk)) {
					if (isStm(tk, val)) {
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

int isReturn(Token** tk, RetVal* val) {
//If there is Error then FALSE
	if (isError())
		return FALSE;

//Save the current token in case of returning false
	Token* savedTk = *tk;
	if (!consume(RETURN, tk))
		return FALSE;
	Token* conservedTk = *tk;
	if (isExpr(tk, val)) {
		if (crtFunc->type->typeBase == TB_VOID) {
			addTkError(conservedTk->line_nr,
					"a void function cannot return a value");
			*tk = savedTk;
			return FALSE;
		}
		canCast(crtFunc->type, val->type, conservedTk);
	}
	if (consume(SEMICOLON, tk))
		return TRUE;
	addTkError((*tk)->line_nr, "missing ; after return or syntax error");
	*tk = savedTk;
	return FALSE;
}

int isStm(Token** tk, RetVal* val) {
//If there is Error then FALSE
	if (isError())
		return FALSE;

//Save the current token in case of returning false
	Token* savedTk = *tk;

	if (isStmCompound(tk, val))
		return TRUE;
	if (isIF(tk, val))
		return TRUE;
	if (isWhile(tk, val))
		return TRUE;
	if (isFor(tk, val))
		return TRUE;
	if (isBreak(tk))
		return TRUE;
	if (isReturn(tk, val))
		return TRUE;

//expr? + SEMICOLON
	isExpr(tk, val);
	if (consume(SEMICOLON, tk))
		return TRUE;
	*tk = savedTk;
	return FALSE;
}

Type* isTypeName(Token** tk, RetVal* val) {
//If there is Error then FALSE
	if (isError())
		return NULL;

	Type* type = isTypeBase(tk);
	if (type == NULL)
		return NULL;
	isArrayDecl(tk, val, type);

	return type;
}

int isFuncArg(Token** tk, RetVal* val) {
//If there is Error then FALSE
	if (isError())
		return FALSE;

//Save the current token in case of returning false
	Token* savedTk = *tk;

	Type* type = isTypeBase(tk);
	if (!type)
		return FALSE;

//Save the token
	Token* checkTk = *tk;
	if (consume(ID, tk)) {
		if (isArrayDecl(tk, val, type)) {
			type->nElements = 0;
		} else {
			type->nElements = -1;
		}
		//Add the argument to  the symbols
		Symbol *s = addSymbol(&symbols, checkTk->text, CLS_VAR, crtDepth);
		s->mem = MEM_ARG;
		s->type = type;

		//Add the argument to the function fields
		s = addSymbol(&(crtFunc->fields), checkTk->text, CLS_VAR, crtDepth);
		s->mem = MEM_ARG;
		s->type = type;
		return TRUE;
	}
	addTkError((*tk)->line_nr, "missing ID");
	*tk = savedTk;
	return FALSE;
}

Symbols* isUnit(Token* tk, RetVal val) {
//set Error to FALSE
	tkError.errorMsg = NULL;
	addBasicFunctions(&symbols);

	while (isStructDecl(&tk, &val) || isFuncDecl(&tk, &val)
			|| isVarDecl(&tk, &val))
		;
	if (consume(C_END, &tk))
		return symbols;
	return NULL;
}
