/* Código gerado automaticamente. Não edite este arquivo diretamente. */
#include <stdio.h>

int main(void) {
    int I = 0;
    int J = 0;

    I = 0;
    while ((I < 3)) {
        printf("%d", I);
        printf("\n");
        I = (I + 1);
    }
    for (J = 0; (J < 3); J = (J + 1)) {
        if ((J == 1)) {
            continue;
        }
        printf("%s", "j=");
        printf("%d", J);
        printf("\n");
    }
    do {
        I = (I - 1);
        if ((I == 1)) {
            break;
        }
    } while ((I > 0));
    printf("%s", "fim ");
    printf("%d", I);
    printf("\n");

    return 0;
}
