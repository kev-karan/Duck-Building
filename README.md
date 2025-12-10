# 🦆 Duck-Building (Simulador ESG)

**Duck-Building** é um jogo de quiz interativo e multijogador desenvolvido para Arduino, focado em temas de **ESG** (Environmental, Social, and Governance) e sobrevivência no mundo corporativo.

O projeto foi **unificado** para correr num único módulo **Arduino Mega**, que gere simultaneamente a lógica do jogo, a renderização gráfica no ecrã TFT e a leitura dos botões físicos.

---

## 🎮 Funcionalidades

* **Multiplayer Local:** Suporte para 1 a 3 jogadores.
* **Sistema de Carreira:** Comece como *Estagiário* e suba de cargo (Analista, Especialista, Coordenador, Gerente) até se tornar **CEO**.
* **Personagens Temáticos:** Jogue com personagens do universo "Duck", como *Mr. Ducks*, *Patycia*, *Josepatu* e outros.
* **Interface Unificada:** Todo o jogo ocorre num único ecrã, facilitando a montagem e o transporte.
* **Eventos Canônicos:** A cada 5 minutos, ocorre um "Evento em Conjunto" que pode mudar a dinâmica.
* **Perguntas Dinâmicas:** 35 cenários corporativos com escolhas éticas e estratégicas.

---

## 🛠️ Hardware Necessário

Para montar o sistema, precisará apenas de:

1. **Microcontrolador:** 1x Arduino Mega 2560.
2. **Display:** 1x TFT Touch Shield (compatível com a biblioteca `MCUFRIEND_kbv`).
3. **Controlos:** 3x Botões de pressão (Push-buttons).
4. **Conexão:** Protoboard e fios (Jumpers).

---

## 🔌 Esquema de Ligação

Os botões devem ser ligados diretamente aos pinos digitais do Arduino Mega (geralmente acessíveis na lateral do shield TFT). Utilize a lógica *INPUT_PULLUP* (um pino do botão vai ao Arduino, o outro ao GND):

| Função | Pino no Arduino Mega |
| :--- | :--- |
| **CIMA (UP)** | Pino 31 |
| **BAIXO (DOWN)** | Pino 33 |
| **OK / CONFIRMA** | Pino 35 |

> **Nota:** Certifique-se de ligar o terminal comum dos botões ao pino **GND** do Arduino.

---

## 🚀 Instalação e Upload

1. **Biblioteca:** Instale a biblioteca **MCUFRIEND_kbv** através do Gerenciador de Bibliotecas da Arduino IDE.
2. **Código:** Abra o ficheiro `predio.ino`.
3. **Upload:** Conecte o seu Arduino Mega ao PC e carregue o código.
4. **Jogar:** O jogo iniciará automaticamente no ecrã TFT.

---

## 🎲 Como Jogar

1. **Configuração:** No menu inicial, use os botões **CIMA/BAIXO** para selecionar o número de jogadores (1-3) e pressione **OK**.
2. **Sorteio:** O sistema sorteará aleatoriamente um personagem para cada jogador.
3. **Rodadas:**
    * O jogo indica de quem é a vez (J1, J2, J3...) de forma sequencial.
    * Uma pergunta aparece com 3 alternativas (A, B, C).
    * Use **CIMA/BAIXO** para navegar e **OK** para confirmar a resposta.
4. **Pontuação:**
    * Resposta correta: **+5 pontos**.
    * Atingir certas pontuações gera uma **Promoção de Cargo** imediata.
5. **Vitória:** O primeiro jogador a atingir **50 pontos** (Cargo de CEO) vence o jogo!.

## 📄 Licença

Este projeto está licenciado sob a licença **MIT** - veja o ficheiro [LICENSE](LICENSE) para mais detalhes.
