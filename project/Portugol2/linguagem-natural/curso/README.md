# Curso interativo de DOC Script

O curso tem dez aulas com navegação, exemplos editáveis, entrada padrão e execução no compilador local.

## Iniciar

Na pasta `linguagem-natural`, compile e inicie o servidor:

```bash
make
./docscript serve
```

Abra <http://127.0.0.1:8080/curso/>. O servidor entrega a página e seus arquivos estáticos e usa `/api/compile` para executar o código no navegador.

## Usar

- Escolha uma aula na navegação lateral e avance na ordem sugerida.
- Edite o exemplo e pressione **Executar código** ou `Ctrl+Enter`.
- Para programas com `leia`, altere os valores na caixa **Entrada do programa**.
- Use **Restaurar** para recuperar o código inicial da aula.
- Marque aulas concluídas; o progresso e suas edições ficam salvos no navegador.

O curso é servido junto com o compilador local. Não abra `index.html` diretamente pelo sistema de arquivos, pois a execução depende da API local.
