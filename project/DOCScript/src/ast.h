#ifndef DOCSCRIPT_AST_H
#define DOCSCRIPT_AST_H

typedef enum {
    TY_INTEIRO,
    TY_REAL,
    TY_LOGICO,
    TY_TEXTO,
    TY_CARACTER
} LnType;

typedef struct {
    char *name;
    LnType type;
    int line;
    int col;
} Variable;

typedef enum {
    EX_NUMBER,
    EX_STRING,
    EX_CHAR,
    EX_BOOL,
    EX_VARIABLE,
    EX_UNARY,
    EX_BINARY
} ExprKind;

typedef struct Expr {
    ExprKind kind;
    char *text;   /* número, texto (com aspas) ou nome da variável */
    int boolean;  /* valor para EX_BOOL */
    char *op;     /* operador para EX_UNARY/EX_BINARY, em minúsculas */
    int line;
    int col;
    struct Expr *lhs;
    struct Expr *rhs;
} Expr;

typedef enum {
    ST_READ,
    ST_WRITE,
    ST_ASSIGN,
    ST_IF,
    ST_DECL,
    ST_WHILE,
    ST_DO_WHILE,
    ST_FOR,
    ST_BREAK,
    ST_CONTINUE,
    ST_RETURN
} StmtKind;

typedef struct Stmt {
    StmtKind kind;
    char *name;            /* ST_READ / ST_ASSIGN / ST_DECL */
    Expr *expr;            /* ST_WRITE / ST_ASSIGN / ST_DECL / condições */
    LnType type;           /* ST_DECL */
    Expr **args;           /* ST_WRITE com múltiplos argumentos */
    int arg_count;
    int newline;           /* ST_WRITE: imprime quebra de linha */
    struct Stmt **body;    /* ramo principal / corpo de laços */
    int body_count;
    struct Stmt **else_body;
    int else_count;
    struct Stmt *init;     /* ST_FOR: inicialização */
    struct Stmt *step;     /* ST_FOR: incremento */
    int line;
    int col;
} Stmt;

typedef struct {
    char *name;
    char **imports;
    int import_count;
    Variable *variables;
    int variable_count;
    Stmt **body;
    int body_count;
} Program;

const char *ln_type_name(LnType type);
void program_free(Program *program);

#endif
