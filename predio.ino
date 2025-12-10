// Código Unificado: Arduino Mega (Lógica do Prédio + Leitura da Mão)
// Usa uma única tela LCD e botões para controlar o jogo.

#include <MCUFRIEND_kbv.h>
#include <avr/pgmspace.h>
#include <string.h>
#include <stdlib.h> // Necessário para atoi (embora menos usado agora)

MCUFRIEND_kbv tft;

// Cores basicas (565)
#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF

// --- CONSTANTES DE LAYOUT (UI) ---
const int PADDING_X = 10;
const int LINE_HEIGHT = 25;
const int TITLE_X = 30;
const int TITLE_Y = 40;
const int SUBTITLE_Y = 90;
const int SCORE_TEXT_Y = 0;
const int QUESTION_START_Y = 70;
const int OPTION_SPACING = 50; // Espaçamento maior entre alternativas

// --- PINOS E DEBOUNCE DOS BOTOES (do código da Mão) ---
const int BTN_UP = 31;
const int BTN_DOWN = 33;
const int BTN_OK = 35;

bool lastUp = HIGH;
bool lastDown = HIGH;
bool lastOk = HIGH;

unsigned long lastDebounceMs = 0;
const unsigned long DEBOUNCE_MS = 150;

// --- REGRAS DO JOGO ---
#define MAX_PLAYERS 3
#define NUM_QUESTOES 35
#define PONTOS_ACERTO 5
#define PONTOS_VITORIA 50
#define INTERVALO_EVENTO_MS (5UL * 60UL * 1000UL) // 5 minutos

// Variaveis de Heartbeat (Não é mais usado para comunicação serial, mas mantido para referência)
unsigned long lastHeartbeat = 0;
const unsigned long HEARTBEAT_INTERVAL = 2000;

// ---------- PERSONAGENS (somente nomes, em PROGMEM) ----------
const char pn0[] PROGMEM = "Mr Ducks";
const char pn1[] PROGMEM = "Patycia";
const char pn2[] PROGMEM = "Josepatu";
const char pn3[] PROGMEM = "Ana Patarina";
const char pn4[] PROGMEM = "Patrucio";
const char pn5[] PROGMEM = "Patilde";
const char pn6[] PROGMEM = "Patucho";
const char pn7[] PROGMEM = "Patina";

const char *const personagemNomes[] PROGMEM = {
    pn0, pn1, pn2, pn3, pn4, pn5, pn6, pn7};

// ---------- PERGUNTAS (35) EM PROGMEM ----------
// Formato: "Pergunta|A) ...|B) ...|C) ..."
const char q0[] PROGMEM = "Reuniao de alinhamento inesperada|A) Explicar rapidamente os pontos principais|B) Preparar apresentacao detalhada e longa|C) Deixar o colega da esquerda falar sozinho";
const char q1[] PROGMEM = "Email de cliente confuso|A) Responder de forma educada e clara|B) Ignorar ate alguem reclamar|C) Responder com irritacao";
const char q2[] PROGMEM = "Conflito interno sobre prioridades|A) Mediar conversa buscando acordo|B) Forcar sua ideia sem ouvir ninguem|C) Sair da discussao e deixar rolar";
const char q3[] PROGMEM = "Feedback negativo sobre apresentacao|A) Culpar o publico pela critica|B) Pedir feedback detalhado para melhorar|C) Fingir que nada aconteceu";
const char q4[] PROGMEM = "Mal entendido com colega|A) Conversar diretamente para esclarecer|B) Falar dele para o chefe|C) Criar fofoca no corredor";
const char q5[] PROGMEM = "Reuniao surpresa com cliente importante|A) Entrar sem se preparar|B) Pedir alguns minutos para alinhar e organizar|C) Mandar outra pessoa ir no seu lugar sem avisar";
const char q6[] PROGMEM = "Feedback ambiguo do gerente|A) Perguntar o que exatamente ele espera|B) Supor que esta tudo bem e continuar igual|C) Reclamar com colegas sem falar com ele";
const char q7[] PROGMEM = "Apresentacao que deu errado|A) Refazer destacando resultados e aprendizados|B) Culpar a equipe em publico|C) Esconder os erros nos proximos relatorios";
const char q8[] PROGMEM = "Desalinhamento entre times|A) Marcar chamada rapida com todos|B) Deixar cada time trabalhar isolado|C) Mandar email passivo agressivo";
const char q9[] PROGMEM = "Cliente pede detalhamento urgente|A) Entregar qualquer coisa so para ser rapido|B) Organizar informacoes e enviar resumo claro|C) Dizer que esta ocupado e nao responder";

const char q10[] PROGMEM = "Brainstorm inesperado|A) Trazer ideias novas e diferentes|B) Ficar calado e copiar ideias dos outros|C) Criticar qualquer ideia sem sugerir nada";
const char q11[] PROGMEM = "Desafio de inovacao rapida|A) Criar prototipo simples para testar|B) Recusar porque nao esta perfeito|C) Esperar outra pessoa comecar";
const char q12[] PROGMEM = "Problema de design no material|A) Propor solucao alternativa|B) Ignorar e seguir com o problema|C) Culpar o time de marketing";
const char q13[] PROGMEM = "Ideia rejeitada pelo gerente|A) Reformular a ideia com melhorias|B) Guardar rancor e parar de sugerir|C) Reclamar da decisao para o time inteiro";
const char q14[] PROGMEM = "Sugestao para otimizar processo|A) Sugerir melhoria com foco em ESG|B) Propor atalho que ignora regras|C) Fazer apenas o minimo exigido";
const char q15[] PROGMEM = "Evento anti etico: apropriacao de ideia|A) Apresentar ideia do colega como sua|B) Creditar a equipe e o colega|C) Mudar pequenos detalhes e dizer que e sua";
const char q16[] PROGMEM = "Evento anti etico: ignorar contribuicoes|A) Falar so do seu trabalho no relatorio|B) Destacar o que cada pessoa contribuiu|C) Apagar o nome dos colegas";
const char q17[] PROGMEM = "Evento anti etico: exagerar inovacao|A) Inventar resultados que ainda nao existem|B) Comunicar com transparencia o que ja foi testado|C) Prometer algo que sabe que nao sera entregue";
const char q18[] PROGMEM = "Workshop criativo|A) Participar ativamente e ouvir os outros|B) Olhar o celular o tempo todo|C) Zombar das ideias diferentes";
const char q19[] PROGMEM = "Revisao de prototipo|A) Testar e ajustar com o time|B) Liberar sem revisar|C) Ignorar feedback de usuarios";

const char q20[] PROGMEM = "Novo projeto urgente|A) Ajudar a organizar tarefas com o time|B) Fingir que nao viu a mensagem|C) Jogar todo o trabalho no estagiario";
const char q21[] PROGMEM = "Solicitacao inesperada de cliente|A) Ver o que e possivel entregar com qualidade|B) Prometer tudo sem avaliar|C) Dizer que nao e problema seu";
const char q22[] PROGMEM = "Prazo muito apertado|A) Priorizar atividades mais importantes|B) Atrasar o resto da equipe|C) Entregar qualquer coisa sem qualidade";
const char q23[] PROGMEM = "Problema tecnico de ultima hora|A) Pedir ajuda e organizar correcao|B) Esconder o problema e torcer|C) Culpar o fornecedor sem investigar";
const char q24[] PROGMEM = "Evento de melhoria continua|A) Trazer sugestao concreta|B) Reclamar que nada muda|C) Ficar quieto para nao dar trabalho";
const char q25[] PROGMEM = "Evento anti etico: pular revisao|A) Aplicar mudanca sem nenhum teste|B) Seguir processo de revisao mesmo com pressa|C) Alterar tudo e nao contar para ninguem";
const char q26[] PROGMEM = "Evento anti etico: tomar credito indevido|A) Assumir resultado da equipe sozinho|B) Dividir merito com todos envolvidos|C) Dizer que so voce trabalhou duro";
const char q27[] PROGMEM = "Evento anti etico: pressionar equipe injustamente|A) Cobrar com respeito e combinar prazos|B) Amedrontar com ameaca de demissao|C) Ignorar limites de horario sempre";
const char q28[] PROGMEM = "Treinamento interno voluntario|A) Participar e compartilhar conhecimento|B) Dizer que e perda de tempo|C) Ir mas ficar sem prestar atencao";
const char q29[] PROGMEM = "Reuniao de planejamento rapido|A) Ajudar a organizar agenda e prioridades|B) Ficar calado e depois reclamar|C) Sabotar decisoes combinadas";

const char q30[] PROGMEM = "Problema critico no codigo|A) Corrigir e documentar a solucao|B) Fazer um jeitinho rapido sem registrar|C) Culpar outro setor sem analisar";
const char q31[] PROGMEM = "Atualizacao urgente de sistema|A) Testar antes de colocar em producao|B) Atualizar direto em producao sem teste|C) Ignorar pedido de atualizacao";
const char q32[] PROGMEM = "Evento anti etico: manipular logs|A) Alterar registros para esconder erro|B) Registrar erro e corrigir com transparencia|C) Apagar qualquer evidencia do problema";
const char q33[] PROGMEM = "Evento anti etico: usar codigo sem credito|A) Copiar codigo de colega e assinar|B) Referenciar quem criou e contribuir|C) Fingir que nao sabia da origem";
const char q34[] PROGMEM = "Deploy de versao importante|A) Planejar, testar e monitorar|B) Subir na sexta sem nenhum teste|C) Fazer deploy escondido para ninguem ver";

const char *const perguntas[NUM_QUESTOES] PROGMEM = {
    q0, q1, q2, q3, q4, q5, q6, q7, q8, q9,
    q10, q11, q12, q13, q14, q15, q16, q17, q18, q19,
    q20, q21, q22, q23, q24, q25, q26, q27, q28, q29,
    q30, q31, q32, q33, q34};

// Resposta correta de cada pergunta (0=A, 1=B, 2=C)
const uint8_t respostasCorretas[NUM_QUESTOES] PROGMEM = {
    // 0-9
    0, 0, 0, 1, 0, 1, 0, 0, 0, 1,
    // 10-19
    0, 0, 0, 0, 0, 1, 1, 1, 0, 0,
    // 20-29
    0, 0, 0, 0, 0, 1, 1, 0, 0, 0,
    // 30-34
    0, 0, 1, 1, 0};

// ---------- ESTADO DO JOGO ----------
enum GameState
{
  STATE_SHOW_TITLE,
  STATE_SELECT_PLAYERS,
  STATE_SORT_CHARACTERS,
  STATE_WAIT_ANSWER,
  STATE_SHOW_FEEDBACK,
  STATE_SHOW_RANK,
  STATE_SHOW_WINNER
};

GameState state = STATE_SHOW_TITLE;

uint8_t numPlayers = 1;
uint8_t scores[MAX_PLAYERS];
uint8_t characters[MAX_PLAYERS];
uint8_t questionOrder[NUM_QUESTOES];
uint8_t questionPos = 0;
uint8_t currentQuestionIndex = 0;
uint8_t currentPlayer = 0;
uint8_t selectedOption = 0;

// Controle de cargos
uint8_t lastRanks[MAX_PLAYERS] = {0, 0, 0};
uint8_t newRank = 0;
uint8_t rankPlayer = 0;

unsigned long lastEventoMs = 0;
bool isEventoCanonico = false;

// --- PROTOTIPOS ---
void getProgmemString(const char *const *table, uint8_t index, char *dest, size_t len);
void drawTitle();
void drawSelectPlayers();
void drawSelectPlayersValue();
void shuffleQuestions();
void sortCharacters();
void drawCharacters();
void drawScores();
void startNextQuestion();
void drawQuestionScreen();
void drawQuestionText();
void drawFeedback(bool acerto);
void drawRankPromotion();
void drawWinner(uint8_t winner);
void handleButton(char c); // Agora processa o char c
uint8_t findHighestScorePlayer();
uint8_t calculateRank(uint8_t score);
const char *getCargoName(uint8_t rank);
bool checkRankPromotion(uint8_t player);
void readButtons(); // Função da Mão para ler botões físicos

// ---------- FUNCOES AUXILIARES ----------
void getProgmemString(const char *const *table, uint8_t index, char *dest, size_t len)
{
  strncpy_P(dest, (PGM_P)pgm_read_word(&(table[index])), len);
  dest[len - 1] = '\0';
}

uint8_t calculateRank(uint8_t score)
{
  if (score >= 50)
    return 5; // CEO
  if (score >= 40)
    return 4; // Gerente
  if (score >= 30)
    return 3; // Coordenador
  if (score >= 20)
    return 2; // Especialista
  if (score >= 10)
    return 1; // Analista
  return 0;   // Estagiario
}

const char *getCargoName(uint8_t rank)
{
  switch (rank)
  {
  case 0:
    return "Estagiario";
  case 1:
    return "Analista";
  case 2:
    return "Especialista";
  case 3:
    return "Coordenador";
  case 4:
    return "Gerente";
  case 5:
    return "CEO";
  default:
    return "Estagiario";
  }
}

bool checkRankPromotion(uint8_t player)
{
  uint8_t currentRank = calculateRank(scores[player]);
  if (currentRank > lastRanks[player])
  {
    lastRanks[player] = currentRank;
    newRank = currentRank;
    rankPlayer = player;
    return true;
  }
  return false;
}

// ---------- SETUP ----------
void setup()
{
  Serial.begin(9600); // opcional debug

  // Configuração dos Pinos dos Botões
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_OK, INPUT_PULLUP);

  uint16_t ID = tft.readID();
  if (ID == 0xD3D3)
    ID = 0x9481; // fallback comum
  tft.begin(ID);
  tft.setRotation(1);
  tft.fillScreen(BLACK);

  randomSeed(analogRead(A0));

  memset(scores, 0, sizeof(scores));
  memset(lastRanks, 0, sizeof(lastRanks));
  lastEventoMs = millis();

  drawTitle();
}

// ---------- LOOP ----------
void loop()
{
  // Substitui a leitura Serial1 pela leitura direta dos botões
  readButtons();

  // O heartbeat (Serial1) não é mais necessário, mas mantemos o ciclo para não travar
  if (millis() - lastHeartbeat > HEARTBEAT_INTERVAL)
  {
    // Código removido: Serial1.println("K");
    lastHeartbeat = millis();
  }
}

// Função da Mão integrada: Lê os botões físicos e chama a lógica de estado (handleButton)
void readButtons()
{
  unsigned long now = millis();

  bool up = digitalRead(BTN_UP);
  bool down = digitalRead(BTN_DOWN);
  bool ok = digitalRead(BTN_OK);

  // UP
  if (lastUp == HIGH && up == LOW && (now - lastDebounceMs) > DEBOUNCE_MS)
  {
    handleButton('U');
    lastDebounceMs = now;
  }
  // DOWN
  if (lastDown == HIGH && down == LOW && (now - lastDebounceMs) > DEBOUNCE_MS)
  {
    handleButton('D');
    lastDebounceMs = now;
  }
  // OK
  if (lastOk == HIGH && ok == LOW && (now - lastDebounceMs) > DEBOUNCE_MS)
  {
    handleButton('O');
    lastDebounceMs = now;
  }

  lastUp = up;
  lastDown = down;
  lastOk = ok;
}

// ---------- DESENHO DE TELAS ----------
void drawTitle()
{
  tft.fillScreen(BLACK);
  tft.setTextSize(3);
  tft.setTextColor(YELLOW);
  tft.setCursor(TITLE_X, TITLE_Y);
  tft.print("Simulador ESG");

  tft.setTextSize(2);
  tft.setTextColor(WHITE);
  tft.setCursor(TITLE_X, SUBTITLE_Y);
  tft.print("Pressione OK para iniciar");

  state = STATE_SHOW_TITLE;
}

void drawSelectPlayers()
{
  tft.fillScreen(BLACK);
  tft.setTextSize(2);
  tft.setTextColor(WHITE);
  tft.setCursor(PADDING_X, 20);
  tft.print("Selecione quantidade");
  tft.setCursor(PADDING_X, 40);
  tft.print("de jogadores (1 a 3)");

  tft.setCursor(PADDING_X, 80);
  tft.print("UP/DOWN altera, OK confirma");

  drawSelectPlayersValue();

  state = STATE_SELECT_PLAYERS;
}

void drawSelectPlayersValue()
{
  tft.fillRect(120, 120, 80, 50, BLACK);
  tft.setTextSize(5);
  tft.setTextColor(YELLOW);
  tft.setCursor(140, 125);
  tft.print(numPlayers);

  if (state == STATE_SELECT_PLAYERS)
  {
    // sendMode removido, mas mantemos a chamada para redesenho
  }
}

void drawScores()
{
  tft.setCursor(0, SCORE_TEXT_Y);
  tft.setTextSize(2);
  tft.setTextColor(WHITE, BLACK);

  for (uint8_t i = 0; i < numPlayers; i++)
  {
    tft.print("J");
    tft.print(i + 1);
    tft.print(":");
    tft.print(scores[i]);
    tft.print(" ");
  }
  tft.print("   ");
}

void drawCharacters()
{
  tft.fillScreen(BLACK);
  drawScores();

  tft.setTextSize(2);
  tft.setTextColor(YELLOW);
  tft.setCursor(PADDING_X, 30);
  tft.print("Personagens sorteados:");

  char buf[30];
  tft.setTextColor(WHITE);
  for (uint8_t p = 0; p < numPlayers; p++)
  {
    getProgmemString(personagemNomes, characters[p], buf, sizeof(buf));
    tft.setCursor(PADDING_X, 60 + p * LINE_HEIGHT);
    tft.print("J");
    tft.print(p + 1);
    tft.print(": ");
    tft.print(buf);
  }

  tft.setCursor(PADDING_X, 60 + numPlayers * LINE_HEIGHT + 10);
  tft.print("Pressione OK para comecar");

  // sendMode removido
}

void drawQuestionText()
{
  char buf[256];
  strcpy_P(buf, (PGM_P)pgm_read_word(&(perguntas[currentQuestionIndex])));

  uint16_t y = QUESTION_START_Y;
  tft.setTextSize(2);

  char *token = strtok(buf, "|");
  uint8_t line = 0;

  while (token != NULL && line < 4)
  {
    tft.setCursor(5, y);
    if (line == 0)
    {
      // Pergunta
      tft.setTextColor(WHITE, BLACK);
      tft.print(token);
    }
    else
    {
      // Alternativas
      uint8_t altIndex = line - 1;
      if (altIndex == selectedOption)
      {
        tft.setTextColor(YELLOW, BLACK);
        tft.print(">");
      }
      else
      {
        tft.setTextColor(WHITE, BLACK);
        tft.print(" ");
      }
      tft.print(token);
    }
    y += OPTION_SPACING;
    line++;
    token = strtok(NULL, "|");
  }
}

void drawQuestionScreen()
{
  tft.fillScreen(BLACK);
  drawScores();

  tft.setTextSize(2);
  tft.setTextColor(CYAN);
  tft.setCursor(5, 22);
  if (isEventoCanonico)
  {
    tft.print("Evento em conjunto");
  }
  else
  {
    tft.print("Evento individual");
  }

  tft.setCursor(5, 44);
  tft.setTextColor(WHITE);
  tft.print("Vez do Jogador ");
  tft.print(currentPlayer + 1);

  drawQuestionText();

  state = STATE_WAIT_ANSWER; // Garante o estado correto
}

void drawFeedback(bool acerto)
{
  tft.fillScreen(BLACK);
  drawScores();

  tft.setTextSize(2);
  tft.setCursor(PADDING_X, 40);
  if (acerto)
  {
    tft.setTextColor(GREEN);
    tft.print("Jogador ");
    tft.print(currentPlayer + 1);
    tft.print(" ACERTOU! +5");
  }
  else
  {
    tft.setTextColor(RED);
    tft.print("Jogador ");
    tft.print(currentPlayer + 1);
    tft.print(" ERROU.");
  }

  tft.setTextColor(WHITE);
  tft.setCursor(PADDING_X, 80);
  tft.print("Pressione OK para continuar");

  state = STATE_SHOW_FEEDBACK; // Garante o estado correto
}

void drawRankPromotion()
{
  tft.fillScreen(BLACK);

  tft.setTextSize(3);
  tft.setTextColor(YELLOW);
  tft.setCursor(TITLE_X, 30);
  tft.print("PARABENS!");

  tft.setTextSize(2);
  tft.setTextColor(WHITE);
  tft.setCursor(PADDING_X, 80);
  tft.print("Jogador ");
  tft.print(rankPlayer + 1);
  tft.print(" se tornou:");

  tft.setTextSize(3);
  tft.setTextColor(GREEN);
  tft.setCursor(TITLE_X, 120);
  tft.print(getCargoName(newRank));

  tft.setTextSize(2);
  tft.setTextColor(WHITE);
  tft.setCursor(PADDING_X, 170);
  tft.print("Pontuacao: ");
  tft.print(scores[rankPlayer]);

  tft.setCursor(PADDING_X, 200);
  tft.print("Pressione OK");

  state = STATE_SHOW_RANK;
}

void drawWinner(uint8_t winner)
{
  tft.fillScreen(BLACK);
  drawScores();

  tft.setTextSize(3);
  tft.setTextColor(YELLOW);
  tft.setCursor(PADDING_X, 50);
  tft.print("Jogador ");
  tft.print(winner + 1);
  tft.print(" venceu!");

  // Exibir conquista final
  tft.setTextSize(2);
  tft.setTextColor(GREEN, BLACK);
  tft.setCursor(PADDING_X, 90);
  tft.print("Novo Cargo: ");
  tft.print(getCargoName(calculateRank(scores[winner])));

  tft.setTextColor(WHITE, BLACK);
  tft.setCursor(PADDING_X, 130);
  tft.print("Pressione OK para reiniciar");

  state = STATE_SHOW_WINNER;
}

// ---------- LOGICA DO JOGO (STARTNEXTQUESTION com ordem cíclica) ----------
void shuffleQuestions()
{
  for (uint8_t i = 0; i < NUM_QUESTOES; i++)
  {
    questionOrder[i] = i;
  }
  for (int i = NUM_QUESTOES - 1; i > 0; i--)
  {
    int j = random(i + 1);
    uint8_t tmp = questionOrder[i];
    questionOrder[i] = questionOrder[j];
    questionOrder[j] = tmp;
  }
  questionPos = 0;
}

void sortCharacters()
{
  uint8_t pool[8];
  for (uint8_t i = 0; i < 8; i++)
    pool[i] = i;
  for (int i = 7; i > 0; i--)
  {
    int j = random(i + 1);
    uint8_t tmp = pool[i];
    pool[i] = pool[j];
    pool[j] = tmp;
  }

  for (uint8_t p = 0; p < numPlayers; p++)
  {
    characters[p] = pool[p];
  }
}

uint8_t findHighestScorePlayer()
{
  uint8_t best = 0;
  for (uint8_t i = 1; i < numPlayers; i++)
  {
    if (scores[i] > scores[best])
      best = i;
  }
  return best;
}

void startNextQuestion()
{
  if (questionPos >= NUM_QUESTOES)
  {
    uint8_t winner = findHighestScorePlayer();
    drawWinner(winner);
    return;
  }

  // Define se este sera um evento em conjunto (a cada ~5 minutos)
  unsigned long now = millis();
  if (now - lastEventoMs >= INTERVALO_EVENTO_MS)
  {
    isEventoCanonico = true;
    lastEventoMs = now;
  }
  else
  {
    isEventoCanonico = false;
  }

  currentQuestionIndex = questionOrder[questionPos];

  // Ordem Cíclica: Garante que J1 (índice 0) comece e todos joguem igualmente
  currentPlayer = questionPos % numPlayers;

  questionPos++; // Incrementa a rodada
  selectedOption = 0;

  drawQuestionScreen();
}

void handleButton(char c)
{
  switch (state)
  {
  case STATE_SHOW_TITLE:
    if (c == 'O')
    {
      memset(scores, 0, sizeof(scores));
      memset(lastRanks, 0, sizeof(lastRanks));
      numPlayers = 1;
      shuffleQuestions();
      drawSelectPlayers();
    }
    break;

  case STATE_SELECT_PLAYERS:
    // UP DIMINUI (sem wrap-around)
    if (c == 'U')
    {
      if (numPlayers > 1)
        numPlayers--;
      drawSelectPlayersValue();
      // DOWN AUMENTA (sem wrap-around)
    }
    else if (c == 'D')
    {
      if (numPlayers < MAX_PLAYERS)
        numPlayers++;
      drawSelectPlayersValue();
    }
    else if (c == 'O')
    {
      sortCharacters();
      drawCharacters();
      state = STATE_SORT_CHARACTERS;
    }
    break;

  case STATE_SORT_CHARACTERS:
    if (c == 'O')
    {
      lastEventoMs = millis();
      startNextQuestion();
    }
    break;

  case STATE_WAIT_ANSWER:
    if (c == 'U')
    {
      if (selectedOption == 0)
        selectedOption = 2;
      else
        selectedOption--;
      drawQuestionScreen(); // Redesenha para mostrar a seleção
    }
    else if (c == 'D')
    {
      selectedOption = (selectedOption + 1) % 3;
      drawQuestionScreen(); // Redesenha para mostrar a seleção
    }
    else if (c == 'O')
    {
      {
        uint8_t correta = pgm_read_byte(&(respostasCorretas[currentQuestionIndex]));
        bool acerto = (selectedOption == correta);
        if (acerto)
        {
          scores[currentPlayer] += PONTOS_ACERTO;
          if (scores[currentPlayer] >= PONTOS_VITORIA)
          {
            scores[currentPlayer] = PONTOS_VITORIA;
            drawWinner(currentPlayer);
            return;
          }

          if (checkRankPromotion(currentPlayer))
          {
            drawRankPromotion();
            return;
          }
        }
        drawFeedback(acerto);
        state = STATE_SHOW_FEEDBACK;
      }
    }
    break;

  case STATE_SHOW_FEEDBACK:
  case STATE_SHOW_RANK:
    if (c == 'O')
    {
      startNextQuestion();
    }
    break;

  case STATE_SHOW_WINNER:
    if (c == 'O')
    {
      memset(scores, 0, sizeof(scores));
      memset(lastRanks, 0, sizeof(lastRanks));
      shuffleQuestions();
      drawTitle();
    }
    break;
  }
}