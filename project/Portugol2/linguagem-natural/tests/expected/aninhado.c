/* Código gerado automaticamente. Não edite este arquivo diretamente. */
#include <stdio.h>

int main(void) {
    int x = 0;
    int y = 0;

    if (scanf("%d", &x) != 1) return 1;
    if ((x > 0)) {
        if ((x > 10)) {
            y = ((x * 2) + 1);
        } else {
            y = (-x);
        }
    } else {
        y = 0;
    }
    printf("%d\n", y);

    return 0;
}
