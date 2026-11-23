/* Código gerado automaticamente. Não edite este arquivo diretamente. */
#include <stdio.h>

int main(void) {
    int numero = 0;
    int multiplicador = 0;

    numero = 5;
    multiplicador = 1;
    while ((multiplicador <= 10)) {
        printf("%d", numero);
        printf("%s", " x ");
        printf("%d", multiplicador);
        printf("%s", " = ");
        printf("%g", (double)((numero * multiplicador)));
        printf("\n");
        multiplicador = (multiplicador + 1);
    }

    return 0;
}
