#include "ast.h"

#include <stdlib.h>

const char *ln_type_name(LnType type)
{
    switch (type) {
    case TY_INTEIRO:
        return "Inteiro";
    case TY_REAL:
        return "Real";
    case TY_LOGICO:
        return "Logico";
    case TY_TEXTO:
        return "Texto";
    case TY_CARACTER:
        return "Caracter";
    }
    return "Desconhecido";
}

static void expr_free(Expr *expr)
{
    if (!expr) {
        return;
    }
    free(expr->text);
    free(expr->op);
    expr_free(expr->lhs);
    expr_free(expr->rhs);
    free(expr);
}

static void stmt_free(Stmt *stmt)
{
    if (!stmt) {
        return;
    }
    free(stmt->name);
    expr_free(stmt->expr);
    for (int i = 0; i < stmt->arg_count; i++) {
        expr_free(stmt->args[i]);
    }
    free(stmt->args);
    for (int i = 0; i < stmt->body_count; i++) {
        stmt_free(stmt->body[i]);
    }
    free(stmt->body);
    for (int i = 0; i < stmt->else_count; i++) {
        stmt_free(stmt->else_body[i]);
    }
    free(stmt->else_body);
    stmt_free(stmt->init);
    stmt_free(stmt->step);
    free(stmt);
}

void program_free(Program *program)
{
    if (!program) {
        return;
    }
    free(program->name);
    for (int i = 0; i < program->import_count; i++) {
        free(program->imports[i]);
    }
    free(program->imports);
    for (int i = 0; i < program->variable_count; i++) {
        free(program->variables[i].name);
    }
    free(program->variables);
    for (int i = 0; i < program->body_count; i++) {
        stmt_free(program->body[i]);
    }
    free(program->body);
    program->name = NULL;
    program->imports = NULL;
    program->import_count = 0;
    program->variables = NULL;
    program->variable_count = 0;
    program->body = NULL;
    program->body_count = 0;
}
