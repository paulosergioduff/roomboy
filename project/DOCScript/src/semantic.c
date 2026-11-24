#include "semantic.h"

#include <stdlib.h>
#include <string.h>

#include "codegen.h"
#include "util.h"

static int variable_index(const Program *program, const char *name)
{
    for (int i = 0; i < program->variable_count; i++) {
        if (ident_ieq(program->variables[i].name, name)) {
            return i;
        }
    }
    return -1;
}

static void check_expr(const Program *program, const Expr *expr,
                       unsigned char *used, const char *file, int warnings,
                       FILE *out)
{
    if (!expr) {
        return;
    }
    switch (expr->kind) {
    case EX_VARIABLE: {
        int index = variable_index(program, expr->text);
        if (index >= 0) {
            used[index] = 1;
        }
        break;
    }
    case EX_UNARY:
        check_expr(program, expr->lhs, used, file, warnings, out);
        break;
    case EX_CALL:
        for (int i = 0; i < expr->arg_count; i++) {
            check_expr(program, expr->args[i], used, file, warnings, out);
        }
        break;
    case EX_BINARY:
        if (warnings &&
            (!strcmp(expr->op, "=") || !strcmp(expr->op, "!=")) &&
            (expr->lhs->kind == EX_STRING || expr->rhs->kind == EX_STRING)) {
            fprintf(out,
                    "AVISO: comparar textos com '%s' compara endereços em C, "
                    "não o conteúdo (%s:%d:%d)\n",
                    expr->op, file, expr->line, expr->col);
        }
        check_expr(program, expr->lhs, used, file, warnings, out);
        check_expr(program, expr->rhs, used, file, warnings, out);
        break;
    default:
        break;
    }
}

static void check_stmt(const Program *program, const Stmt *stmt,
                       unsigned char *used, const char *file, int warnings,
                       FILE *out)
{
    switch (stmt->kind) {
    case ST_READ: {
        int index = variable_index(program, stmt->name);
        if (index >= 0) {
            used[index] = 1;
            if (warnings && program->variables[index].type == TY_TEXTO) {
                fprintf(out,
                        "AVISO: leitura de Texto ainda é parcial e usa "
                        "fgets com buffer de 256 bytes (%s:%d:%d)\n",
                        file, program->variables[index].line,
                        program->variables[index].col);
            }
        }
        break;
    }
    case ST_WRITE:
        if (stmt->arg_count > 0) {
            for (int i = 0; i < stmt->arg_count; i++) {
                check_expr(program, stmt->args[i], used, file, warnings, out);
            }
        } else {
            check_expr(program, stmt->expr, used, file, warnings, out);
        }
        break;
    case ST_ASSIGN:
    case ST_DECL: {
        if (stmt->kind == ST_ASSIGN) {
            int index = variable_index(program, stmt->name);
            if (index >= 0) {
                used[index] = 1;
            }
        }
        check_expr(program, stmt->expr, used, file, warnings, out);
        break;
    }
    case ST_IF:
    case ST_WHILE:
    case ST_DO_WHILE:
        check_expr(program, stmt->expr, used, file, warnings, out);
        for (int i = 0; i < stmt->body_count; i++) {
            check_stmt(program, stmt->body[i], used, file, warnings, out);
        }
        for (int i = 0; i < stmt->else_count; i++) {
            check_stmt(program, stmt->else_body[i], used, file, warnings, out);
        }
        break;
    case ST_FOR:
        check_stmt(program, stmt->init, used, file, warnings, out);
        check_expr(program, stmt->expr, used, file, warnings, out);
        check_stmt(program, stmt->step, used, file, warnings, out);
        for (int i = 0; i < stmt->body_count; i++) {
            check_stmt(program, stmt->body[i], used, file, warnings, out);
        }
        break;
    case ST_RETURN:
        check_expr(program, stmt->expr, used, file, warnings, out);
        break;
    case ST_BREAK:
    case ST_CONTINUE:
        break;
    }
}

void semantic_check(const Program *program, const char *file, int warnings,
                    FILE *out)
{
    if (!warnings || program->variable_count == 0) {
        return;
    }
    unsigned char *used = xcalloc((size_t)program->variable_count, 1);
    for (int i = 0; i < program->body_count; i++) {
        check_stmt(program, program->body[i], used, file, warnings, out);
    }
    for (int i = 0; i < program->variable_count; i++) {
        if (!used[i]) {
            fprintf(out,
                    "AVISO: variável '%s' declarada mas não utilizada "
                    "(%s:%d:%d)\n",
                    program->variables[i].name, file, program->variables[i].line,
                    program->variables[i].col);
        }
    }

    /* Nomes distintos em DOC Script podem convergir para o mesmo
     * identificador C após a transliteração. */
    for (int i = 0; i < program->variable_count; i++) {
        char *left = codegen_ident(program->variables[i].name);
        for (int j = i + 1; j < program->variable_count; j++) {
            char *right = codegen_ident(program->variables[j].name);
            if (!strcmp(left, right)) {
                fprintf(out,
                        "AVISO: as variáveis '%s' e '%s' geram o mesmo "
                        "identificador C '%s' (%s:%d:%d)\n",
                        program->variables[i].name,
                        program->variables[j].name, left, file,
                        program->variables[j].line,
                        program->variables[j].col);
            }
            free(right);
        }
        free(left);
    }

    free(used);
}
