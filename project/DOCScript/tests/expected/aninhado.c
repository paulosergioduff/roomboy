/* Código gerado automaticamente. Não edite este arquivo diretamente. */
#include <stdio.h>

int main(void) {
    int X = 0;
    int Y = 0;

    if (scanf("%d", &X) != 1) return 1;
    if ((X > 0)) {
        if ((X > 10)) {
            Y = ((X * 2) + 1);
        } else {
            Y = (-X);
        }
    } else {
        Y = 0;
    }
    printf("%d\n", Y);

    return 0;
}
