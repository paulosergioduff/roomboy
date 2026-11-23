/* Código gerado automaticamente. Não edite este arquivo diretamente. */
#include <stdio.h>

int main(void) {
    int limite = 0;
    int numero = 0;


    printf("%s", "Contar ate: ");

    if (scanf("%d", &limite) != 1) return 1;
    for (numero = 1; (numero <= limite); numero = (numero + 1)) {
        printf("%d", numero);
        printf("\n");
    }

    return 0;
}
