/* Código gerado automaticamente. Não edite este arquivo diretamente. */
#include <stdio.h>

int main(void) {
    int A = 0;
    int B = 0;
    int Soma = 0;
    double Media = 0;





    if (scanf("%d", &A) != 1) return 1;
    if (scanf("%d", &B) != 1) return 1;
    Soma = (A + B);
    Media = (Soma / 2.0);
    printf("%s", "soma=");
    printf("%d", Soma);
    printf("\n");
    printf("%s", "media=");
    printf("%g", Media);
    printf("\n");

    return 0;
}
