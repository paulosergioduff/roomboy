#include "codegen.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "strbuf.h"
#include "util.h"

/* Transliteração de Latin-1 Supplement para ASCII (equivalente ao NFKD usado
 * pela implementação de referência). Retorna -1 quando o caractere é
 * descartado. */
static int latin1_to_ascii(unsigned cp)
{
    switch (cp) {
    case 0xC0: case 0xC1: case 0xC2: case 0xC3: case 0xC4: case 0xC5:
        return 'A';
    case 0xC7:
        return 'C';
    case 0xC8: case 0xC9: case 0xCA: case 0xCB:
        return 'E';
    case 0xCC: case 0xCD: case 0xCE: case 0xCF:
        return 'I';
    case 0xD1:
        return 'N';
    case 0xD2: case 0xD3: case 0xD4: case 0xD5: case 0xD6:
        return 'O';
    case 0xD9: case 0xDA: case 0xDB: case 0xDC:
        return 'U';
    case 0xDD:
        return 'Y';
    case 0xE0: case 0xE1: case 0xE2: case 0xE3: case 0xE4: case 0xE5:
        return 'a';
    case 0xE7:
        return 'c';
    case 0xE8: case 0xE9: case 0xEA: case 0xEB:
        return 'e';
    case 0xEC: case 0xED: case 0xEE: case 0xEF:
        return 'i';
    case 0xF1:
        return 'n';
    case 0xF2: case 0xF3: case 0xF4: case 0xF5: case 0xF6:
        return 'o';
    case 0xF9: case 0xFA: case 0xFB: case 0xFC:
        return 'u';
    case 0xFD: case 0xFF:
        return 'y';
    default:
        return -1;
    }
}

static int is_c_keyword(const char *value)
{
    static const char *const keywords[] = {
        "auto",     "break",  "case",     "char",   "const",    "continue",
        "default",  "do",     "double",   "else",   "enum",     "extern",
        "float",    "for",    "goto",     "if",     "inline",   "int",
        "long",     "register", "restrict", "return", "short",  "signed",
        "sizeof",   "static", "struct",   "switch", "typedef",  "union",
        "unsigned", "void",   "volatile", "while",  "_Bool",    "_Complex",
        "_Imaginary",
    };
    for (size_t i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
        if (!strcmp(value, keywords[i])) {
            return 1;
        }
    }
    return 0;
}

char *codegen_ident(const char *name)
{
    StrBuf raw;
    sb_init(&raw);

    const char *p = name;
    while (*p) {
        unsigned cp = utf8_decode(&p);
        if (cp < 0x80) {
            sb_putc(&raw, (char)cp);
        } else {
            int mapped = latin1_to_ascii(cp);
            if (mapped >= 0) {
                sb_putc(&raw, (char)mapped);
            }
        }
    }

    for (size_t i = 0; i < raw.len; i++) {
        unsigned char c = (unsigned char)raw.data[i];
        if (!(isalnum(c) || c == '_')) {
            raw.data[i] = '_';
        }
    }

    int needs_prefix =
        raw.len == 0 || isdigit((unsigned char)raw.data[0]) ||
        (raw.len > 0 && is_c_keyword(raw.data));

    StrBuf out;
    sb_init(&out);
    if (needs_prefix) {
        sb_puts(&out, "v_");
    }
    sb_puts(&out, raw.len ? raw.data : "");
    sb_free(&raw);

    return sb_take(&out);
}

static const Variable *find_variable(const Program *program, const char *name)
{
    for (int i = 0; i < program->variable_count; i++) {
        if (ident_ieq(program->variables[i].name, name)) {
            return &program->variables[i];
        }
    }
    return NULL;
}

static const char *c_type_of(LnType type)
{
    switch (type) {
    case TY_INTEIRO:
        return "int";
    case TY_REAL:
        return "double";
    case TY_LOGICO:
        return "int";
    case TY_TEXTO:
        return "char";
    case TY_CARACTER:
        return "char";
    }
    return "int";
}

static void expr_to_c(StrBuf *sb, const Expr *expr)
{
    switch (expr->kind) {
    case EX_NUMBER:
    case EX_STRING:
    case EX_CHAR:
        sb_puts(sb, expr->text);
        break;
    case EX_BOOL:
        sb_puts(sb, expr->boolean ? "1" : "0");
        break;
    case EX_VARIABLE: {
        char *name = codegen_ident(expr->text);
        sb_puts(sb, name);
        free(name);
        break;
    }
    case EX_CALL: {
        char *name = codegen_ident(expr->text);
        sb_printf(sb, "%s(", name);
        free(name);
        for (int i = 0; i < expr->arg_count; i++) {
            if (i) {
                sb_puts(sb, ", ");
            }
            expr_to_c(sb, expr->args[i]);
        }
        sb_putc(sb, ')');
        break;
    }
    case EX_UNARY: {
        sb_putc(sb, '(');
        if (!strcmp(expr->op, "nao")) {
            sb_putc(sb, '!');
        } else {
            sb_puts(sb, expr->op);
        }
        expr_to_c(sb, expr->lhs);
        sb_putc(sb, ')');
        break;
    }
    case EX_BINARY: {
        const char *op = expr->op;
        if (!strcmp(op, "e")) {
            op = "&&";
        } else if (!strcmp(op, "ou")) {
            op = "||";
        } else if (!strcmp(op, "=")) {
            op = "==";
        }
        sb_putc(sb, '(');
        expr_to_c(sb, expr->lhs);
        sb_putc(sb, ' ');
        sb_puts(sb, op);
        sb_putc(sb, ' ');
        expr_to_c(sb, expr->rhs);
        sb_putc(sb, ')');
        break;
    }
    }
}

static void emit_assign_expr(StrBuf *sb, const Program *program,
                             const char *name, const Expr *expr)
{
    const Variable *var = find_variable(program, name);
    LnType type = var ? var->type : TY_INTEIRO;
    char *ident = codegen_ident(name);
    if (type == TY_TEXTO) {
        sb_printf(sb, "snprintf(%s, sizeof(%s), \"%%s\", ", ident, ident);
        expr_to_c(sb, expr);
        sb_putc(sb, ')');
    } else {
        sb_printf(sb, "%s = ", ident);
        expr_to_c(sb, expr);
    }
    free(ident);
}

static void emit_assign(StrBuf *sb, const Program *program, const Stmt *stmt,
                        int indent)
{
    sb_indent(sb, indent);
    emit_assign_expr(sb, program, stmt->name, stmt->expr);
    sb_putc(sb, ';');
}

static void emit_write_arg(StrBuf *sb, const Program *program,
                           const Expr *expr, int indent)
{
    sb_indent(sb, indent);
    switch (expr->kind) {
    case EX_STRING:
        sb_puts(sb, "printf(\"%s\", ");
        sb_puts(sb, expr->text);
        sb_puts(sb, ");");
        break;
    case EX_CHAR:
        sb_puts(sb, "printf(\"%c\", ");
        sb_puts(sb, expr->text);
        sb_puts(sb, ");");
        break;
    case EX_BOOL:
        sb_printf(sb, "printf(\"%s\");",
                  expr->boolean ? "verdadeiro" : "falso");
        break;
    case EX_VARIABLE: {
        const Variable *var = find_variable(program, expr->text);
        LnType type = var ? var->type : TY_INTEIRO;
        char *name = codegen_ident(expr->text);
        if (type == TY_TEXTO) {
            sb_printf(sb, "printf(\"%%s\", %s);", name);
        } else if (type == TY_CARACTER) {
            sb_printf(sb, "printf(\"%%c\", %s);", name);
        } else if (type == TY_REAL) {
            sb_printf(sb, "printf(\"%%g\", %s);", name);
        } else if (type == TY_LOGICO) {
            sb_printf(sb, "printf(\"%%s\", %s ? \"verdadeiro\" : \"falso\");",
                      name);
        } else {
            sb_printf(sb, "printf(\"%%d\", %s);", name);
        }
        free(name);
        break;
    }
    default:
        sb_puts(sb, "printf(\"%g\", (double)(");
        expr_to_c(sb, expr);
        sb_puts(sb, "));");
        break;
    }
}

static void emit_inline_stmt(StrBuf *sb, const Program *program,
                             const Stmt *stmt)
{
    if (!stmt) {
        return;
    }
    switch (stmt->kind) {
    case ST_DECL:
    case ST_ASSIGN:
        if (stmt->expr) {
            emit_assign_expr(sb, program, stmt->name, stmt->expr);
        }
        break;
    default:
        break;
    }
}

static void emit_statement(StrBuf *sb, const Stmt *stmt, const Program *program,
                           int indent)
{
    switch (stmt->kind) {
    case ST_READ: {
        const Variable *var = find_variable(program, stmt->name);
        LnType type = var ? var->type : TY_INTEIRO;
        char *name = codegen_ident(stmt->name);
        if (type == TY_TEXTO) {
            sb_indent(sb, indent);
            sb_puts(sb, "/* Leitura de Texto requer buffer definido pelo "
                        "programa. */\n");
            sb_indent(sb, indent);
            sb_printf(sb, "fgets(%s, sizeof(%s), stdin);", name, name);
        } else {
            const char *fmt = type == TY_REAL ? "%lf" : "%d";
            sb_indent(sb, indent);
            sb_printf(sb, "if (scanf(\"%s\", &%s) != 1) return 1;", fmt, name);
        }
        free(name);
        break;
    }
    case ST_WRITE: {
        if (stmt->arg_count > 0) {
            for (int i = 0; i < stmt->arg_count; i++) {
                emit_write_arg(sb, program, stmt->args[i], indent);
                sb_putc(sb, '\n');
            }
            if (stmt->newline) {
                sb_indent(sb, indent);
                sb_puts(sb, "printf(\"\\n\");");
            }
            break;
        }
        const Expr *expr = stmt->expr;
        if (!expr) {
            if (stmt->newline) {
                sb_indent(sb, indent);
                sb_puts(sb, "printf(\"\\n\");");
            }
            break;
        }
        if (expr->kind == EX_STRING) {
            sb_indent(sb, indent);
            sb_puts(sb, "printf(\"%s\\n\", ");
            sb_puts(sb, expr->text);
            sb_puts(sb, ");");
        } else if (expr->kind == EX_VARIABLE) {
            const Variable *var = find_variable(program, expr->text);
            LnType type = var ? var->type : TY_INTEIRO;
            const char *fmt = "%d";
            if (type == TY_REAL) {
                fmt = "%g";
            } else if (type == TY_TEXTO || type == TY_LOGICO) {
                fmt = "%s";
            }
            char *name = codegen_ident(expr->text);
            sb_indent(sb, indent);
            sb_printf(sb, "printf(\"%s\\n\", ", fmt);
            if (type == TY_LOGICO) {
                sb_printf(sb, "(%s ? \"verdadeiro\" : \"falso\")", name);
            } else {
                sb_puts(sb, name);
            }
            sb_puts(sb, ");");
            free(name);
        } else {
            sb_indent(sb, indent);
            sb_puts(sb, "printf(\"%g\\n\", (double)(");
            expr_to_c(sb, expr);
            sb_puts(sb, "));");
        }
        break;
    }
    case ST_ASSIGN:
        emit_assign(sb, program, stmt, indent);
        break;
    case ST_DECL:
        if (stmt->expr) {
            emit_assign(sb, program, stmt, indent);
        }
        break;
    case ST_IF: {
        sb_indent(sb, indent);
        sb_puts(sb, "if (");
        expr_to_c(sb, stmt->expr);
        sb_puts(sb, ") {\n");
        for (int i = 0; i < stmt->body_count; i++) {
            emit_statement(sb, stmt->body[i], program, indent + 1);
            sb_putc(sb, '\n');
        }
        if (stmt->else_count > 0) {
            sb_indent(sb, indent);
            sb_puts(sb, "} else {\n");
            for (int i = 0; i < stmt->else_count; i++) {
                emit_statement(sb, stmt->else_body[i], program, indent + 1);
                sb_putc(sb, '\n');
            }
        }
        sb_indent(sb, indent);
        sb_putc(sb, '}');
        break;
    }
    case ST_WHILE: {
        sb_indent(sb, indent);
        sb_puts(sb, "while (");
        expr_to_c(sb, stmt->expr);
        sb_puts(sb, ") {\n");
        for (int i = 0; i < stmt->body_count; i++) {
            emit_statement(sb, stmt->body[i], program, indent + 1);
            sb_putc(sb, '\n');
        }
        sb_indent(sb, indent);
        sb_putc(sb, '}');
        break;
    }
    case ST_DO_WHILE: {
        sb_indent(sb, indent);
        sb_puts(sb, "do {\n");
        for (int i = 0; i < stmt->body_count; i++) {
            emit_statement(sb, stmt->body[i], program, indent + 1);
            sb_putc(sb, '\n');
        }
        sb_indent(sb, indent);
        sb_puts(sb, "} while (");
        expr_to_c(sb, stmt->expr);
        sb_puts(sb, ");");
        break;
    }
    case ST_FOR: {
        sb_indent(sb, indent);
        sb_puts(sb, "for (");
        emit_inline_stmt(sb, program, stmt->init);
        sb_puts(sb, "; ");
        if (stmt->expr) {
            expr_to_c(sb, stmt->expr);
        }
        sb_puts(sb, "; ");
        emit_inline_stmt(sb, program, stmt->step);
        sb_puts(sb, ") {\n");
        for (int i = 0; i < stmt->body_count; i++) {
            emit_statement(sb, stmt->body[i], program, indent + 1);
            sb_putc(sb, '\n');
        }
        sb_indent(sb, indent);
        sb_putc(sb, '}');
        break;
    }
    case ST_BREAK:
        sb_indent(sb, indent);
        sb_puts(sb, "break;");
        break;
    case ST_CONTINUE:
        sb_indent(sb, indent);
        sb_puts(sb, "continue;");
        break;
    case ST_RETURN:
        sb_indent(sb, indent);
        sb_puts(sb, "return");
        if (stmt->expr) {
            sb_putc(sb, ' ');
            expr_to_c(sb, stmt->expr);
        }
        sb_putc(sb, ';');
        break;
    }
}

char *codegen_program(const Program *program, int module_only)
{
    StrBuf sb;
    sb_init(&sb);

    sb_puts(&sb, "/* Código gerado automaticamente. Não edite este arquivo "
                 "diretamente. */\n");
    for (int i = 0; i < program->import_count; i++) {
        const char *header = program->imports[i];
        size_t n = strlen(header);
        if ((n >= 2 && !strcmp(header + n - 2, ".h")) ||
            (n >= 4 && !strcmp(header + n - 4, ".hpp"))) {
            sb_printf(&sb, "#include \"%s\"\n", header);
        }
    }
    if (program->import_count) sb_putc(&sb, '\n');
    sb_puts(&sb, "#include <stdio.h>\n\n");

    for (int i = 0; i < program->function_count; i++) {
        const Function *function = &program->functions[i];
        char *name = codegen_ident(function->name);
        sb_printf(&sb, "%sdouble %s(", module_only ? "" : "static ", name);
        free(name);
        if (function->param_count == 0) {
            sb_puts(&sb, "void");
        }
        for (int j = 0; j < function->param_count; j++) {
            char *param = codegen_ident(function->params[j]);
            if (j) {
                sb_puts(&sb, ", ");
            }
            sb_printf(&sb, "double %s", param);
            free(param);
        }
        sb_puts(&sb, ") {\n");
        if (function->return_expr) {
            sb_puts(&sb, "    return ");
            expr_to_c(&sb, function->return_expr);
            sb_puts(&sb, ";\n");
        } else {
            for (int j = 0; j < function->body_count; j++) {
                emit_statement(&sb, function->body[j], program, 1);
                sb_putc(&sb, '\n');
            }
        }
        sb_puts(&sb, "}\n\n");
    }
    if (module_only) return sb_take(&sb);
    sb_puts(&sb, "int main(void) {\n");

    for (int i = 0; i < program->variable_count; i++) {
        const Variable *var = &program->variables[i];
        char *name = codegen_ident(var->name);
        if (var->type == TY_TEXTO) {
            sb_printf(&sb, "    char %s[256] = {0};\n", name);
        } else {
            sb_printf(&sb, "    %s %s = 0;\n", c_type_of(var->type), name);
        }
        free(name);
    }

    if (program->variable_count > 0) {
        sb_putc(&sb, '\n');
    }

    for (int i = 0; i < program->body_count; i++) {
        emit_statement(&sb, program->body[i], program, 1);
        sb_putc(&sb, '\n');
    }

    sb_puts(&sb, "\n    return 0;\n}\n");

    return sb_take(&sb);
}
