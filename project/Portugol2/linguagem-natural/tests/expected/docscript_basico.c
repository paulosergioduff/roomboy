/* Código gerado automaticamente. Não edite este arquivo diretamente. */
#include <stdio.h>

int main(void) {
    int idade = 0;
    int resto = 0;
    char nome[256] = {0};
    int autorizado = 0;

    idade = 20;
    resto = (7 % 2);
    snprintf(nome, sizeof(nome), "%s", "Ana");
    autorizado = 1;
    printf("%s", "Nome: ");
    printf("%s", nome);
    printf("\n");
    printf("%s", "Idade: ");
    printf("%d", idade);
    printf("\n");
    printf("%s", "Resto: ");
    printf("%d", resto);
    printf("\n");
    if (((idade >= 18) && autorizado)) {
        printf("%s", "maior");
        printf("\n");
    } else {
        printf("%s", "menor");
        printf("\n");
    }
    if ((!autorizado)) {
        printf("%s", "negado");
        printf("\n");
    }

    return 0;
}
