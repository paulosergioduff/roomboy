const lessons = [
  {
    module: "Comece a programar",
    title: "Olá, programa!",
    duration: "5 min",
    lead: "Programar é dar instruções precisas ao computador. Nesta aula, você cria e executa o seu primeiro programa em DOC Script.",
    body: `<h2>O formato de um programa</h2>
      <p>Um programa começa com <code>programa</code> e tem uma função chamada <code>inicio</code>. O computador começa a executar as instruções dentro dela.</p>
      <div class="concept-row"><div class="concept-card"><strong>programa</strong><span>Dá um nome ao seu projeto.</span></div><div class="concept-card"><strong>funcao inicio</strong><span>Marca onde a execução começa.</span></div></div>
      <p>Escreva uma mensagem com <code>escreval</code>. O comando também pula para a linha seguinte.</p>
      <div class="challenge"><span class="challenge-label">DESAFIO</span><p>Troque a mensagem do editor pelo seu nome e execute o programa.</p></div>`,
    code: `programa "OlaMundo" {
    funcao inicio {
        escreval "Ola, mundo!"
    }
}`,
    stdin: ""
  },
  {
    module: "Comece a programar",
    title: "Instruções e comentários",
    duration: "6 min",
    lead: "Um programa é uma sequência de instruções. Comentários ajudam pessoas a entender o código e são ignorados pelo compilador.",
    body: `<h2>Mostre informações</h2>
      <p><code>escreva</code> imprime sem pular de linha. <code>escreval</code> imprime e termina a linha. Use cada comando para controlar como a mensagem aparece.</p>
      <h2>Explique suas escolhas</h2>
      <p>Toda referência (variáveis, funções e nomes) começa com maiúscula. Comentários ficam entre parênteses e são removidos pelo tokenizador. Os marcadores <code>/*</code> e <code>*/</code> delimitam trechos de exemplo; os marcadores não geram código. As palavras reservadas ficam no conjunto de idioma <code>pt</code>.</p>
      <div class="tip"><strong>Dica:</strong> comentários explicam o motivo de uma decisão. Prefira uma frase curta que acrescente algo ao próprio código.</div>
      <p>Funções recebem parâmetros com <code>Recebe</code>, separam nomes por vírgula (ou por <code>e</code>) e encerram a lista com ponto. Exemplo da declaração:</p>
      <pre><code>Função Calcular Recebe Valor e Percentual.
    Retorna Valor * Percentual.</code></pre>
      <div class="challenge"><span class="challenge-label">DESAFIO</span><p>Altere os valores passados a <code>Calcular</code> e confira o resultado. Depois, mova a chamada para fora dos marcadores de snippet e veja que eles não mudam a execução.</p></div>`,
    code: `Programa "Calculadora".

Variaveis
    Resultado: Real.

Função Calcular Recebe Valor e Percentual.
    Retorna Valor * Percentual.

Inicio
    (O comentário não altera o cálculo.)
    /*
    Resultado <- Calcular Recebe 200 e 0.15.
    */
    Escreva Resultado.
Fim.`,
    stdin: ""
  },
  {
    module: "Dados e decisões",
    title: "Variáveis e tipos",
    duration: "8 min",
    lead: "Variáveis guardam valores que o programa pode consultar e alterar. Cada variável é declarada com um tipo, que descreve o dado armazenado.",
    body: `<h2>Escolha o tipo de dado</h2>
      <div class="concept-row"><div class="concept-card"><strong>inteiro</strong><span>Números sem casas decimais, como 12.</span></div><div class="concept-card"><strong>real</strong><span>Números com casas decimais, como 1.75.</span></div><div class="concept-card"><strong>cadeia</strong><span>Texto entre aspas duplas.</span></div><div class="concept-card"><strong>logico</strong><span>Um valor <code>verdadeiro</code> ou <code>falso</code>.</span></div></div>
      <p>Declare a variável antes de usá-la. Você pode atribuir um valor na declaração ou mais tarde com <code>=</code>.</p>
      <div class="challenge"><span class="challenge-label">DESAFIO</span><p>Crie uma variável do tipo <code>inteiro</code> para guardar sua idade e mostre-a na tela.</p></div>`,
    code: `programa "DadosPessoais" {
    funcao inicio {
        cadeia Nome = "Lia"
        inteiro Idade = 16
        real Altura = 1.68
        logico Estudante = verdadeiro

        escreval "Nome: ", Nome
        escreval "Idade: ", Idade
        escreval "Altura: ", Altura
        escreval "Estudante: ", Estudante
    }
}`,
    stdin: ""
  },
  {
    module: "Dados e decisões",
    title: "Contas e expressões",
    duration: "7 min",
    lead: "Expressões combinam valores e operadores para produzir um resultado. Elas permitem que o programa faça cálculos em vez de apenas mostrar valores fixos.",
    body: `<h2>Operadores básicos</h2>
      <div class="concept-row"><div class="concept-card"><strong>+ &nbsp; -</strong><span>Adição e subtração.</span></div><div class="concept-card"><strong>* &nbsp; / &nbsp; %</strong><span>Multiplicação, divisão e resto.</span></div></div>
      <p>Multiplicação, divisão e resto são avaliados antes de adição e subtração. O operador <code>%</code> devolve o resto da divisão inteira: <code>17 % 5</code> resulta em <code>2</code>.</p>
      <div class="tip">Agrupamento com parênteses ainda não está implementado. Use a precedência dos operadores para escrever a expressão desejada.</div>
      <div class="challenge"><span class="challenge-label">DESAFIO</span><p>Altere os valores e calcule o perímetro de um retângulo: largura + largura + altura + altura.</p></div>`,
    code: `programa "Calculos" {
    funcao inicio {
        inteiro Largura = 7
        inteiro Altura = 3
        inteiro Area = Largura * Altura
        inteiro Resto = Largura % Altura

        escreval "Area: ", Area
        escreval "Resto: ", Resto
    }
}`,
    stdin: ""
  },
  {
    module: "Dados e decisões",
    title: "Receba dados com leia",
    duration: "8 min",
    lead: "Programas ficam mais úteis quando respondem a dados fornecidos por quem os usa. A instrução <code>leia</code> recebe valores pela entrada padrão.",
    body: `<h2>Entrada e saída</h2>
      <p>Declare as variáveis antes de chamar <code>leia</code>. Você pode ler uma variável por instrução ou separar várias por vírgula. Escreva os valores na caixa de entrada, na mesma ordem.</p>
      <p>Este exemplo recebe dois números e mostra a soma. A entrada abaixo já vem preenchida com <code>12</code> e <code>8</code>.</p>
      <div class="challenge"><span class="challenge-label">DESAFIO</span><p>Troque os valores de entrada e faça o programa calcular também a diferença.</p></div>`,
    code: `programa "Soma" {
    funcao inicio {
        inteiro Primeiro
        inteiro Segundo
        inteiro Resultado

        escreva "Digite dois numeros: "
        leia Primeiro, Segundo
        Resultado = Primeiro + Segundo
        escreval "Soma: ", Resultado
    }
}`,
    stdin: "12\n8\n"
  },
  {
    module: "Dados e decisões",
    title: "Escolhas com se e senao",
    duration: "8 min",
    lead: "Condições permitem que um programa escolha entre caminhos. Uma condição é uma expressão lógica: ela é verdadeira ou falsa.",
    body: `<h2>Compare e escolha</h2>
      <p>Operadores como <code>==</code>, <code>&gt;</code>, <code>&lt;=</code> comparam valores. Use <code>se</code> para executar um bloco quando a condição for verdadeira e <code>senao</code> para o outro caso.</p>
      <p>Combine condições com <code>e</code>, <code>ou</code> e <code>nao</code>. O programa abaixo usa o resto da divisão por dois para classificar o número.</p>
      <div class="challenge"><span class="challenge-label">DESAFIO</span><p>Adicione uma condição para dizer se o número é zero antes de verificar se é par ou ímpar.</p></div>`,
    code: `programa "ParOuImpar" {
    funcao inicio {
        inteiro Numero

        escreva "Digite um numero: "
        leia Numero

        se Numero % 2 = 0 {
            escreval "Par"
        } senao {
            escreval "Impar"
        }
    }
}`,
    stdin: "17\n"
  },
  {
    module: "Repetição",
    title: "Repita com enquanto",
    duration: "8 min",
    lead: "Laços repetem instruções. O laço <code>enquanto</code> continua executando enquanto a condição for verdadeira.",
    body: `<h2>Evite um laço infinito</h2>
      <p>Atualize dentro do laço algum valor usado na condição. Neste exemplo, <code>contador</code> aumenta até chegar ao limite e então o laço termina.</p>
      <div class="tip">Antes de executar, pergunte: o que muda a cada repetição? Em que momento a condição deixa de ser verdadeira?</div>
      <div class="challenge"><span class="challenge-label">DESAFIO</span><p>Altere o programa para contar de 10 até 1 e, no final, escrever “FIM”.</p></div>`,
    code: `programa "ContagemEnquanto" {
    funcao inicio {
        inteiro Contador = 1

        enquanto Contador <= 5 {
            escreval Contador
            Contador = Contador + 1
        }
    }
}`,
    stdin: ""
  },
  {
    module: "Repetição",
    title: "Contagem com para",
    duration: "8 min",
    lead: "Quando você conhece o início, o fim e o passo de uma contagem, o laço <code>para</code> reúne essas três partes no cabeçalho.",
    body: `<h2>Três partes do laço</h2>
      <ul><li>inicialização: cria o contador;</li><li>condição: define quando parar;</li><li>incremento: avança a cada volta.</li></ul>
      <p>As partes do cabeçalho são separadas por ponto e vírgula. O operador <code>++</code> aumenta o contador em um.</p>
      <div class="challenge"><span class="challenge-label">DESAFIO</span><p>Leia um limite da entrada e use-o como condição do laço.</p></div>`,
    code: `programa "Tabuada" {
    funcao inicio {
        inteiro Numero = 6

        para inteiro Fator = 1; Fator <= 5; Fator++ {
            escreval Numero, " x ", Fator, " = ", Numero * Fator
        }
    }
}`,
    stdin: ""
  },
  {
    module: "Repetição",
    title: "Combine lógica e laços",
    duration: "9 min",
    lead: "Programas mais interessantes combinam ideias: percorrer uma sequência, testar cada valor e decidir o que fazer.",
    body: `<h2>Controle a repetição</h2>
      <p><code>continue</code> pula para a próxima volta do laço. <code>pare</code> encerra o laço imediatamente. Use-os dentro de um laço.</p>
      <p>O exemplo mostra apenas números ímpares entre 1 e 10. Quando o resto por dois é zero, <code>continue</code> pula a impressão.</p>
      <div class="challenge"><span class="challenge-label">DESAFIO</span><p>Mude a condição para mostrar apenas múltiplos de três.</p></div>`,
    code: `programa "Impares" {
    funcao inicio {
        para inteiro Numero = 1; Numero <= 10; Numero++ {
            se Numero % 2 = 0 {
                continue
            }
            escreval Numero
        }
    }
}`,
    stdin: ""
  },
  {
    module: "Projeto final",
    title: "Projeto: média da turma",
    duration: "12 min",
    lead: "Você já conhece os blocos básicos. Agora combine entrada, variáveis, cálculos e condições para criar um programa completo.",
    body: `<h2>Planeje antes de codificar</h2>
      <p>O programa recebe duas notas, calcula a média e informa se a pessoa foi aprovada. Decomponha o problema em passos:</p>
      <ul><li>declarar as notas e a média;</li><li>ler as duas notas;</li><li>calcular a média;</li><li>comparar com a nota mínima.</li></ul>
      <p>A média é escrita como a soma de cada nota dividida por dois. Isso evita agrupamento com parênteses, que ainda não faz parte da gramática.</p>
      <div class="challenge"><span class="challenge-label">PROJETO</span><p>Execute com outras notas. Depois, mude a nota mínima ou estenda o programa para receber uma terceira nota.</p></div>`,
    code: `programa "MediaDaTurma" {
    funcao inicio {
        real Nota1
        real Nota2
        real Media

        escreva "Primeira nota: "
        leia Nota1
        escreva "Segunda nota: "
        leia Nota2

        Media = Nota1 / 2.0 + Nota2 / 2.0
        escreval "Media: ", Media

        se Media >= 6.0 {
            escreval "Aprovado"
        } senao {
            escreval "Continue estudando"
        }
    }
}`,
    stdin: "8\n10\n"
  }
];

const nav = document.querySelector("#lesson-nav");
const title = document.querySelector("#lesson-title");
const content = document.querySelector("#lesson-content");
const editor = document.querySelector("#code-editor");
const stdin = document.querySelector("#stdin-input");
const output = document.querySelector("#program-output");
const status = document.querySelector("#run-status");
const runButton = document.querySelector("#run-button");
const done = new Set(JSON.parse(localStorage.getItem("docscript-course-done") || "[]"));
let activeIndex = Math.max(0, Math.min(lessons.length - 1, Number(localStorage.getItem("docscript-course-active") || 0)));

function updateProgress() {
  const count = done.size;
  document.querySelector("#progress-label").textContent = `${count} de ${lessons.length} aulas concluídas`;
  document.querySelector("#progress-fill").style.width = `${count / lessons.length * 100}%`;
}

function renderNav() {
  nav.replaceChildren();
  let moduleName = "";
  lessons.forEach((lesson, index) => {
    if (lesson.module !== moduleName) {
      moduleName = lesson.module;
      const heading = document.createElement("div");
      heading.className = "module-title";
      heading.textContent = moduleName;
      nav.append(heading);
    }
    const button = document.createElement("button");
    button.className = `lesson-link${index === activeIndex ? " active" : ""}${done.has(index) ? " done" : ""}`;
    button.type = "button";
    const number = document.createElement("span");
    number.className = "lesson-index";
    number.textContent = done.has(index) ? "✓" : String(index + 1).padStart(2, "0");
    const label = document.createElement("span");
    label.textContent = lesson.title;
    button.append(number, label);
    button.addEventListener("click", () => loadLesson(index));
    nav.append(button);
  });
}

function loadLesson(index) {
  activeIndex = index;
  const lesson = lessons[index];
  localStorage.setItem("docscript-course-active", String(index));
  document.querySelector("#crumb-module").textContent = lesson.module.toLocaleUpperCase("pt-BR");
  document.querySelector("#lesson-number").textContent = `AULA ${String(index + 1).padStart(2, "0")}`;
  document.querySelector("#lesson-duration").textContent = lesson.duration.toLocaleUpperCase("pt-BR");
  title.textContent = lesson.title;
  content.innerHTML = `<p class="lead">${lesson.lead}</p>${lesson.body}`;
  const savedCode = localStorage.getItem(`docscript-course-code-${index}`);
  const savedInput = localStorage.getItem(`docscript-course-input-${index}`);
  editor.value = savedCode === null ? lesson.code : savedCode;
  stdin.value = savedInput === null ? lesson.stdin : savedInput;
  output.textContent = "A saída do programa aparecerá aqui.";
  output.classList.add("placeholder");
  setStatus("Pronto para executar", "");
  document.querySelector("#complete-button").textContent = done.has(index) ? "✓ Aula concluída" : "Marcar como concluída";
  document.querySelector("#complete-button").classList.toggle("completed", done.has(index));
  document.querySelector("#previous-button").disabled = index === 0;
  document.querySelector("#next-button").innerHTML = index === lessons.length - 1 ? "Curso concluído <span>✓</span>" : "Próxima aula <span>→</span>";
  renderNav();
  updateProgress();
  updateLineNumbers();
  document.querySelector("#sidebar").classList.remove("open");
  document.querySelector("#menu-toggle").setAttribute("aria-expanded", "false");
}

function setStatus(text, kind) {
  status.textContent = text;
  status.className = `run-status${kind ? ` ${kind}` : ""}`;
}

function updateLineNumbers() {
  const count = editor.value.split("\n").length;
  document.querySelector("#line-numbers").textContent = Array.from({length: count}, (_, i) => i + 1).join("\n");
}

editor.addEventListener("input", () => {
  localStorage.setItem(`docscript-course-code-${activeIndex}`, editor.value);
  updateLineNumbers();
});
stdin.addEventListener("input", () => localStorage.setItem(`docscript-course-input-${activeIndex}`, stdin.value));

document.querySelector("#reset-button").addEventListener("click", () => {
  localStorage.removeItem(`docscript-course-code-${activeIndex}`);
  localStorage.removeItem(`docscript-course-input-${activeIndex}`);
  loadLesson(activeIndex);
});

document.querySelector("#complete-button").addEventListener("click", () => {
  if (done.has(activeIndex)) done.delete(activeIndex);
  else done.add(activeIndex);
  localStorage.setItem("docscript-course-done", JSON.stringify([...done]));
  loadLesson(activeIndex);
});

document.querySelector("#previous-button").addEventListener("click", () => {
  if (activeIndex > 0) loadLesson(activeIndex - 1);
});
document.querySelector("#next-button").addEventListener("click", () => {
  if (activeIndex < lessons.length - 1) loadLesson(activeIndex + 1);
  else {
    done.add(activeIndex);
    localStorage.setItem("docscript-course-done", JSON.stringify([...done]));
    loadLesson(activeIndex);
  }
});

runButton.addEventListener("click", runProgram);
editor.addEventListener("keydown", (event) => {
  if (event.key === "Enter" && (event.ctrlKey || event.metaKey)) {
    event.preventDefault();
    runProgram();
  }
  if (event.key === "Tab") {
    event.preventDefault();
    const start = editor.selectionStart;
    editor.setRangeText("    ", start, editor.selectionEnd, "end");
    editor.dispatchEvent(new Event("input"));
  }
});

async function runProgram() {
  runButton.disabled = true;
  setStatus("Compilando…", "");
  output.classList.remove("placeholder");
  output.textContent = "Executando seu programa…";
  try {
    const response = await fetch("/api/compile", {
      method: "POST",
      headers: {"Content-Type": "application/json"},
      body: JSON.stringify({code: editor.value, stdin: stdin.value, mode: "run"})
    });
    const result = await response.json();
    if (!response.ok) throw new Error(`Servidor respondeu ${response.status}`);
    const sections = [];
    if (result.stdout) sections.push(result.stdout.replace(/\n$/, ""));
    if (result.stderr) sections.push(`stderr:\n${result.stderr.replace(/\n$/, "")}`);
    if (result.diagnostics) sections.push(result.diagnostics);
    if (!sections.length) sections.push(result.ok ? "Programa executado sem saída." : "Não foi possível compilar o programa.");
    output.textContent = sections.join("\n\n");
    if (result.timedOut) setStatus("Tempo limite excedido", "error");
    else if (!result.ok) setStatus("Erro de compilação", "error");
    else if (result.exitCode !== 0) setStatus(`Programa terminou com código ${result.exitCode}`, "error");
    else setStatus("Executado com sucesso", "success");
  } catch (error) {
    output.textContent = `${error.message}\nInicie o compilador local com: ./docscript serve`;
    setStatus("Servidor indisponível", "error");
  } finally {
    runButton.disabled = false;
  }
}

document.querySelector("#menu-toggle").addEventListener("click", (event) => {
  const open = document.querySelector("#sidebar").classList.toggle("open");
  event.currentTarget.setAttribute("aria-expanded", String(open));
});

loadLesson(activeIndex);
