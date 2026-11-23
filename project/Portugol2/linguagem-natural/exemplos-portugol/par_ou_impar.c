/* Código gerado automaticamente. Não edite este arquivo diretamente. */
#include <stdio.h>

int main(void) {
    int numero = 0;


    printf("%s", "Digite um numero inteiro: ");

    if (scanf("%d", &numero) != 1) return 1;
    if (((numero % 2) == 0)) {
        printf("%s", "Par");
        printf("\n");
    } else {
        printf("%s", "Impar");
        printf("\n");
    }

    return 0;
}
