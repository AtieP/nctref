#include"ast.h"

#include<stdint.h>

int BINOP_COMMUTATIVE[] = {[BINOP_ADD] = 1, [BINOP_SUB] = 0, [BINOP_MUL] = 1, [BINOP_DIV] = 0};

static int cmp_op_level(BinaryOp one, BinaryOp two) {
	if(one == BINOP_ADD || one == BINOP_SUB) {
		return two == BINOP_ADD || two == BINOP_SUB;
	} else if(one == BINOP_MUL || one == BINOP_DIV) {
		return two == BINOP_MUL || two == BINOP_DIV;
	}
	return 0;
}

void ast_expression_optimize(AST *ast) {
	if(ast->nodeKind == AST_EXPRESSION_BINARY_OP) {
		ast_expression_optimize(ast->expressionBinaryOp.left);
		ast_expression_optimize(ast->expressionBinaryOp.right);
		
		if(ast->expressionBinaryOp.right->nodeKind == AST_EXPRESSION_BINARY_OP && cmp_op_level(ast->expressionBinaryOp.right->expressionBinaryOp.op, ast->expressionBinaryOp.op)) {
			
		}
		
		if(ast->expressionBinaryOp.left->nodeKind == AST_EXPRESSION_PRIMITIVE && ast->expressionBinaryOp.right->nodeKind == AST_EXPRESSION_PRIMITIVE) {
			int64_t l = ast->expressionBinaryOp.left->expressionPrimitive.numerator / ast->expressionBinaryOp.left->expressionPrimitive.denominator;
			int64_t r = ast->expressionBinaryOp.right->expressionPrimitive.numerator / ast->expressionBinaryOp.right->expressionPrimitive.denominator;
			
			int64_t x;
			if(ast->expressionBinaryOp.op == BINOP_ADD) {
				x = l + r;
			} else if(ast->expressionBinaryOp.op == BINOP_SUB) {
				x = l - r;
			} else if(ast->expressionBinaryOp.op == BINOP_MUL) {
				x = l * r;
			} else if(ast->expressionBinaryOp.op == BINOP_DIV) {
				x = l / r;
			}
			
			ast->nodeKind = AST_EXPRESSION_PRIMITIVE;
			ast->expressionPrimitive.numerator = x;
			ast->expressionPrimitive.denominator = 1;
		}
	}
}