#!/usr/bin/env python3
"""Compilador didático: Linguagem Natural procedural -> C."""
from __future__ import annotations
import re
import sys
from dataclasses import dataclass
from pathlib import Path

@dataclass
class Token:
    kind: str
    value: str
    line: int
    col: int

class CompileError(Exception):
    pass

KEYWORDS = {
    "arquivos", "programa", "variaveis", "inicio", "fim", "inteiro",
    "real", "logico", "texto", "leia", "escreva", "se", "senao",
    "fimse", "entao", "verdadeiro", "falso", "e", "ou", "nao"
}

TOKEN_RE = re.compile(
    r'(?P<SPACE>\s+)'
    r'|(?P<COMMENT>\([^()]*\))'
    r'|(?P<STRING>"(?:\\.|[^"\\])*")'
    r'|(?P<NUMBER>\d+(?:\.\d+)?)'
    r'|(?P<ASSIGN><-)'
    r'|(?P<OP>>=|<=|==|!=|[=<>+*/-])'
    r'|(?P<DOT>\.)'
    r'|(?P<COLON>:)'
    r'|(?P<COMMA>,)'
    r'|(?P<IDENT>[A-Za-zÀ-ÿ_][A-Za-zÀ-ÿ_0-9]*)'
)

def tokenize(source: str) -> list[Token]:
    tokens, pos, line, col = [], 0, 1, 1
    while pos < len(source):
        match = TOKEN_RE.match(source, pos)
        if not match:
            raise CompileError(f"Caractere inesperado {source[pos]!r} na linha {line}, coluna {col}.")
        kind, value = match.lastgroup, match.group()
        start_line, start_col = line, col
        line_breaks = value.count("\n")
        if line_breaks:
            line += line_breaks
            col = len(value.rsplit("\n", 1)[-1]) + 1
        else:
            col += len(value)
        pos = match.end()
        if kind in ("SPACE", "COMMENT"):
            continue
        if kind == "IDENT" and value.casefold() in KEYWORDS:
            kind = "KW"
        tokens.append(Token(kind, value, start_line, start_col))
    tokens.append(Token("EOF", "", line, col))
    return tokens

class Parser:
    def __init__(self, tokens: list[Token]):
        self.tokens, self.i = tokens, 0
        self.variables: dict[str, str] = {}

    @property
    def current(self) -> Token:
        return self.tokens[self.i]

    def advance(self) -> Token:
        token = self.current
        if token.kind != "EOF": self.i += 1
        return token

    def fail(self, message: str):
        t = self.current
        raise CompileError(f"{message} (linha {t.line}, coluna {t.col}; encontrado {t.value or 'fim do arquivo'!r}).")

    def is_kw(self, value: str) -> bool:
        return self.current.kind == "KW" and self.current.value.casefold() == value.casefold()

    def accept_kw(self, value: str) -> bool:
        if self.is_kw(value): self.advance(); return True
        return False

    def expect_kw(self, value: str):
        if not self.accept_kw(value): self.fail(f"Esperado '{value}'.")

    def accept(self, kind: str) -> Token | None:
        if self.current.kind == kind: return self.advance()
        return None

    def expect(self, kind: str, description: str):
        token = self.accept(kind)
        if not token: self.fail(f"Esperado {description}.")
        return token

    def expect_dot(self): self.expect("DOT", "ponto final '.'")

    def parse(self):
        imports = []
        if self.accept_kw("Arquivos"):
            self.expect("COLON", "':' depois de Arquivos")
            while True:
                imports.append(self.expect("STRING", "nome de arquivo entre aspas").value[1:-1])
                self.expect_dot()
                if not self.accept("COMMA"): break
        program_name = "Programa"
        if self.accept_kw("Programa"):
            program_name = self.expect("STRING", "nome do programa").value[1:-1]
            self.expect_dot()
        if self.accept_kw("Variaveis"):
            while self.current.kind == "IDENT":
                name = self.advance().value
                key = name.casefold()
                if key in self.variables: self.fail(f"Variável '{name}' declarada mais de uma vez")
                self.expect("COLON", "':' depois do nome da variável")
                typ = self.current.value.casefold() if self.current.kind == "KW" else ""
                if typ not in ("inteiro", "real", "logico", "texto"):
                    self.fail("Tipo esperado: Inteiro, Real, Logico ou Texto")
                self.advance(); self.expect_dot()
                self.variables[key] = typ
        self.expect_kw("Inicio")
        statements = self.parse_block({"fim"})
        self.expect_kw("Fim")
        self.expect_dot()
        if self.current.kind != "EOF": self.fail("Conteúdo inesperado depois de Fim.")
        return {"name": program_name, "imports": imports, "variables": dict(self.variables), "body": statements}

    def parse_block(self, stop_words: set[str]):
        result = []
        while self.current.kind != "EOF" and not (self.current.kind == "KW" and self.current.value.casefold() in stop_words):
            result.append(self.parse_statement())
        return result

    def parse_statement(self):
        if self.accept_kw("Leia"):
            name = self.expect("IDENT", "nome de variável").value
            self.require_variable(name); self.expect_dot()
            return ("read", name)
        if self.accept_kw("Escreva"):
            expr = self.parse_expression(); self.expect_dot()
            return ("write", expr)
        if self.accept_kw("Se"):
            condition = self.parse_expression()
            self.expect_kw("entao"); self.expect_dot()
            yes = self.parse_block({"senao", "fimse"})
            no = []
            if self.accept_kw("Senao"):
                self.expect_dot()
                no = self.parse_block({"fimse"})
            self.expect_kw("FimSe"); self.expect_dot()
            return ("if", condition, yes, no)
        if self.current.kind == "IDENT":
            name = self.advance().value
            self.require_variable(name)
            self.expect("ASSIGN", "operador de atribuição '<-'")
            expr = self.parse_expression(); self.expect_dot()
            return ("assign", name, expr)
        self.fail("Instrução não reconhecida. Esperado Leia, Escreva, Se ou atribuição")

    def require_variable(self, name: str):
        if name.casefold() not in self.variables:
            self.fail(f"Variável '{name}' não foi declarada")

    PRECEDENCE = {"ou": 1, "e": 2, "=": 3, "==": 3, "!=": 3, ">": 3, "<": 3, ">=": 3, "<=": 3, "+": 4, "-": 4, "*": 5, "/": 5}

    def parse_expression(self, min_prec=1):
        token = self.current
        if token.kind == "OP" and token.value in ("-", "+"):
            self.advance(); left = ("unary", token.value, self.parse_expression(6))
        elif self.accept_kw("nao"):
            left = ("unary", "nao", self.parse_expression(6))
        elif self.accept("NUMBER"):
            left = ("number", token.value)
        elif self.accept("STRING"):
            left = ("string", token.value)
        elif self.accept_kw("verdadeiro"):
            left = ("boolean", True)
        elif self.accept_kw("falso"):
            left = ("boolean", False)
        elif self.accept("IDENT"):
            self.require_variable(token.value); left = ("variable", token.value)
        else:
            self.fail("Expressão esperada")
        while True:
            op_token = self.current
            op = op_token.value.casefold()
            if op_token.kind not in ("OP", "KW") or op not in self.PRECEDENCE: break
            prec = self.PRECEDENCE[op]
            if prec < min_prec: break
            self.advance()
            right = self.parse_expression(prec + 1)
            left = ("binary", op, left, right)
        return left

C_TYPES = {"inteiro": "int", "real": "double", "logico": "int", "texto": "char"}

def c_expr(expr):
    kind = expr[0]
    if kind in ("number", "string"): return expr[1]
    if kind == "boolean": return "1" if expr[1] else "0"
    if kind == "variable": return c_name(expr[1])
    if kind == "unary": return f"({'!' if expr[1] == 'nao' else expr[1]}{c_expr(expr[2])})"
    if kind == "binary":
        op = {"e": "&&", "ou": "||", "=": "=="}.get(expr[1], expr[1])
        return f"({c_expr(expr[2])} {op} {c_expr(expr[3])})"
    raise CompileError("Expressão interna não suportada.")

def c_name(name: str) -> str:
    # Mantém identificadores ASCII válidos em C e evita palavras-chave do C.
    import unicodedata
    value = unicodedata.normalize("NFKD", name).encode("ascii", "ignore").decode()
    value = re.sub(r"\W", "_", value)
    if not value or value[0].isdigit(): value = "v_" + value
    if value in {"int", "char", "double", "if", "else", "return", "while", "for", "const", "struct"}:
        value = "v_" + value
    return value

def emit_statement(stmt, variables, indent=1):
    pad = "    " * indent
    kind = stmt[0]
    if kind == "read":
        name = stmt[1]; typ = variables[name.casefold()]
        if typ == "texto": return f'{pad}/* Leitura de Texto requer buffer definido pelo programa. */\n{pad}fgets({c_name(name)}, sizeof({c_name(name)}), stdin);'
        fmt = {"inteiro": "%d", "real": "%lf", "logico": "%d"}[typ]
        return f'{pad}if (scanf("{fmt}", &{c_name(name)}) != 1) return 1;'
    if kind == "write":
        expr = stmt[1]
        if expr[0] == "string": return f"{pad}printf(\"%s\\n\", {c_expr(expr)});"
        if expr[0] == "variable":
            typ = variables[expr[1].casefold()]
            fmt = {"inteiro": "%d", "real": "%g", "logico": "%s", "texto": "%s"}[typ]
            value = f'({c_name(expr[1])} ? "verdadeiro" : "falso")' if typ == "logico" else c_name(expr[1])
            return f'{pad}printf("{fmt}\\n", {value});'
        return f'{pad}printf("%g\\n", (double)({c_expr(expr)}));'
    if kind == "assign": return f"{pad}{c_name(stmt[1])} = {c_expr(stmt[2])};"
    if kind == "if":
        lines = [f"{pad}if ({c_expr(stmt[1])}) {{"]
        for child in stmt[2]: lines.append(emit_statement(child, variables, indent + 1))
        if stmt[3]:
            lines.append(f"{pad}}} else {{")
            for child in stmt[3]: lines.append(emit_statement(child, variables, indent + 1))
        lines.append(f"{pad}}}")
        return "\n".join(lines)
    raise CompileError("Instrução interna não suportada.")

def translate(ast):
    lines = ["/* Código gerado automaticamente. Não edite este arquivo diretamente. */", "#include <stdio.h>", "", "int main(void) {"]
    for name, typ in ast["variables"].items():
        if typ == "texto": lines.append(f"    char {c_name(name)}[256] = {{0}};")
        else: lines.append(f"    {C_TYPES[typ]} {c_name(name)} = 0;")
    if ast["variables"]: lines.append("")
    for stmt in ast["body"]: lines.append(emit_statement(stmt, ast["variables"]))
    lines.extend(["", "    return 0;", "}"])
    return "\n".join(lines) + "\n"

def main():
    if len(sys.argv) < 2:
        print("Uso: python3 compiler.py arquivo.ds [-o arquivo.c]", file=sys.stderr); return 2
    source_path = Path(sys.argv[1])
    output_path = Path(sys.argv[3]) if len(sys.argv) >= 4 and sys.argv[2] == "-o" else source_path.with_suffix(".c")
    try:
        tokens = tokenize(source_path.read_text(encoding="utf-8"))
        ast = Parser(tokens).parse()
        output_path.write_text(translate(ast), encoding="utf-8")
        print(f"Compilação concluída: {output_path}")
        return 0
    except (CompileError, OSError) as error:
        print(f"ERRO DE COMPILAÇÃO: {error}", file=sys.stderr); return 1

def ParserVariables(ast):
    # O parser mantém a tabela de tipos; ela é reconstruída a partir da AST expandida abaixo.
    return ast["variables"]

if __name__ == "__main__": raise SystemExit(main())
