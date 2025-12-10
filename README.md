# 🦆 Duck-Building (Simulador ESG)

**Duck-Building** é um jogo de quiz interativo e multijogador desenvolvido para Arduino, focado em temas de **ESG** (Environmental, Social, and Governance) e sobrevivência no mundo corporativo.

O projeto utiliza **dois módulos Arduino** comunicando-se via Serial: uma unidade central ("Prédio") que gerencia o jogo e exibe o tabuleiro principal, e um controlador ("Mão") que serve como interface para os jogadores responderem às perguntas e verem seus status individuais.

---

## 🎮 Funcionalidades

* **Multiplayer Local:** Suporte para 1 a 3 jogadores.
* **Sistema de Carreira:** Comece como *Estagiário* e suba de cargo (Analista, Especialista, Coordenador, Gerente) até se tornar **CEO**.
* **Personagens Temáticos:** Jogue com personagens do universo "Duck", como *Mr. Ducks*, *Patycia*, *Josepatu* e outros.
* **Dual Screen:**
  * **Tela do Prédio:** Mostra a pergunta, o ranking geral e eventos globais.
  * **Tela da Mão:** Interface pessoal para seleção de respostas, feedback de acerto/erro e notificações de promoção.
* **Eventos Canônicos:** A cada 5 minutos, ocorre um "Evento em Conjunto" que muda a dinâmica das perguntas.
* **Perguntas Dinâmicas:** 35 cenários corporativos com escolhas éticas e estratégicas.

---

## 🛠️ Hardware Necessário

Para montar o sistema completo, você precisará de:

1. **Módulo Central (Prédio):**
    * 1x Arduino (Recomendado Arduino Mega devido ao uso de `Serial1` e memória).
    * 1x Display TFT Touch (compatível com a biblioteca `MCUFRIEND_kbv`).
2. **Módulo Controlador (Mão):**
    * 1x Arduino (Mega ou similar).
    * 1x Display TFT Touch (compatível com a biblioteca `MCUFRIEND_kbv`).
    * 3x Botões (Push-buttons).
3. **Conexão:**
    * Fios para conexão Serial (TX/RX) entre os dois Arduinos.
    * Protoboard e Jumpers.

---

## 🔌 Esquema de Ligação

### 1. Conexão entre Arduinos (Comunicação)

Os dois Arduinos comunicam-se a uma taxa de **9600 baud**. Conecte os pinos cruzados:

* **Prédio TX1** -> **Mão RX1**
* **Prédio RX1** -> **Mão TX1**
* **GND** -> **GND** (Essencial para referência comum).

### 2. Botões no Controlador (Mão)

Os botões devem ser conectados aos pinos digitais com lógica *INPUT_PULLUP* (o outro lado do botão vai para o GND):

| Função | Pino no Arduino (Mão) |
| :--- | :--- |
| **CIMA (UP)** | Pino 31 |
| **BAIXO (DOWN)** | Pino 33 |
| **OK / CONFIRMA** | Pino 35 |

---

## 🚀 Instalação e Upload

1. Instale a biblioteca **MCUFRIEND_kbv** através do Gerenciador de Bibliotecas da Arduino IDE.
2. Abra o arquivo `predio.ino` e faça o upload para o Arduino que servirá como **Central**.
3. Abra o arquivo `mao.ino` e faça o upload para o Arduino que servirá como **Controlador**.
4. Ligue ambos os dispositivos. O sistema possui um "Heartbeat" (sincronização) que conecta automaticamente as telas após alguns segundos.

---

## 🎲 Como Jogar

1. **Configuração:** Na tela inicial, use os botões da Mão para selecionar o número de jogadores (1-3) e pressione OK.
2. **Sorteio:** O sistema sorteará aleatoriamente um personagem para cada jogador.
3. **Rodadas:**
    * O jogo indicará de quem é a vez.
    * Uma pergunta aparecerá na tela do Prédio com 3 alternativas (A, B, C).
    * O jogador usa **CIMA/BAIXO** na Mão para selecionar a alternativa e **OK** para confirmar.
4. **Pontuação:**
    * Resposta correta: **+5 pontos**.
    * Atingir certas pontuações gera uma **Promoção de Cargo** imediata.
5. **Vitória:** O primeiro jogador a atingir **50 pontos** (Cargo de CEO) vence o jogo!.

---

## 📂 Estrutura do Código

* **`predio.ino`**: Contém toda a lógica do jogo, banco de perguntas (armazenado em `PROGMEM` para economizar memória), regras de pontuação e renderização da tela principal.
* **`mao.ino`**: Atua como um terminal "burro" inteligente. Lê os botões, envia os comandos para o prédio e renderiza o feedback pessoal recebido via Serial.

---

## 📄 Licença

Este projeto está licenciado sob a licença **MIT** - veja o arquivo [LICENSE](LICENSE) para mais detalhes.
