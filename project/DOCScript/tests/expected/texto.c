/* Código gerado automaticamente. Não edite este arquivo diretamente. */
#include <stdio.h>

int main(void) {
    char Nome[256] = {0};
    int Idade = 0;

    printf("%s\n", "Nome: ");
    /* Leitura de Texto requer buffer definido pelo programa. */
    fgets(Nome, sizeof(Nome), stdin);
    printf("%s\n", "Idade: ");
    if (scanf("%d", &Idade) != 1) return 1;
    printf("%s\n", Nome);
    printf("%d\n", Idade);

    return 0;
}
