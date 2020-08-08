#include"ast.h"

#include<stdint.h>
#include<string.h>

int BINOP_COMMUTATIVE[] = {[BINOP_ADD] = 1, [BINOP_SUB] = 0, [BINOP_MUL] = 1, [BINOP_DIV] = 0};

static int cmp_op_level(BinaryOp one, BinaryOp two) {
	if(one == BINOP_ADD || one == BINOP_SUB) {
		return two == BINOP_ADD || two == BINOP_SUB;
	} else if(one == BINOP_MUL || one == BINOP_DIV) {
		return two == BINOP_MUL || two == BINOP_DIV;
	}
	return 0;
}

AST *ast_expression_optimize(AST *ast) {
	if(ast->nodeKind == AST_EXPRESSION_BINARY_OP) {
		int *constont = NULL;
		
		for(size_t i = 0; i < ast->expressionBinaryOp.amountOfOperands; i++) {
			//~ ast->expressionBinaryOp.operands[i] = ast_expression_optimize(ast->expressionBinaryOp.operands[i]);
			
			if(ast->expressionBinaryOp.operands[i]->nodeKind == AST_EXPRESSION_PRIMITIVE) {
				if(constont) {
					switch(ast->expressionBinaryOp.operators[i - 1]) {
					case BINOP_ADD:
						*constont += ast->expressionBinaryOp.operands[i]->expressionPrimitive.numerator;
						break;
					case BINOP_SUB:
						*constont -= ast->expressionBinaryOp.operands[i]->expressionPrimitive.numerator;
						break;
					case BINOP_MUL:
						*constont *= ast->expressionBinaryOp.operands[i]->expressionPrimitive.numerator;
						break;
					case BINOP_DIV:
						*constont /= ast->expressionBinaryOp.operands[i]->expressionPrimitive.numerator;
						break;
					}
					
					memmove(&ast->expressionBinaryOp.operands[i], &ast->expressionBinaryOp.operands[i + 1], sizeof(AST*) * (ast->expressionBinaryOp.amountOfOperands - i));
					memmove(&ast->expressionBinaryOp.operators[i - 1], &ast->expressionBinaryOp.operators[i], sizeof(BinaryOp) * (ast->expressionBinaryOp.amountOfOperands - i));
					ast->expressionBinaryOp.amountOfOperands--, i--; /* i-- because the index shouldn't change */
				} else {
					constont = &ast->expressionBinaryOp.operands[i]->expressionPrimitive.numerator;
				}
			}
		}
		
		if(ast->expressionBinaryOp.amountOfOperands == 1) {
			return ast->expressionBinaryOp.operands[0];
		}
	}
	
	return ast;
}