/* Código gerado automaticamente. Não edite este arquivo diretamente. */
#include <stdio.h>

int main(void) {
    int Idade = 0;
    int Autorizado = 0;
    int Resultado = 0;

    if (scanf("%d", &Idade) != 1) return 1;
    Autorizado = 1;
    Resultado = ((Idade >= 18) && (Autorizado == 1));
    if (Resultado) {
        printf("%s\n", "entrou");
    } else {
        printf("%s\n", "bloqueado");
    }
    if ((!Resultado)) {
        printf("%s\n", "negado");
    }

    return 0;
}
