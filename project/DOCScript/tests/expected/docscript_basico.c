/* Código gerado automaticamente. Não edite este arquivo diretamente. */
#include <stdio.h>

int main(void) {
    int Idade = 0;
    int Resto = 0;
    char Nome[256] = {0};
    int Autorizado = 0;

    Idade = 20;
    Resto = (7 % 2);
    snprintf(Nome, sizeof(Nome), "%s", "Ana");
    Autorizado = 1;
    printf("%s", "Nome: ");
    printf("%s", Nome);
    printf("\n");
    printf("%s", "Idade: ");
    printf("%d", Idade);
    printf("\n");
    printf("%s", "Resto: ");
    printf("%d", Resto);
    printf("\n");
    if (((Idade >= 18) && Autorizado)) {
        printf("%s", "maior");
        printf("\n");
    } else {
        printf("%s", "menor");
        printf("\n");
    }
    if ((!Autorizado)) {
        printf("%s", "negado");
        printf("\n");
    }

    return 0;
}
