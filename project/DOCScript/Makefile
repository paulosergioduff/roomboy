CC ?= cc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra
LDFLAGS ?=
BIN := docscript

CORE_SRC := \
	src/util.c \
	src/strbuf.c \
	src/diag.c \
	src/token.c \
	src/ast.c \
	src/parser.c \
	src/codegen.c \
	src/semantic.c \
	src/process.c \
	src/json.c

CORE_OBJ := $(CORE_SRC:.c=.o)
OBJ := $(CORE_OBJ) src/main.o src/server.o
DEP := $(OBJ:.o=.d)

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDFLAGS)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -MMD -MP -c -o $@ $<

src/main.o: src/main.c src/ast.h src/codegen.h src/diag.h src/parser.h \
	src/process.h src/semantic.h src/server.h src/token.h src/util.h
src/server.o: src/server.c src/json.h src/process.h src/server.h src/strbuf.h \
	src/util.h

test: $(BIN)
	./tests/run.sh

clean:
	rm -f $(BIN) $(OBJ) $(DEP)

-include $(DEP)

.PHONY: all test clean
