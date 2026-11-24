/* Código gerado automaticamente. Não edite este arquivo diretamente. */
#include <stdio.h>

int main(void) {
    int idade = 0;
    int autorizado = 0;
    int resultado = 0;

    if (scanf("%d", &idade) != 1) return 1;
    autorizado = 1;
    resultado = ((idade >= 18) && (autorizado == 1));
    if (resultado) {
        printf("%s\n", "entrou");
    } else {
        printf("%s\n", "bloqueado");
    }
    if ((!resultado)) {
        printf("%s\n", "negado");
    }

    return 0;
}
