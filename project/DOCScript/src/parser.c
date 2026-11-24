#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

typedef struct {
    const TokenList *tokens;
    int i;
    Variable *vars;
    int var_count;
    int var_cap;
    Program *program;
    Diagnostic *diag;
} Parser;

static void parser_expr_free(Expr *expr)
{
    if (!expr) {
        return;
    }
    free(expr->text);
    free(expr->op);
    parser_expr_free(expr->lhs);
    parser_expr_free(expr->rhs);
    for (int i = 0; i < expr->arg_count; i++) {
        parser_expr_free(expr->args[i]);
    }
    free(expr->args);
    free(expr);
}

static char *strip_quotes(const char *value);

static const Token *cur(const Parser *p)
{
    return &p->tokens->items[p->i];
}

static const Token *advance(Parser *p)
{
    const Token *token = cur(p);
    if (token->kind != TK_EOF) {
        p->i++;
    }
    return token;
}

static void describe_token(const Token *token, char *buf, size_t size)
{
    if (token->kind == TK_EOF) {
        snprintf(buf, size, "fim do arquivo");
        return;
    }
    if (token->kind == TK_STRING) {
        snprintf(buf, size, "texto %s", token->value);
        return;
    }
    snprintf(buf, size, "'%s'", token->value ? token->value : "");
}

static bool fail_at(Parser *p, DiagCategory cat, const Token *token,
                    const char *message, const char *expected,
                    const char *hint)
{
    char found[160];
    describe_token(token, found, sizeof(found));
    diag_set_full(p->diag, cat, token->line, token->col, found, expected,
                  message, hint);
    return false;
}

static bool fail(Parser *p, DiagCategory cat, const char *message,
                 const char *expected, const char *hint)
{
    return fail_at(p, cat, cur(p), message, expected, hint);
}

static bool accept_kw(Parser *p, const char *keyword)
{
    if (token_is_kw(cur(p), keyword)) {
        advance(p);
        return true;
    }
    return false;
}

static bool expect_kw(Parser *p, const char *keyword)
{
    if (accept_kw(p, keyword)) {
        return true;
    }
    char message[256];
    snprintf(message, sizeof(message), "Esperado '%s'.", keyword);
    return fail(p, DIAG_SINTATICO, message, keyword, NULL);
}

static const Token *accept(Parser *p, TokenKind kind)
{
    if (cur(p)->kind == kind) {
        return advance(p);
    }
    return NULL;
}

static const Token *expect(Parser *p, TokenKind kind, const char *description,
                           const char *hint)
{
    const Token *token = accept(p, kind);
    if (token) {
        return token;
    }
    char message[256];
    snprintf(message, sizeof(message), "Esperado %s.", description);
    fail(p, DIAG_SINTATICO, message, description, hint);
    return NULL;
}

static bool expect_dot(Parser *p)
{
    return expect(p, TK_DOT, "ponto final '.'",
                  "Toda instrução e declaração termina com ponto final, não com "
                  "ponto e vírgula.")
               ? true
               : false;
}

static Variable *find_var(Parser *p, const char *name)
{
    for (int i = 0; i < p->var_count; i++) {
        if (ident_ieq(p->vars[i].name, name)) {
            return &p->vars[i];
        }
    }
    return NULL;
}

static Function *find_function(Parser *p, const char *name)
{
    for (int i = 0; i < p->program->function_count; i++) {
        if (ident_ieq(p->program->functions[i].name, name)) {
            return &p->program->functions[i];
        }
    }
    return NULL;
}

static void add_var(Parser *p, const char *name, LnType type, int line, int col)
{
    if (p->var_count == p->var_cap) {
        p->var_cap = p->var_cap ? p->var_cap * 2 : 16;
        p->vars = xrealloc(p->vars, (size_t)p->var_cap * sizeof(Variable));
    }
    Variable *var = &p->vars[p->var_count++];
    var->name = xstrdup(name);
    var->type = type;
    var->line = line;
    var->col = col;
}

static bool require_variable(Parser *p, const Token *name)
{
    if (find_var(p, name->value)) {
        return true;
    }
    char message[512];
    snprintf(message, sizeof(message), "Variável '%s' não foi declarada.",
             name->value);
    return fail_at(p, DIAG_SEMANTICO, name, message,
                   "variável declarada na seção Variaveis",
                   "Declare a variável na seção Variaveis antes de usá-la.");
}

static Expr *new_expr_at(ExprKind kind, const Token *token)
{
    Expr *expr = xcalloc(1, sizeof(Expr));
    expr->kind = kind;
    expr->line = token->line;
    expr->col = token->col;
    return expr;
}

static void ascii_lower(char *text)
{
    for (; *text; text++) {
        if (*text >= 'A' && *text <= 'Z') {
            *text = (char)(*text + 32);
        }
    }
}

static int precedence_of(const Token *token)
{
    if (token->kind != TK_OP && token->kind != TK_KW) {
        return 0;
    }
    const char *value = token->value;
    if (ascii_ieq(value, "ou")) {
        return 1;
    }
    if (ascii_ieq(value, "e")) {
        return 2;
    }
    if (!strcmp(value, "=") || !strcmp(value, "==") || !strcmp(value, "!=") ||
        !strcmp(value, ">") || !strcmp(value, "<") || !strcmp(value, ">=") ||
        !strcmp(value, "<=")) {
        return 3;
    }
    if (!strcmp(value, "+") || !strcmp(value, "-")) {
        return 4;
    }
    if (!strcmp(value, "*") || !strcmp(value, "/") || !strcmp(value, "%")) {
        return 5;
    }
    return 0;
}

static bool parse_expression(Parser *p, int min_prec, Expr **out)
{
    const Token *token = cur(p);
    Expr *left = NULL;

    if (token->kind == TK_OP &&
        (!strcmp(token->value, "-") || !strcmp(token->value, "+"))) {
        advance(p);
        Expr *operand = NULL;
        if (!parse_expression(p, 6, &operand)) {
            return false;
        }
        left = new_expr_at(EX_UNARY, token);
        left->op = xstrdup(token->value);
        left->lhs = operand;
    } else if (accept_kw(p, "nao")) {
        Expr *operand = NULL;
        if (!parse_expression(p, 6, &operand)) {
            return false;
        }
        left = new_expr_at(EX_UNARY, token);
        left->op = xstrdup("nao");
        left->lhs = operand;
    } else if (token->kind == TK_NUMBER) {
        advance(p);
        left = new_expr_at(EX_NUMBER, token);
        left->text = xstrdup(token->value);
    } else if (token->kind == TK_STRING) {
        advance(p);
        left = new_expr_at(EX_STRING, token);
        left->text = xstrdup(token->value);
    } else if (token->kind == TK_CHAR) {
        advance(p);
        left = new_expr_at(EX_CHAR, token);
        left->text = xstrdup(token->value);
    } else if (accept_kw(p, "verdadeiro")) {
        left = new_expr_at(EX_BOOL, token);
        left->boolean = 1;
    } else if (accept_kw(p, "falso")) {
        left = new_expr_at(EX_BOOL, token);
        left->boolean = 0;
    } else if (token->kind == TK_IDENT) {
        advance(p);
        if (accept_kw(p, "recebe")) {
            Function *function = find_function(p, token->value);
            if (!function) {
                char message[256];
                snprintf(message, sizeof(message),
                         "Função '%s' não foi declarada.", token->value);
                return fail_at(p, DIAG_SEMANTICO, token, message,
                               "nome de função declarada com Recebe",
                               "Declare a função antes de chamá-la.");
            }
            left = new_expr_at(EX_CALL, token);
            left->text = xstrdup(token->value);
            int cap = 0;
            do {
                Expr *arg = NULL;
                if (!parse_expression(p, 3, &arg)) {
                    return false;
                }
                if (left->arg_count == cap) {
                    cap = cap ? cap * 2 : 4;
                    left->args = xrealloc(left->args,
                                          (size_t)cap * sizeof(Expr *));
                }
                left->args[left->arg_count++] = arg;
                if (accept(p, TK_COMMA)) {
                    continue;
                }
                if (accept_kw(p, "e")) {
                    continue;
                }
                break;
            } while (true);
            if (left->arg_count != function->param_count) {
                char message[256];
                snprintf(message, sizeof(message),
                         "Função '%s' recebe %d valor(es), mas recebeu %d.",
                         function->name, function->param_count,
                         left->arg_count);
                return fail_at(p, DIAG_SEMANTICO, token, message,
                               "quantidade de valores declarada em Recebe",
                               "Passe um valor para cada parâmetro.");
            }
        } else {
        if (!require_variable(p, token)) {
            return false;
        }
        left = new_expr_at(EX_VARIABLE, token);
        left->text = xstrdup(token->value);
        }
    } else {
        return fail(p, DIAG_SINTATICO, "Expressão esperada.",
                    "número, texto, variável, verdadeiro/falso ou expressão "
                    "entre operadores",
                    "Exemplo: idade >= 18 e renda > 1000.");
    }

    for (;;) {
        const Token *op_token = cur(p);
        int prec = precedence_of(op_token);
        if (prec == 0 || prec < min_prec) {
            break;
        }
        advance(p);
        Expr *right = NULL;
        if (!parse_expression(p, prec + 1, &right)) {
            return false;
        }
        Expr *binary = new_expr_at(EX_BINARY, op_token);
        binary->op = xstrdup(op_token->value);
        ascii_lower(binary->op);
        binary->lhs = left;
        binary->rhs = right;
        left = binary;
    }

    *out = left;
    return true;
}

static Stmt *new_stmt(StmtKind kind)
{
    Stmt *stmt = xcalloc(1, sizeof(Stmt));
    stmt->kind = kind;
    return stmt;
}

static Stmt *new_stmt_at(StmtKind kind, const Token *token)
{
    Stmt *stmt = new_stmt(kind);
    if (token) {
        stmt->line = token->line;
        stmt->col = token->col;
    }
    return stmt;
}

typedef struct {
    Stmt **items;
    int count;
    int cap;
} StmtVec;

static void stmt_vec_push(StmtVec *vec, Stmt *stmt)
{
    if (vec->count == vec->cap) {
        vec->cap = vec->cap ? vec->cap * 2 : 16;
        vec->items = xrealloc(vec->items, (size_t)vec->cap * sizeof(Stmt *));
    }
    vec->items[vec->count++] = stmt;
}

static Stmt **stmt_vec_take(StmtVec *vec, int *count)
{
    *count = vec->count;
    Stmt **items = vec->items;
    vec->items = NULL;
    vec->count = 0;
    vec->cap = 0;
    return items;
}

static const Token *accept_assignment(Parser *p)
{
    const Token *token = cur(p);
    if (token->kind == TK_ASSIGN) {
        return advance(p);
    }
    if (token->kind == TK_OP && !strcmp(token->value, "=")) {
        return advance(p);
    }
    return NULL;
}

static Expr *new_variable_expr(const Token *name)
{
    Expr *expr = new_expr_at(EX_VARIABLE, name);
    expr->text = xstrdup(name->value);
    return expr;
}

static Expr *new_number_expr(const Token *at, const char *text)
{
    Expr *expr = new_expr_at(EX_NUMBER, at);
    expr->text = xstrdup(text);
    return expr;
}

static Expr *new_binary_expr(const Token *at, const char *op, Expr *lhs,
                             Expr *rhs)
{
    Expr *expr = new_expr_at(EX_BINARY, at);
    expr->op = xstrdup(op);
    expr->lhs = lhs;
    expr->rhs = rhs;
    return expr;
}

static bool is_type_kw(const Token *token)
{
    return token->kind == TK_KW &&
           (token_is_kw(token, "inteiro") || token_is_kw(token, "real") ||
            token_is_kw(token, "logico") || token_is_kw(token, "texto") ||
            token_is_kw(token, "cadeia") || token_is_kw(token, "caracter"));
}

static bool parse_new_type(Parser *p, LnType *out)
{
    const Token *token = cur(p);
    if (token->kind == TK_KW) {
        if (token_is_kw(token, "inteiro")) {
            advance(p);
            *out = TY_INTEIRO;
            return true;
        }
        if (token_is_kw(token, "real")) {
            advance(p);
            *out = TY_REAL;
            return true;
        }
        if (token_is_kw(token, "logico")) {
            advance(p);
            *out = TY_LOGICO;
            return true;
        }
        if (token_is_kw(token, "texto") || token_is_kw(token, "cadeia")) {
            advance(p);
            *out = TY_TEXTO;
            return true;
        }
        if (token_is_kw(token, "caracter")) {
            advance(p);
            *out = TY_CARACTER;
            return true;
        }
    }
    return fail(p, DIAG_SEMANTICO,
                "Tipo esperado depois da palavra-chave de declaração.",
                "inteiro, real, logico, cadeia ou caracter",
                "Exemplo: inteiro idade = 18.");
}

static bool expect_lbrace(Parser *p)
{
    return expect(p, TK_LBRACE, "'{' para abrir o bloco",
                  "Exemplo: se idade >= 18 { ... }")
               ? true
               : false;
}

static bool expect_rbrace(Parser *p)
{
    return expect(p, TK_RBRACE, "'}' para fechar o bloco",
                  "Feche o bloco aberto com '{'.")
               ? true
               : false;
}

static void skip_separators(Parser *p)
{
    while (cur(p)->kind == TK_DOT || cur(p)->kind == TK_SEMI) {
        advance(p);
    }
}

static bool check_duplicate_var(Parser *p, const Token *name)
{
    if (!find_var(p, name->value)) {
        return true;
    }
    char message[512];
    snprintf(message, sizeof(message),
             "Variável '%s' declarada mais de uma vez.", name->value);
    return fail_at(p, DIAG_SEMANTICO, name, message,
                   "nome de variável ainda não declarado",
                   "Escolha outro nome ou remova a declaração duplicada.");
}

static bool parse_new_assignment(Parser *p, StmtVec *out, int require_separator)
{
    const Token *name = expect(p, TK_IDENT, "nome de variável",
                               "Exemplo: idade = 18.");
    if (!name || !require_variable(p, name)) {
        return false;
    }

    Stmt *stmt = new_stmt_at(ST_ASSIGN, name);
    stmt->name = xstrdup(name->value);

    if (cur(p)->kind == TK_OP &&
        (!strcmp(cur(p)->value, "++") || !strcmp(cur(p)->value, "--"))) {
        const Token *op = advance(p);
        char symbol[2] = {op->value[0], '\0'};
        stmt->expr = new_binary_expr(op, symbol, new_variable_expr(name),
                                     new_number_expr(op, "1"));
    } else {
        const Token *assign = accept_assignment(p);
        if (!assign) {
            free(stmt->name);
            free(stmt);
            return fail(p, DIAG_SINTATICO,
                        "Esperado operador de atribuição depois do nome da "
                        "variável.",
                        "'=', '<-', '+=', '-=', '*=', '/=', '++' ou '--'",
                        "Exemplo: idade = idade + 1.");
        }
        Expr *value = NULL;
        if (!parse_expression(p, 1, &value)) {
            free(stmt->name);
            free(stmt);
            return false;
        }
        if (!strcmp(assign->value, "<-") || !strcmp(assign->value, "=")) {
            stmt->expr = value;
        } else {
            char op[2] = {assign->value[0], '\0'};
            stmt->expr = new_binary_expr(assign, op, new_variable_expr(name),
                                         value);
        }
    }

    stmt_vec_push(out, stmt);
    if (require_separator) {
        skip_separators(p);
    }
    return true;
}

static bool parse_new_declaration(Parser *p, StmtVec *out)
{
    LnType type;
    if (!parse_new_type(p, &type)) {
        return false;
    }

    do {
        const Token *name =
            expect(p, TK_IDENT, "nome de variável",
                   "Exemplo: inteiro idade = 18.");
        if (!name || !check_duplicate_var(p, name)) {
            return false;
        }
        Expr *init = NULL;
        if (accept_assignment(p) && !parse_expression(p, 1, &init)) {
            return false;
        }
        add_var(p, name->value, type, name->line, name->col);

        Stmt *stmt = new_stmt_at(ST_DECL, name);
        stmt->name = xstrdup(name->value);
        stmt->type = type;
        stmt->expr = init;
        stmt_vec_push(out, stmt);
    } while (accept(p, TK_COMMA));

    skip_separators(p);
    return true;
}

static Stmt *parse_for_declaration(Parser *p)
{
    LnType type;
    if (!parse_new_type(p, &type)) {
        return NULL;
    }
    const Token *name =
        expect(p, TK_IDENT, "nome de variável do laço Para",
               "Exemplo: para inteiro i = 0; i < 10; i++ { ... }");
    if (!name || !check_duplicate_var(p, name)) {
        return NULL;
    }
    Expr *init = NULL;
    if (accept_assignment(p) && !parse_expression(p, 1, &init)) {
        return NULL;
    }
    add_var(p, name->value, type, name->line, name->col);

    Stmt *stmt = new_stmt_at(ST_DECL, name);
    stmt->name = xstrdup(name->value);
    stmt->type = type;
    stmt->expr = init;
    return stmt;
}

static bool parse_new_block(Parser *p, StmtVec *out);

static bool parse_new_if_tail(Parser *p, const Token *kw, Stmt **out)
{
    Expr *condition = NULL;
    if (!parse_expression(p, 1, &condition) || !expect_lbrace(p)) {
        return false;
    }

    StmtVec yes = {0};
    if (!parse_new_block(p, &yes) || !expect_rbrace(p)) {
        return false;
    }

    StmtVec no = {0};
    if (accept_kw(p, "senao")) {
        if (token_is_kw(cur(p), "se")) {
            const Token *nested_kw = advance(p);
            Stmt *nested = NULL;
            if (!parse_new_if_tail(p, nested_kw, &nested)) {
                return false;
            }
            stmt_vec_push(&no, nested);
        } else {
            if (!expect_lbrace(p) || !parse_new_block(p, &no) ||
                !expect_rbrace(p)) {
                return false;
            }
        }
    }

    Stmt *stmt = new_stmt_at(ST_IF, kw);
    stmt->expr = condition;
    stmt->body = stmt_vec_take(&yes, &stmt->body_count);
    stmt->else_body = stmt_vec_take(&no, &stmt->else_count);
    *out = stmt;
    return true;
}

static bool is_statement_end(const Token *token)
{
    return token->kind == TK_RBRACE || token->kind == TK_EOF ||
           token->kind == TK_DOT || token->kind == TK_SEMI;
}

static bool parse_new_statement(Parser *p, StmtVec *out)
{
    if (accept_kw(p, "leia")) {
        do {
            const Token *name = expect(p, TK_IDENT, "nome de variável",
                                       "Exemplo: leia idade.");
            if (!name || !require_variable(p, name)) {
                return false;
            }
            Stmt *stmt = new_stmt_at(ST_READ, name);
            stmt->name = xstrdup(name->value);
            stmt_vec_push(out, stmt);
        } while (accept(p, TK_COMMA));
        skip_separators(p);
        return true;
    }

    if (token_is_kw(cur(p), "escreva") || token_is_kw(cur(p), "escreval")) {
        bool newline = token_is_kw(cur(p), "escreval");
        const Token *kw = advance(p);
        Stmt *stmt = new_stmt_at(ST_WRITE, kw);
        stmt->newline = newline ? 1 : 0;
        int cap = 0;
        if (!is_statement_end(cur(p))) {
            do {
                Expr *arg = NULL;
                if (!parse_expression(p, 1, &arg)) {
                    return false;
                }
                if (stmt->arg_count == cap) {
                    cap = cap ? cap * 2 : 4;
                    stmt->args = xrealloc(stmt->args,
                                          (size_t)cap * sizeof(Expr *));
                }
                stmt->args[stmt->arg_count++] = arg;
            } while (accept(p, TK_COMMA));
        }
        stmt_vec_push(out, stmt);
        skip_separators(p);
        return true;
    }

    if (token_is_kw(cur(p), "se")) {
        const Token *kw = advance(p);
        Stmt *stmt = NULL;
        if (!parse_new_if_tail(p, kw, &stmt)) {
            return false;
        }
        stmt_vec_push(out, stmt);
        skip_separators(p);
        return true;
    }

    if (token_is_kw(cur(p), "enquanto")) {
        const Token *kw = advance(p);
        Expr *condition = NULL;
        if (!parse_expression(p, 1, &condition) || !expect_lbrace(p)) {
            return false;
        }
        StmtVec body = {0};
        if (!parse_new_block(p, &body) || !expect_rbrace(p)) {
            return false;
        }
        Stmt *stmt = new_stmt_at(ST_WHILE, kw);
        stmt->expr = condition;
        stmt->body = stmt_vec_take(&body, &stmt->body_count);
        stmt_vec_push(out, stmt);
        skip_separators(p);
        return true;
    }

    if (token_is_kw(cur(p), "faca")) {
        const Token *kw = advance(p);
        if (!expect_lbrace(p)) {
            return false;
        }
        StmtVec body = {0};
        if (!parse_new_block(p, &body) || !expect_rbrace(p) ||
            !expect_kw(p, "enquanto")) {
            return false;
        }
        Expr *condition = NULL;
        if (!parse_expression(p, 1, &condition)) {
            return false;
        }
        Stmt *stmt = new_stmt_at(ST_DO_WHILE, kw);
        stmt->expr = condition;
        stmt->body = stmt_vec_take(&body, &stmt->body_count);
        stmt_vec_push(out, stmt);
        skip_separators(p);
        return true;
    }

    if (token_is_kw(cur(p), "para")) {
        const Token *kw = advance(p);
        Stmt *init = NULL;
        if (cur(p)->kind != TK_SEMI) {
            if (is_type_kw(cur(p))) {
                init = parse_for_declaration(p);
                if (!init) {
                    return false;
                }
            } else if (cur(p)->kind == TK_IDENT) {
                StmtVec temp = {0};
                if (!parse_new_assignment(p, &temp, 0)) {
                    return false;
                }
                init = temp.items[0];
                free(temp.items);
            } else {
                return fail(p, DIAG_SINTATICO,
                            "Inicialização inválida no laço Para.",
                            "declaração, atribuição ou ';'",
                            "Exemplo: para inteiro i = 0; i < 10; i++ { ... }");
            }
        }
        if (!expect(p, TK_SEMI, "';' depois da inicialização do Para",
                    "Exemplo: para inteiro i = 0; i < 10; i++ { ... }")) {
            return false;
        }
        Expr *condition = NULL;
        if (cur(p)->kind != TK_SEMI && !parse_expression(p, 1, &condition)) {
            return false;
        }
        if (!expect(p, TK_SEMI, "';' depois da condição do Para",
                    "Exemplo: para inteiro i = 0; i < 10; i++ { ... }")) {
            return false;
        }
        Stmt *step = NULL;
        if (cur(p)->kind != TK_LBRACE) {
            if (cur(p)->kind != TK_IDENT) {
                return fail(p, DIAG_SINTATICO,
                            "Incremento inválido no laço Para.",
                            "atribuição ou '++'/'--'",
                            "Exemplo: i = i + 1 ou i++.");
            }
            StmtVec temp = {0};
            if (!parse_new_assignment(p, &temp, 0)) {
                return false;
            }
            step = temp.items[0];
            free(temp.items);
        }
        if (!expect_lbrace(p)) {
            return false;
        }
        StmtVec body = {0};
        if (!parse_new_block(p, &body) || !expect_rbrace(p)) {
            return false;
        }
        Stmt *stmt = new_stmt_at(ST_FOR, kw);
        stmt->init = init;
        stmt->expr = condition;
        stmt->step = step;
        stmt->body = stmt_vec_take(&body, &stmt->body_count);
        stmt_vec_push(out, stmt);
        skip_separators(p);
        return true;
    }

    if (accept_kw(p, "pare")) {
        Stmt *stmt = new_stmt_at(ST_BREAK, cur(p));
        stmt_vec_push(out, stmt);
        skip_separators(p);
        return true;
    }

    if (accept_kw(p, "continue")) {
        Stmt *stmt = new_stmt_at(ST_CONTINUE, cur(p));
        stmt_vec_push(out, stmt);
        skip_separators(p);
        return true;
    }

    if (token_is_kw(cur(p), "retorne")) {
        const Token *kw = advance(p);
        Stmt *stmt = new_stmt_at(ST_RETURN, kw);
        if (!is_statement_end(cur(p))) {
            if (!parse_expression(p, 1, &stmt->expr)) {
                return false;
            }
        }
        stmt_vec_push(out, stmt);
        skip_separators(p);
        return true;
    }

    if (is_type_kw(cur(p))) {
        return parse_new_declaration(p, out);
    }

    if (cur(p)->kind == TK_IDENT) {
        return parse_new_assignment(p, out, 1);
    }

    return fail(p, DIAG_SINTATICO,
                "Instrução não reconhecida no bloco.",
                "declaração, atribuição, leia, escreva/escreval, se, "
                "enquanto, para, faca, pare, continue ou retorne",
                "Exemplo: inteiro idade = 18; escreval idade.");
}

static bool parse_new_block(Parser *p, StmtVec *out)
{
    while (cur(p)->kind != TK_RBRACE && cur(p)->kind != TK_EOF) {
        skip_separators(p);
        if (cur(p)->kind == TK_RBRACE || cur(p)->kind == TK_EOF) {
            break;
        }
        if (!parse_new_statement(p, out)) {
            return false;
        }
        skip_separators(p);
    }
    return true;
}

static bool parse_new_function(Parser *p, StmtVec *body, bool *has_entry,
                               const Token *kw)
{
    const Token *name = NULL;
    if (is_type_kw(cur(p))) {
        LnType return_type;
        if (!parse_new_type(p, &return_type)) {
            return false;
        }
        name = expect(p, TK_IDENT, "nome da função",
                      "Exemplo: funcao inicio { ... }");
    } else if (cur(p)->kind == TK_IDENT || token_is_kw(cur(p), "inicio")) {
        name = advance(p);
    } else {
        return fail(p, DIAG_SINTATICO, "Nome de função esperado.",
                    "funcao inicio",
                    "Exemplo: funcao inicio { ... }");
    }
    if (!name) {
        return false;
    }
    if (!ident_ieq(name->value, "inicio")) {
        return fail_at(
            p, DIAG_SINTATICO, name,
            "Funções definidas pelo usuário ainda não são suportadas nesta "
            "etapa.",
            "funcao inicio",
            "Use apenas a função de entrada 'funcao inicio { ... }' por "
            "enquanto.");
    }
    if (*has_entry) {
        return fail_at(p, DIAG_SINTATICO, name,
                       "A função de entrada 'inicio' foi declarada mais de uma "
                       "vez.",
                       "uma única funcao inicio",
                       "Remova a declaração duplicada.");
    }
    if (!expect_lbrace(p)) {
        return false;
    }
    StmtVec entry = {0};
    if (!parse_new_block(p, &entry) || !expect_rbrace(p)) {
        return false;
    }
    for (int i = 0; i < entry.count; i++) {
        stmt_vec_push(body, entry.items[i]);
    }
    free(entry.items);
    *has_entry = true;
    (void)kw;
    return true;
}

static bool is_new_syntax(const TokenList *tokens)
{
    const Token *first = &tokens->items[0];
    if (token_is_kw(first, "funcao")) {
        if (tokens->items[1].kind == TK_IDENT &&
            token_is_kw(&tokens->items[2], "recebe")) {
            return false;
        }
        return true;
    }
    if (token_is_kw(first, "inicio")) {
        return tokens->items[1].kind == TK_LBRACE;
    }
    if (!token_is_kw(first, "programa")) {
        return false;
    }
    int i = 1;
    if (tokens->items[i].kind == TK_STRING) {
        i++;
    }
    return tokens->items[i].kind == TK_LBRACE;
}

static bool parse_new_program(Parser *p, Program *program)
{
    StmtVec body = {0};
    bool has_entry = false;
    bool ok = true;

    if (accept_kw(p, "programa")) {
        if (cur(p)->kind == TK_STRING) {
            free(program->name);
            program->name = strip_quotes(cur(p)->value);
            advance(p);
        }
        if (!expect_lbrace(p)) {
            return false;
        }
        while (cur(p)->kind != TK_RBRACE && cur(p)->kind != TK_EOF) {
            skip_separators(p);
            if (cur(p)->kind == TK_RBRACE || cur(p)->kind == TK_EOF) {
                break;
            }
            if (token_is_kw(cur(p), "funcao")) {
                const Token *kw = advance(p);
                if (!parse_new_function(p, &body, &has_entry, kw)) {
                    ok = false;
                    break;
                }
            } else if (token_is_kw(cur(p), "inicio")) {
                const Token *kw = advance(p);
                if (has_entry) {
                    ok = fail_at(p, DIAG_SINTATICO, kw,
                                 "A função de entrada 'inicio' foi declarada "
                                 "mais de uma vez.",
                                 "uma única funcao inicio",
                                 "Remova a declaração duplicada.");
                    break;
                }
                if (!expect_lbrace(p)) {
                    ok = false;
                    break;
                }
                StmtVec entry = {0};
                if (!parse_new_block(p, &entry) || !expect_rbrace(p)) {
                    ok = false;
                    break;
                }
                for (int i = 0; i < entry.count; i++) {
                    stmt_vec_push(&body, entry.items[i]);
                }
                free(entry.items);
                has_entry = true;
            } else if (!parse_new_statement(p, &body)) {
                ok = false;
                break;
            }
        }
        if (ok && !expect_rbrace(p)) {
            ok = false;
        }
    } else if (token_is_kw(cur(p), "funcao")) {
        const Token *kw = advance(p);
        ok = parse_new_function(p, &body, &has_entry, kw);
    } else if (token_is_kw(cur(p), "inicio")) {
        const Token *kw = advance(p);
        if (!expect_lbrace(p)) {
            ok = false;
        } else {
            StmtVec entry = {0};
            if (!parse_new_block(p, &entry) || !expect_rbrace(p)) {
                ok = false;
            } else {
                for (int i = 0; i < entry.count; i++) {
                    stmt_vec_push(&body, entry.items[i]);
                }
                free(entry.items);
                has_entry = true;
            }
        }
        (void)kw;
    } else {
        ok = fail(p, DIAG_SINTATICO, "Programa não reconhecido.",
                  "programa { ... } ou funcao inicio { ... }",
                  "Use blocos com '{' e '}' na sintaxe do DOC Script.");
    }

    if (ok && cur(p)->kind != TK_EOF) {
        ok = fail(p, DIAG_SINTATICO,
                  "Conteúdo inesperado depois do bloco principal.",
                  "fim do arquivo",
                  "Feche todos os blocos e remova o conteúdo restante.");
    }
    if (ok && !has_entry && body.count == 0) {
        ok = fail(p, DIAG_SINTATICO, "Programa vazio.",
                  "ao menos uma instrução ou funcao inicio",
                  "Adicione comandos dentro do bloco principal.");
    }

    program->body = stmt_vec_take(&body, &program->body_count);
    return ok;
}

static Stmt *parse_statement(Parser *p);

static bool parse_block(Parser *p, const char *const *stops, int stop_count,
                        Stmt ***out, int *out_count)
{
    Stmt **items = NULL;
    int count = 0;
    int cap = 0;

    while (cur(p)->kind != TK_EOF) {
        bool is_stop = false;
        if (cur(p)->kind == TK_KW) {
            for (int i = 0; i < stop_count; i++) {
                if (token_is_kw(cur(p), stops[i])) {
                    is_stop = true;
                    break;
                }
            }
        }
        if (is_stop) {
            break;
        }
        Stmt *stmt = parse_statement(p);
        if (!stmt) {
            free(items);
            return false;
        }
        if (count == cap) {
            cap = cap ? cap * 2 : 16;
            items = xrealloc(items, (size_t)cap * sizeof(Stmt *));
        }
        items[count++] = stmt;
    }

    *out = items;
    *out_count = count;
    return true;
}

static Stmt *parse_statement(Parser *p)
{
    if (accept_kw(p, "leia")) {
        const Token *name = expect(p, TK_IDENT, "nome de variável",
                                   "Exemplo: Leia idade.");
        if (!name || !require_variable(p, name) || !expect_dot(p)) {
            return NULL;
        }
        Stmt *stmt = new_stmt(ST_READ);
        stmt->name = xstrdup(name->value);
        return stmt;
    }

    if (accept_kw(p, "escreva")) {
        Expr *expr = NULL;
        if (!parse_expression(p, 1, &expr) || !expect_dot(p)) {
            return NULL;
        }
        Stmt *stmt = new_stmt(ST_WRITE);
        stmt->expr = expr;
        return stmt;
    }

    if (accept_kw(p, "se")) {
        Expr *condition = NULL;
        if (!parse_expression(p, 1, &condition) || !expect_kw(p, "entao") ||
            !expect_dot(p)) {
            return NULL;
        }
        static const char *const stops_then[] = {"senao", "fimse"};
        Stmt **yes = NULL;
        int yes_count = 0;
        if (!parse_block(p, stops_then, 2, &yes, &yes_count)) {
            return NULL;
        }
        Stmt **no = NULL;
        int no_count = 0;
        if (accept_kw(p, "senao")) {
            if (!expect_dot(p)) {
                return NULL;
            }
            static const char *const stops_else[] = {"fimse"};
            if (!parse_block(p, stops_else, 1, &no, &no_count)) {
                return NULL;
            }
        }
        if (!expect_kw(p, "fimse") || !expect_dot(p)) {
            return NULL;
        }
        Stmt *stmt = new_stmt(ST_IF);
        stmt->expr = condition;
        stmt->body = yes;
        stmt->body_count = yes_count;
        stmt->else_body = no;
        stmt->else_count = no_count;
        return stmt;
    }

    if (cur(p)->kind == TK_IDENT) {
        const Token *name = advance(p);
        if (!require_variable(p, name)) {
            return NULL;
        }
        if (!accept(p, TK_ASSIGN)) {
            fail(p, DIAG_SINTATICO,
                 "Esperado operador de atribuição '<-' depois do nome da "
                 "variável.",
                 "'<-'",
                 "Em DOC Script a atribuição usa '<-', como em "
                 "'resultado <- idade + 1.'.");
            return NULL;
        }
        Expr *expr = NULL;
        if (!parse_expression(p, 1, &expr) || !expect_dot(p)) {
            return NULL;
        }
        Stmt *stmt = new_stmt(ST_ASSIGN);
        stmt->name = xstrdup(name->value);
        stmt->expr = expr;
        return stmt;
    }

    fail(p, DIAG_SINTATICO,
         "Instrução não reconhecida. Esperado Leia, Escreva, Se ou "
         "atribuição.",
         "Leia, Escreva, Se ou atribuição com '<-'",
         "Cada instrução começa com uma palavra-chave ou com o nome de uma "
         "variável seguido de '<-'.");
    return NULL;
}

static bool parse_type(Parser *p, LnType *out)
{
    const Token *token = cur(p);
    if (token->kind == TK_KW) {
        if (ascii_ieq(token->value, "inteiro")) {
            advance(p);
            *out = TY_INTEIRO;
            return true;
        }
        if (ascii_ieq(token->value, "real")) {
            advance(p);
            *out = TY_REAL;
            return true;
        }
        if (ascii_ieq(token->value, "logico")) {
            advance(p);
            *out = TY_LOGICO;
            return true;
        }
        if (ascii_ieq(token->value, "texto")) {
            advance(p);
            *out = TY_TEXTO;
            return true;
        }
    }
    char found[160];
    describe_token(token, found, sizeof(found));
    diag_set_full(p->diag, DIAG_SEMANTICO, token->line, token->col, found,
                  "Inteiro, Real, Logico ou Texto",
                  "Tipo não reconhecido.",
                  "Declare o tipo depois de ':' usando um dos tipos da "
                  "linguagem.");
    return false;
}

static char *strip_quotes(const char *value)
{
    size_t len = strlen(value);
    if (len >= 2) {
        return xstrndup(value + 1, len - 2);
    }
    return xstrdup("");
}

static bool parse_imports(Parser *p, Program *program)
{
    if (!expect(p, TK_COLON, "':' depois de Arquivos",
                "Exemplo: Arquivos: \"Cadastro\".")) {
        return false;
    }
    int count = 0;
    int cap = 0;
    for (;;) {
        const Token *token = expect(p, TK_STRING, "nome de arquivo entre aspas",
                                    "Use aspas duplas: \"Cadastro\".");
        if (!token) {
            return false;
        }
        if (count == cap) {
            cap = cap ? cap * 2 : 4;
            program->imports =
                xrealloc(program->imports, (size_t)cap * sizeof(char *));
        }
        program->imports[count++] = strip_quotes(token->value);
        program->import_count = count;
        if (!expect_dot(p)) {
            return false;
        }
        if (!accept(p, TK_COMMA)) {
            break;
        }
    }
    return true;
}

static bool parse_variables(Parser *p)
{
    while (cur(p)->kind == TK_IDENT) {
        const Token *name = advance(p);
        if (find_var(p, name->value)) {
            char message[512];
            snprintf(message, sizeof(message),
                     "Variável '%s' declarada mais de uma vez.", name->value);
            return fail_at(p, DIAG_SEMANTICO, name, message,
                           "nome de variável ainda não declarado",
                           "Escolha outro nome ou remova a declaração "
                           "duplicada.");
        }
        if (!expect(p, TK_COLON, "':' depois do nome da variável",
                    "Exemplo: idade: Inteiro.")) {
            return false;
        }
        LnType type;
        if (!parse_type(p, &type) || !expect_dot(p)) {
            return false;
        }
        add_var(p, name->value, type, name->line, name->col);
    }
    return true;
}

static bool parse_function(Parser *p)
{
    const Token *keyword = advance(p);
    const Token *name = expect(p, TK_IDENT, "nome da função",
                               "Exemplo: Função Calcular Recebe Valor.");
    if (!name) {
        return false;
    }
    if (find_function(p, name->value)) {
        char message[256];
        snprintf(message, sizeof(message), "Função '%s' declarada mais de uma vez.",
                 name->value);
        return fail_at(p, DIAG_SEMANTICO, name, message,
                       "nome de função ainda não declarado",
                       "Escolha outro nome para a função.");
    }
    if (!expect_kw(p, "recebe")) {
        return false;
    }

    Function function;
    memset(&function, 0, sizeof(function));
    function.name = xstrdup(name->value);
    function.line = keyword->line;
    function.col = keyword->col;
    int params_cap = 0;
    int saved_var_count = p->var_count;
    for (;;) {
        const Token *param = expect(p, TK_IDENT, "nome de parâmetro",
                                    "Separe parâmetros com vírgula ou 'e'.");
        if (!param) {
            goto error;
        }
        if (find_var(p, param->value)) {
            char message[256];
            snprintf(message, sizeof(message),
                     "Parâmetro '%s' duplicado ou já declarado no escopo.",
                     param->value);
            fail_at(p, DIAG_SEMANTICO, param, message,
                    "nome de parâmetro único", NULL);
            goto error;
        }
        if (function.param_count == params_cap) {
            params_cap = params_cap ? params_cap * 2 : 4;
            function.params = xrealloc(function.params,
                                       (size_t)params_cap * sizeof(char *));
        }
        function.params[function.param_count++] = xstrdup(param->value);
        add_var(p, param->value, TY_REAL, param->line, param->col);
        if (accept(p, TK_COMMA) || accept_kw(p, "e")) {
            continue;
        }
        break;
    }

    if (!expect_dot(p) || !expect_kw(p, "retorna") ||
        !parse_expression(p, 1, &function.return_expr) || !expect_dot(p)) {
        goto error;
    }

    while (p->var_count > saved_var_count) {
        free(p->vars[--p->var_count].name);
    }
    p->program->functions = xrealloc(
        p->program->functions,
        (size_t)(p->program->function_count + 1) * sizeof(Function));
    p->program->functions[p->program->function_count++] = function;
    return true;

error:
    free(function.name);
    for (int i = 0; i < function.param_count; i++) {
        free(function.params[i]);
    }
    free(function.params);
    parser_expr_free(function.return_expr);
    return false;
}

bool parse_program(const TokenList *tokens, Program *program, Diagnostic *diag)
{
    Parser p;
    memset(&p, 0, sizeof(p));
    p.tokens = tokens;
    p.diag = diag;

    memset(program, 0, sizeof(*program));
    p.program = program;
    program->name = xstrdup("Programa");

    if (is_new_syntax(tokens)) {
        bool new_ok = parse_new_program(&p, program);
        program->variables = p.vars;
        program->variable_count = p.var_count;
        if (!new_ok) {
            program_free(program);
        }
        return new_ok;
    }

    bool ok = true;

    if (accept_kw(&p, "arquivos")) {
        ok = parse_imports(&p, program);
    }

    if (ok && accept_kw(&p, "programa")) {
        const Token *name =
            expect(&p, TK_STRING, "nome do programa entre aspas",
                   "Exemplo: Programa \"Exemplo\".");
        if (!name) {
            ok = false;
        } else {
            free(program->name);
            program->name = strip_quotes(name->value);
            ok = expect_dot(&p);
        }
    }

    if (ok && accept_kw(&p, "variaveis")) {
        ok = parse_variables(&p);
    }

    while (ok && token_is_kw(cur(&p), "funcao")) {
        ok = parse_function(&p);
    }

    if (ok) {
        ok = expect_kw(&p, "inicio");
    }

    if (ok) {
        static const char *const stops[] = {"fim"};
        ok = parse_block(&p, stops, 1, &program->body, &program->body_count);
    }

    if (ok) {
        ok = expect_kw(&p, "fim");
    }

    if (ok) {
        ok = expect_dot(&p);
    }

    if (ok && cur(&p)->kind != TK_EOF) {
        ok = fail(&p, DIAG_SINTATICO, "Conteúdo inesperado depois de Fim.",
                  "fim do arquivo",
                  "O programa termina em 'Fim.'; remova o conteúdo restante ou "
                  "verifique se falta fechar um bloco.");
    }

    program->variables = p.vars;
    program->variable_count = p.var_count;

    if (!ok) {
        program_free(program);
    }
    return ok;
}
