/* Código gerado automaticamente. Não edite este arquivo diretamente. */
#include <stdio.h>

int main(void) {
    double nota1 = 0;
    double nota2 = 0;
    double media = 0;




    printf("%s", "Primeira nota: ");

    if (scanf("%lf", &nota1) != 1) return 1;
    printf("%s", "Segunda nota: ");

    if (scanf("%lf", &nota2) != 1) return 1;
    media = ((nota1 / 2.0) + (nota2 / 2.0));
    printf("%s", "Media: ");
    printf("%g", media);
    printf("\n");

    return 0;
}
