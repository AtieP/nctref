#include"ast.h"

#include<stdint.h>
#include<string.h>
#include<stdlib.h>

int BINOP_COMMUTATIVE[] = {[BINOP_ADD] = 1, [BINOP_SUB] = 0, [BINOP_MUL] = 1, [BINOP_DIV] = 0};

void ast_expression_binop_removeoperand(AST *ast, size_t i) {
	free(ast->expressionBinaryOp.operands[i]); /* TODO: free the children! */
	
	if(i != ast->expressionBinaryOp.amountOfOperands - 1) {
		memmove(&ast->expressionBinaryOp.operands[i], &ast->expressionBinaryOp.operands[i + 1], sizeof(AST*) * (ast->expressionBinaryOp.amountOfOperands - i - 1));
	}
	
	if(i != 0) {
		memmove(&ast->expressionBinaryOp.operators[i - 1], &ast->expressionBinaryOp.operators[i], sizeof(BinaryOp) * (ast->expressionBinaryOp.amountOfOperands - i));
	}
	
	ast->expressionBinaryOp.amountOfOperands--;
}

AST *ast_expression_optimize(AST *ast) {
	if(ast->nodeKind == AST_EXPRESSION_BINARY_OP) {
		int *constont = NULL;
		
		for(size_t i = 0; i < ast->expressionBinaryOp.amountOfOperands; i++) {
			//~ ast->expressionBinaryOp.operands[i] = ast_expression_optimize(ast->expressionBinaryOp.operands[i]);
			
			for(size_t j = 0; j < ast->expressionBinaryOp.amountOfOperands; j++) {
				if(i == j) continue;
				
				if(((i == 0 || ast->expressionBinaryOp.operators[i - 1] == BINOP_ADD) && (j != 0 && ast->expressionBinaryOp.operators[j - 1] == BINOP_SUB))
					|| ((j == 0 || ast->expressionBinaryOp.operators[j - 1] == BINOP_ADD) && (i != 0 && ast->expressionBinaryOp.operators[i - 1] == BINOP_SUB))) {
					
					ast_expression_binop_removeoperand(ast, i);
					ast_expression_binop_removeoperand(ast, (j > i) ? (j - 1) : j);
				}
			}
			
			if(ast->expressionBinaryOp.amountOfOperands == 0) {
				free(ast);
				ast = malloc(sizeof(ASTExpressionPrimitive));
				ast->nodeKind = AST_EXPRESSION_PRIMITIVE;
				ast->expression.type = (Type*) primitive_parse("u8");
				ast->expression.constantType = EXPRESSION_CONSTANT_FALSY;
				ast->expressionPrimitive.numerator = 0;
				ast->expressionPrimitive.denominator = 1;
				return ast;
			}
			
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
					
					ast_expression_binop_removeoperand(ast, i);
					i--; /* i-- because the index shouldn't change. */
				} else {
					constont = &ast->expressionBinaryOp.operands[i]->expressionPrimitive.numerator;
				}
			}
		}
		
		if(ast->expressionBinaryOp.amountOfOperands == 1) {
			AST *ret = ast->expressionBinaryOp.operands[0];
			free(ast);
			return ret;
		}
	}
	
	return ast;
}

int ast_expression_equal(AST *a, AST *b) {
	if(a->nodeKind != b->nodeKind) return 0;
	
	if(a->nodeKind == AST_EXPRESSION_PRIMITIVE) {
		return a->expressionPrimitive.numerator == b->expressionPrimitive.numerator && a->expressionPrimitive.denominator == b->expressionPrimitive.denominator;
	} else if(a->nodeKind == AST_EXPRESSION_VAR) {
		return a->expressionVar.thing == b->expressionVar.thing;
	} else if(a->nodeKind == AST_EXPRESSION_UNARY_OP) {
		return a->expressionUnaryOp.operator == b->expressionUnaryOp.operator && ast_expression_equal(a->expressionUnaryOp.chaiuld, b->expressionUnaryOp.chaiuld);
	}
	
	return 0;
}