#ifndef DOCSCRIPT_JSON_H
#define DOCSCRIPT_JSON_H

/* Escapa uma string para ser usada como valor JSON. Retorna buffer malloc. */
char *json_escape_alloc(const char *text);

/* Extrai o valor string de uma chave simples de um objeto JSON.
 * Retorna NULL se a chave não existir ou não for string. */
char *json_get_string_alloc(const char *json, const char *key);

#endif
