/* Código gerado automaticamente. Não edite este arquivo diretamente. */
#include <stdio.h>

int main(void) {
    int i = 0;
    int j = 0;

    i = 0;
    while ((i < 3)) {
        printf("%d", i);
        printf("\n");
        i = (i + 1);
    }
    for (j = 0; (j < 3); j = (j + 1)) {
        if ((j == 1)) {
            continue;
        }
        printf("%s", "j=");
        printf("%d", j);
        printf("\n");
    }
    do {
        i = (i - 1);
        if ((i == 1)) {
            break;
        }
    } while ((i > 0));
    printf("%s", "fim ");
    printf("%d", i);
    printf("\n");

    return 0;
}
