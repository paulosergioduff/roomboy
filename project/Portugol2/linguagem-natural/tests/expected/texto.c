/* Código gerado automaticamente. Não edite este arquivo diretamente. */
#include <stdio.h>

int main(void) {
    char nome[256] = {0};
    int idade = 0;

    printf("%s\n", "Nome: ");
    /* Leitura de Texto requer buffer definido pelo programa. */
    fgets(nome, sizeof(nome), stdin);
    printf("%s\n", "Idade: ");
    if (scanf("%d", &idade) != 1) return 1;
    printf("%s\n", nome);
    printf("%d\n", idade);

    return 0;
}
