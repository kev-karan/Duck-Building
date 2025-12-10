#include <MCUFRIEND_kbv.h>
#include <string.h>
#include <stdlib.h>

MCUFRIEND_kbv tft;

// Cores basicas (565)
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

#define MAX_PLAYERS 3

// Botoes
const int BTN_UP   = 31;
const int BTN_DOWN = 33;
const int BTN_OK   = 35;

bool lastUp   = HIGH;
bool lastDown = HIGH;
bool lastOk   = HIGH;

unsigned long lastDebounceMs = 0;
const unsigned long DEBOUNCE_MS = 150;

// Estado recebido do Arduino central
char currentState = 'T';   // 'T','P','C','Q','F','W','R' (R = Rank/Cargo)
uint8_t lastResult = 0;    // 0 = nada ou erro, 1 = acerto
uint8_t numPlayers = 1;
uint8_t currentPlayer = 0;
uint8_t scores[MAX_PLAYERS] = {0, 0, 0};
int8_t winnerIdx = -1;

// Controle de cargos
uint8_t lastRanks[MAX_PLAYERS] = {0, 0, 0};  // Ultimo cargo de cada jogador
uint8_t newRank = 0;  // Novo cargo atingido
uint8_t rankPlayer = 0;  // Jogador que atingiu novo cargo

// Buffer de recepcao
char lineBuf[64];
uint8_t linePos = 0;

// Prototipos
void readButtons();
void readCentralMessages();
void parseLine(char *s);
void drawScreen();
void drawScores();
const char* getCargoName(uint8_t rank);
uint8_t calculateRank(uint8_t score);

void setup() {
  pinMode(BTN_UP,   INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_OK,   INPUT_PULLUP);

  Serial1.begin(9600);   // ligado ao Arduino do predio

  uint16_t ID = tft.readID();
  if (ID == 0xD3D3) ID = 0x9481;  // fallback comum
  tft.begin(ID);
  tft.setRotation(1);
  tft.fillScreen(BLACK);

  drawScreen();
}

void loop() {
  readButtons();
  readCentralMessages();
}

void readButtons() {
  unsigned long now = millis();

  bool up   = digitalRead(BTN_UP);
  bool down = digitalRead(BTN_DOWN);
  bool ok   = digitalRead(BTN_OK);

  if (lastUp == HIGH && up == LOW && (now - lastDebounceMs) > DEBOUNCE_MS) {
    Serial1.write('U');
    lastDebounceMs = now;
  }
  if (lastDown == HIGH && down == LOW && (now - lastDebounceMs) > DEBOUNCE_MS) {
    Serial1.write('D');
    lastDebounceMs = now;
  }
  if (lastOk == HIGH && ok == LOW && (now - lastDebounceMs) > DEBOUNCE_MS) {
    Serial1.write('O');
    lastDebounceMs = now;
  }

  lastUp   = up;
  lastDown = down;
  lastOk   = ok;
}

void readCentralMessages() {
  while (Serial1.available()) {
    char c = (char)Serial1.read();
    if (c == '\n' || c == '\r') {
      if (linePos > 0) {
        lineBuf[linePos] = '\0';
        parseLine(lineBuf);
        linePos = 0;
      }
    } else {
      if (linePos < sizeof(lineBuf) - 1) {
        lineBuf[linePos++] = c;
      }
    }
  }
}

uint8_t calculateRank(uint8_t score) {
  if (score >= 50) return 5;  // CEO
  if (score >= 40) return 4;  // Gerente
  if (score >= 30) return 3;  // Coordenador
  if (score >= 20) return 2;  // Especialista
  if (score >= 10) return 1;  // Analista
  return 0;  // Estagiario
}

const char* getCargoName(uint8_t rank) {
  switch (rank) {
    case 0: return "Estagiario";
    case 1: return "Analista";
    case 2: return "Especialista";
    case 3: return "Coordenador";
    case 4: return "Gerente";
    case 5: return "CEO";
    default: return "Estagiario";
  }
}

void parseLine(char *s) {
  // Espera formato: M,estado,lastResult,numPlayers,currentPlayer,s1,s2,s3,winner
  // ou R,player,rank para notificacao de cargo
  
  if (s[0] == 'R') {
    // Notificacao de novo cargo
    char *token = strtok(s, ",");
    token = strtok(NULL, ",");
    if (!token) return;
    rankPlayer = (uint8_t)atoi(token);
    
    token = strtok(NULL, ",");
    if (!token) return;
    newRank = (uint8_t)atoi(token);
    
    currentState = 'R';
    drawScreen();
    return;
  }
  
  if (s[0] != 'M') return;

  char *token = strtok(s, ",");
  // token == "M"
  token = strtok(NULL, ",");
  if (!token) return;
  currentState = token[0];

  token = strtok(NULL, ",");
  if (!token) return;
  lastResult = (uint8_t)atoi(token);

  token = strtok(NULL, ",");
  if (!token) return;
  numPlayers = (uint8_t)atoi(token);
  if (numPlayers == 0 || numPlayers > MAX_PLAYERS) numPlayers = 1;

  token = strtok(NULL, ",");
  if (!token) return;
  currentPlayer = (uint8_t)atoi(token);
  if (currentPlayer >= numPlayers) currentPlayer = 0;

  for (uint8_t i = 0; i < MAX_PLAYERS; i++) {
    token = strtok(NULL, ",");
    if (!token) {
      scores[i] = 0;
    } else {
      scores[i] = (uint8_t)atoi(token);
    }
  }

  token = strtok(NULL, ",");
  if (token) {
    int w = atoi(token);
    if (w == 255) winnerIdx = -1;
    else winnerIdx = (int8_t)w;
  } else {
    winnerIdx = -1;
  }

  drawScreen();
}

void drawScores() {
  tft.fillRect(0, 0, 320, 20, BLACK);
  tft.setTextSize(2);
  tft.setTextColor(WHITE);
  tft.setCursor(0, 0);
  for (uint8_t i = 0; i < numPlayers; i++) {
    tft.print("J");
    tft.print(i + 1);
    tft.print(":");
    tft.print(scores[i]);
    tft.print(" ");
  }
}

void drawScreen() {
  tft.fillScreen(BLACK);
  drawScores();

  tft.setTextSize(2);
  tft.setTextColor(WHITE);
  tft.setCursor(10, 30);

  switch (currentState) {
    case 'T':
      tft.setTextSize(3);
      tft.setTextColor(YELLOW);
      tft.setCursor(20, 60);
      tft.print("Simulador ESG");
      tft.setTextSize(2);
      tft.setTextColor(WHITE);
      tft.setCursor(20, 110);
      tft.print("Pressione OK para iniciar");
      break;

    case 'P':
      tft.print("Config. de jogadores");
      tft.setCursor(10, 60);
      tft.print("Total: ");
      tft.print(numPlayers);
      tft.setCursor(10, 90);
      tft.print("Use UP/DOWN e OK");
      break;

    case 'C':
      tft.print("Personagens sorteados");
      tft.setCursor(10, 60);
      tft.print("Acompanhe tela do predio");
      break;

    case 'Q':
      tft.print("Vez do Jogador ");
      tft.print(currentPlayer + 1);
      tft.setCursor(10, 60);
      tft.print("Use UP/DOWN/OK");
      tft.setCursor(10, 90);
      tft.print("Pontuacao: ");
      tft.print(scores[currentPlayer]);
      tft.setCursor(10, 120);
      tft.setTextColor(CYAN);
      tft.print("Cargo: ");
      tft.print(getCargoName(calculateRank(scores[currentPlayer])));
      break;

    case 'F':
      tft.setCursor(10, 30);
      if (lastResult == 1) {
        tft.setTextColor(GREEN);
        tft.print("Jogador ");
        tft.print(currentPlayer + 1);
        tft.print(" ACERTOU!");
      } else {
        tft.setTextColor(RED);
        tft.print("Jogador ");
        tft.print(currentPlayer + 1);
        tft.print(" ERROU.");
      }
      tft.setTextColor(WHITE);
      tft.setCursor(10, 70);
      tft.print("Pontuacao: ");
      tft.print(scores[currentPlayer]);
      tft.setCursor(10, 100);
      tft.print("Pressione OK para continuar");
      break;

    case 'R':
      // Tela de promocao de cargo
      tft.fillScreen(BLACK);
      tft.setTextSize(3);
      tft.setTextColor(YELLOW);
      tft.setCursor(20, 40);
      tft.print("PARABENS!");
      
      tft.setTextSize(2);
      tft.setTextColor(WHITE);
      tft.setCursor(10, 90);
      tft.print("Jogador ");
      tft.print(rankPlayer + 1);
      tft.print(" virou:");
      
      tft.setTextSize(3);
      tft.setTextColor(GREEN);
      tft.setCursor(10, 130);
      tft.print(getCargoName(newRank));
      
      tft.setTextSize(2);
      tft.setTextColor(WHITE);
      tft.setCursor(10, 180);
      tft.print("Pressione OK");
      break;

    case 'W':
      tft.setTextColor(YELLOW);
      tft.setTextSize(3);
      tft.setCursor(10, 60);
      if (winnerIdx >= 0 && winnerIdx < numPlayers) {
        tft.print("Jogador ");
        tft.print(winnerIdx + 1);
        tft.print(" venceu!");
      } else {
        tft.print("Fim de jogo");
      }
      tft.setTextSize(2);
      tft.setTextColor(WHITE);
      tft.setCursor(10, 110);
      tft.print("Pressione OK para reiniciar");
      break;

    default:
      tft.print("Aguardando dados...");
      break;
  }
}