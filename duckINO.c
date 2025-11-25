// ARDUINO CENTRAL (EMPRESA) - Gerencia Jogo, Display LCD, Pontuação e Eventos.

#include <MCUFRIEND_kbv.h>
#include <SoftwareSerial.h>

// --- Configuração de Hardware ---
#define RX_PIN 10
#define TX_PIN 11
SoftwareSerial ComuColetivo(RX_PIN, TX_PIN);

#define PINO_BOTAO_CIMA 5       // Botão CIMA (Incrementa, Seleciona Opção A/B/C)
#define PINO_BOTAO_CONFIRMA 6   // Botão CONFIRMA (OK/Avança Fase)
#define PINO_BOTAO_BAIXO 7      // Botão BAIXO (Decrementa, Seleciona Opção A/B/C)

MCUFRIEND_kbv tft;

// Cores RGB565 (Padrão LCD)
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define GRAY    0xAD55  // Novo cinza para o fundo da pergunta
#define PURPLE  0x801F  // Novo roxo para as opções

#define MAX_JOGADORES 8
#define TOTAL_EVENTOS 60 // Número total de eventos no jogo

// --- Definições de Jogo ---
enum HabilidadeDestaque { COMUNICACAO_DESTAQUE, CRIATIVIDADE_DESTAQUE, PROATIVIDADE_DESTAQUE, TECNICA_DESTAQUE, NENHUM_DESTAQUE };
enum HabilidadePadrao { COMUNICATIVO, CRIATIVO, RESILIENTE, DOMINIO_SOFTWARE, ANALITICO, TOTAL_HABILIDADES_PADRAO };

const String NOMES_PERSONAGENS[] = {
    "Mr. Ducks", "Patycia", "Josepatu", "Ana Patarina",
    "Patrucio", "Patilde", "Patucho", "Patina"
};
const int TOTAL_PERSONAGENS = sizeof(NOMES_PERSONAGENS) / sizeof(NOMES_PERSONAGENS[0]);

const HabilidadeDestaque MAPA_DESTAQUES[] = {
    COMUNICACAO_DESTAQUE, COMUNICACAO_DESTAQUE, CRIATIVIDADE_DESTAQUE, CRIATIVIDADE_DESTAQUE, 
    PROATIVIDADE_DESTAQUE, PROATIVIDADE_DESTAQUE, TECNICA_DESTAQUE, TECNICA_DESTAQUE
};

struct Jogador {
    String nomePersonagem;
    HabilidadeDestaque habilidadeDestaque;
    int pontuacao[TOTAL_HABILIDADES_PADRAO];
    int cargo; // 0=Estagiário
};

// Variáveis Globais de Jogo e Estado
Jogador jogadoresAtivos[MAX_JOGADORES];
int numJogadores = 2;
int jogadorAtual = 0; // Índice do jogador da vez (0 a numJogadores-1)
bool sorteioConcluido = false;

// Variáveis para debounce
long lastDebounceTime = 0;
long debounceDelay = 200;

// Variáveis de Controle de Fluxo
int estadoTela = -1; // -1: Setup, 0: Inicializacao, 1: Coleta, 2: Sorteio, 3: Jogo Pronto, 4: Turno Informativo, 5: Pergunta

// Variáveis de Rodada
int eventoAtualIndex = -1;       // Índice do evento sorteado no array global de eventos
int eventosDaRodada[MAX_JOGADORES]; // Armazena os índices dos eventos para cada jogador
int opcaoSelecionada = 0;        // 0=A, 1=B, 2=C
long inicioTurnoMillis = 0;
const long DURACAO_TURNO_MS = 4 * 60 * 1000; // 4 minutos

// -------------------------------------------------------------------------
// --- ESTRUTURA DE EVENTOS (60 Eventos) ---

struct Evento {
    String titulo;
    String opcaoA;
    String opcaoB;
    String opcaoC;
    HabilidadeDestaque categoria; // Para bônus
    int idGlobal; // Índice real do evento (0 a 59)
};

// Declaração do array de eventos (60 eventos)
Evento Eventos[TOTAL_EVENTOS];

// Função para popular os 60 eventos
void inicializarEventos() {
    
    // --- CARTA DE COMUNICAÇÃO (Índices 0 a 14) ---
    int i = 0;
    Eventos[i++] = {"E-mail do cliente confuso", "A: Responde educadamente e clara", "B: Liga p/ explicar detalhes", "C: Prop. reuniao conjunta c/ equipe", COMUNICACAO_DESTAQUE, i-1};
    Eventos[i++] = {"Conflito sobre prioridades", "A: Media discussao c/ tato", "B: Sugere votacao do grupo", "C: Consolida decisoes em doc", COMUNICACAO_DESTAQUE, i-1};
    Eventos[i++] = {"Feedback negativo sobre apres.", "A: Reapresenta focando pontos", "B: Solicita feedback detalhado", "C: Envolve equipe p/ melhoria", COMUNICACAO_DESTAQUE, i-1};
    Eventos[i++] = {"Mal-entendido com colega", "A: Explica imediatamente", "B: Propoe reuniao p/ alinhamento", "C: Cria resumo compartilhado", COMUNICACAO_DESTAQUE, i-1};
    Eventos[i++] = {"Reuniao surpresa do cliente", "A: Responde rapido por chat", "B: Liga p/ explicar detalhadamente", "C: Envolve colega a esquerda", COMUNICACAO_DESTAQUE, i-1};
    Eventos[i++] = {"Feedback ambiguo do gerente", "A: Solicita esclarecimento", "B: Reune equipe p/ interpretar", "C: Resume pontos-chave", COMUNICACAO_DESTAQUE, i-1};
    Eventos[i++] = {"Apresentacao falha", "A: Reapresenta focando resultados", "B: Propoe reuniao interna", "C: Convida colega p/ co-apresentar", COMUNICACAO_DESTAQUE, i-1};
    Eventos[i++] = {"Desalinhamento entre times", "A: Coordena call rapida", "B: Cria documento de alinhamento", "C: Facilita grupo de discussao", COMUNICACAO_DESTAQUE, i-1};
    Eventos[i++] = {"Cliente solicita detalhamento urge.", "A: Redige resumo claro", "B: Organiza reuniao rapida", "C: Propoe apresentacao colaborativa", COMUNICACAO_DESTAQUE, i-1};
    Eventos[i++] = {"Debate interno sobre prioridades", "A: Media buscando consenso", "B: Sugere votacao", "C: Documenta decisoes", COMUNICACAO_DESTAQUE, i-1};
    Eventos[i++] = {"Problema de comunicacao de colega", "A: Corrige de forma clara", "B: Sugere feedback estruturado", "C: Cria guia compartilhado", COMUNICACAO_DESTAQUE, i-1};
    // Antiéticos
    Eventos[i++] = {"AntiEtico: Manipulacao de metricas", "A: Ajusta relatorio p/ valorizar", "B: Assume merito sozinho", "C: Reorganiza dados p/ analise", COMUNICACAO_DESTAQUE, i-1};
    Eventos[i++] = {"AntiEtico: Exagero de resultados", "A: Destaca progresso proprio", "B: Ajusta graficos p/ melhoria", "C: Apres. relatorio detalhado", COMUNICACAO_DESTAQUE, i-1};
    Eventos[i++] = {"AntiEtico: Credito do time", "A: Assume meritos do time", "B: Realca apenas seu trabalho", "C: Organiza destaque equilibrado", COMUNICACAO_DESTAQUE, i-1};

    // --- CARTA DE CRIATIVIDADE (Índices 15 a 29) ---
    Eventos[i++] = {"Brainstorm inesperado", "A: Propoe ideias originais", "B: Sugere adaptacao de solucao", "C: Coord. colega a direita p/ compl.", CRIATIVIDADE_DESTAQUE, i-1};
    Eventos[i++] = {"Desafio de inovacao rapida", "A: Cria prototipo funcional", "B: Reune equipe p/ melhorar prototipo", "C: Documenta processo criativo", CRIATIVIDADE_DESTAQUE, i-1};
    Eventos[i++] = {"Problema inesperado de design", "A: Propoe solucao alternativa", "B: Adapta ideia existente c/ melhorias", "C: Coordena sugestao coletiva", CRIATIVIDADE_DESTAQUE, i-1};
    Eventos[i++] = {"Ideia rejeitada pelo gerente", "A: Reformula ideia", "B: Propoe outra abordagem", "C: Organiza sessao de feedback", CRIATIVIDADE_DESTAQUE, i-1};
    Eventos[i++] = {"Sugestao para otimizar processo", "A: Propoe melhoria inovadora", "B: Cria prototipo de teste", "C: Coordena colegas p/ implementar", CRIATIVIDADE_DESTAQUE, i-1};
    // Antiéticos
    Eventos[i++] = {"AntiEtico: Apropriacao de ideia alheia", "A: Apresenta ideia do colega como sua", "B: Adapta parcialmente p/ parecer proprio", "C: Reorganiza e credita equipe", CRIATIVIDADE_DESTAQUE, i-1};
    Eventos[i++] = {"AntiEtico: Ignorar contribuicoes", "A: Destaca apenas seu conceito", "B: Reescreve proposta p/ parecer sozinho", "C: Ajusta plano destacando colegas", CRIATIVIDADE_DESTAQUE, i-1};
    Eventos[i++] = {"AntiEtico: Exagerar inovacao", "A: Propoe recurso arriscado s/ validar", "B: Apresenta solucao sem revisao", "C: Documenta ideia p/ analise futura", CRIATIVIDADE_DESTAQUE, i-1};
    Eventos[i++] = {"Workshop criativo", "A: Participa ativamente", "B: Sugere melhoria em ideia do colega", "C: Coordena proposta coletiva", CRIATIVIDADE_DESTAQUE, i-1};
    Eventos[i++] = {"Revisao de prototipo", "A: Propoe ajustes inovadores", "B: Sugere melhorias incrementais", "C: Coordena teste coletivo", CRIATIVIDADE_DESTAQUE, i-1};
    Eventos[i++] = {"Ideia de marketing digital", "A: Cria campanha original", "B: Sugere adaptacao de campanhas ant.", "C: Coordena colega a esquerda p/ compl.", CRIATIVIDADE_DESTAQUE, i-1};
    Eventos[i++] = {"Problema de branding", "A: Propoe conceito alternativo", "B: Adapta design existente", "C: Coordena apresentacao do grupo", CRIATIVIDADE_DESTAQUE, i-1};
    Eventos[i++] = {"Sugestao de melhoria de produto", "A: Propoe funcionalidade inovadora", "B: Cria prototipo de teste", "C: Coordena time p/ aplicar", CRIATIVIDADE_DESTAQUE, i-1};
    Eventos[i++] = {"Brainstorm de urgencia", "A: Cria varias ideias", "B: Prioriza melhores ideias", "C: Coordena resumo de ideias", CRIATIVIDADE_DESTAQUE, i-1};
    Eventos[i++] = {"Apresentacao final de conceito", "A: Destaca proposta mais original", "B: Coordena colegas p/ apresentar", "C: Sugere documentacao detalhada", CRIATIVIDADE_DESTAQUE, i-1};
    
    // --- CARTA DE PROATIVIDADE (Índices 30 a 44) ---
    Eventos[i++] = {"Novo projeto urgente", "A: Assume a lideranca imediata", "B: Organiza colegas p/ dividir tarefas", "C: Planeja cronograma detalhado", PROATIVIDADE_DESTAQUE, i-1};
    Eventos[i++] = {"Solicitacao de cliente inesperada", "A: Resolve rapidamente sozinho", "B: Coordena equipe p/ acao rapida", "C: Propoe plano alternativo", PROATIVIDADE_DESTAQUE, i-1};
    Eventos[i++] = {"Deadline apertado", "A: Trabalha horas extras", "B: Reorganiza tarefas da equipe", "C: Prioriza tarefas mais importantes", PROATIVIDADE_DESTAQUE, i-1};
    Eventos[i++] = {"Problema tecnico de ultima hora", "A: Resolve rapidamente", "B: Organiza equipe p/ correcao", "C: Sugere solucao alternativa", PROATIVIDADE_DESTAQUE, i-1};
    Eventos[i++] = {"Evento de melhoria continua", "A: Propoe novos processos", "B: Sugere pequenas mudancas incr.", "C: Coordena implementacao do grupo", PROATIVIDADE_DESTAQUE, i-1};
    // Antiéticos
    Eventos[i++] = {"AntiEtico: Pular etapas de revisao", "A: Aplica mudanca sem validar", "B: Assume que resultados serao aceitos", "C: Reorganiza implement. respeitando", PROATIVIDADE_DESTAQUE, i-1};
    Eventos[i++] = {"AntiEtico: Tomar credito indevido", "A: Assume resultado de colegas", "B: Destaca apenas seu trabalho", "C: Divide merito de forma equilibrada", PROATIVIDADE_DESTAQUE, i-1};
    Eventos[i++] = {"AntiEtico: Pressionar equipe injusta", "A: Exige entrega sem consenso", "B: Aplica pressao p/ parecer eficiente", "C: Motiva equipe mantendo equilibrio", PROATIVIDADE_DESTAQUE, i-1};
    Eventos[i++] = {"Treinamento interno voluntario", "A: Participa ativamente", "B: Propoe pratica em grupo", "C: Coordena colegas p/ executar", PROATIVIDADE_DESTAQUE, i-1};
    Eventos[i++] = {"Reuniao de planejamento rapido", "A: Assume coordenacao", "B: Ajuda a organizar agenda", "C: Sugere tarefas prioritarias", PROATIVIDADE_DESTAQUE, i-1};
    Eventos[i++] = {"Revisao de workflow", "A: Propoe melhorias imediatas", "B: Cria plano incremental", "C: Coordena testes do grupo", PROATIVIDADE_DESTAQUE, i-1};
    Eventos[i++] = {"Cliente muda prioridade de proj.", "A: Ajusta cronograma rapidamente", "B: Reorganiza equipe p/ atender", "C: Sugere estrategia alternativa", PROATIVIDADE_DESTAQUE, i-1};
    Eventos[i++] = {"Evento de feedback do time", "A: Implementa sugestao rapidamente", "B: Coordena equipe p/ melhorias", "C: Propoe plano de acao coletivo", PROATIVIDADE_DESTAQUE, i-1};
    Eventos[i++] = {"Desafio de urgencia inesperado", "A: Assume acao decisiva", "B: Reune equipe p/ responder", "C: Planeja solucao alternativa", PROATIVIDADE_DESTAQUE, i-1};
    Eventos[i++] = {"Implementacao de inovacao", "A: Executa rapidamente", "B: Coordena equipe p/ aplicar", "C: Sugere melhorias durante exec.", PROATIVIDADE_DESTAQUE, i-1};

    // --- CARTA DE TÉCNICA (Índices 45 a 59) ---
    Eventos[i++] = {"Problema critico no codigo", "A: Corrige bug sozinho", "B: Reune equipe p/ depuracao", "C: Docum. solucao e ensina colegas", TECNICA_DESTAQUE, i-1};
    Eventos[i++] = {"Atualizacao urgente de sistema", "A: Aplica patch rapidamente", "B: Testa com equipe antes", "C: Cria checklist de seguranca", TECNICA_DESTAQUE, i-1};
    Eventos[i++] = {"Falha inesperada de servidor", "A: Resolve imediatamente", "B: Organiza grupo p/ diagnosticar", "C: Sugere monitoramento preventivo", TECNICA_DESTAQUE, i-1};
    Eventos[i++] = {"Revisao de codigo em equipe", "A: Corrige problemas criticos", "B: Propoe melhorias de performance", "C: Coordena grupo para testes", TECNICA_DESTAQUE, i-1};
    Eventos[i++] = {"Evento de documentacao tecnica", "A: Cria manual claro", "B: Organiza exemplos praticos", "C: Coordena revisao do grupo", TECNICA_DESTAQUE, i-1};
    // Antiéticos
    Eventos[i++] = {"AntiEtico: Manipulacao de logs", "A: Altera registros p/ parecer melhor", "B: Oculta erro intencionalmente", "C: Documenta ajuste de forma transpare.", TECNICA_DESTAQUE, i-1};
    Eventos[i++] = {"AntiEtico: Codigo de colega s/ credito", "A: Assina codigo de outro como proprio", "B: Modifica parcialmente p/ parecer orig.", "C: Credita corretamente equipe e colegas", TECNICA_DESTAQUE, i-1};
    Eventos[i++] = {"AntiEtico: Contornar processo de revisao", "A: Aplica solucao sem validacao", "B: Ignora etapas de teste", "C: Executa testes minimos mantendo reg.", TECNICA_DESTAQUE, i-1};
    Eventos[i++] = {"Integracao de sistema", "A: Implementa sozinho", "B: Coordena equipe para integrar", "C: Documenta e revisa integracao", TECNICA_DESTAQUE, i-1};
    Eventos[i++] = {"Teste de performance", "A: Otimiza codigo", "B: Analisa resultados com equipe", "C: Sugere melhorias coletivas", TECNICA_DESTAQUE, i-1};
    Eventos[i++] = {"Atualizacao de framework", "A: Aplica mudancas rapidamente", "B: Testa impacto em modulo especifico", "C: Coordena equipe para validar", TECNICA_DESTAQUE, i-1};
    Eventos[i++] = {"Revisao de seguranca", "A: Corrige vulnerabilidades", "B: Sugere praticas preventivas", "C: Organiza teste coletivo", TECNICA_DESTAQUE, i-1};
    Eventos[i++] = {"Debug critico de producao", "A: Resolve problema sozinho", "B: Coordena equipe para solucao", "C: Documenta solucao e aprendizado", TECNICA_DESTAQUE, i-1};
    Eventos[i++] = {"Otimizacao de codigo legado", "A: Refatora modulos criticos", "B: Propoe melhorias incrementais", "C: Coordena testes e documentacao", TECNICA_DESTAQUE, i-1};
    Eventos[i++] = {"Deploy de release importante", "A: Executa rollout sozinho", "B: Coordena equipe p/ implantacao segura", "C: Documenta e monitora resultados", TECNICA_DESTAQUE, i-1};
}

// Função para sortear eventos únicos para a rodada
void sortearEventosRodada() {
    int eventosDisponiveis[TOTAL_EVENTOS];
    for (int i = 0; i < TOTAL_EVENTOS; i++) { eventosDisponiveis[i] = i; }

    // Embaralha o array de índices de eventos
    for (int i = TOTAL_EVENTOS - 1; i > 0; i--) {
        int j = random(i + 1);
        int temp = eventosDisponiveis[i];
        eventosDisponiveis[i] = eventosDisponiveis[j];
        eventosDisponiveis[j] = temp;
    }

    // Pega os N primeiros eventos (onde N = numJogadores)
    // Estes N eventos serão usados sequencialmente nos turnos
    for (int i = 0; i < numJogadores; i++) {
        eventosDaRodada[i] = eventosDisponiveis[i];
    }
}

// -------------------------------------------------------------------------
// --- FUNÇÕES DE DISPLAY LCD (IMPLEMENTADAS) ---

void limparTela(uint16_t cor) {
    tft.fillScreen(cor);
    tft.setCursor(0, 0);
}

void centralizarTexto(String texto, int y, uint16_t cor, int tamanho = 3) {
    int16_t x1, y1;
    uint16_t w, h;
    tft.setTextSize(tamanho);
    tft.setTextColor(cor);
    tft.getTextBounds(texto, 0, 0, &x1, &y1, &w, &h);
    int x = (tft.width() - w) / 2;
    tft.setCursor(x, y);
    tft.print(texto);
}

void displayInicializacao() {
    limparTela(BLUE);
    centralizarTexto("RubberDucks Inc.", 60, WHITE, 4);
    centralizarTexto("Simulador ESG", 120, YELLOW, 4);
}

void displayPerguntarJogadores() {
    limparTela(BLACK);
    centralizarTexto("Escolha Participantes", 40, YELLOW, 3);
    centralizarTexto("Numero: " + String(numJogadores), 100, CYAN, 5);
    centralizarTexto("[CIMA/BAIXO] Ajustar", 180, WHITE, 2);
    centralizarTexto("[CONFIRMA] Iniciar", 210, WHITE, 2);
}

String getNomeDestaque(HabilidadeDestaque d) {
    switch (d) {
        case COMUNICACAO_DESTAQUE: return "Comunicacao";
        case CRIATIVIDADE_DESTAQUE: return "Criatividade";
        case PROATIVIDADE_DESTAQUE: return "Proatividade";
        case TECNICA_DESTAQUE: return "Tecnica";
        default: return "N/A";
    }
}

void displaySorteio(int idJogador) {
    limparTela(GREEN);
    String destaque = getNomeDestaque(jogadoresAtivos[idJogador].habilidadeDestaque);

    centralizarTexto("Jogador " + String(idJogador + 1), 30, BLACK, 4);

    tft.setTextSize(3);
    tft.setTextColor(RED);
    tft.setCursor(20, 90); tft.print("Personagem:");
    tft.setTextColor(WHITE);
    tft.setCursor(20, 120); tft.print(jogadoresAtivos[idJogador].nomePersonagem);

    tft.setTextColor(RED);
    tft.setCursor(20, 170); tft.print("Destaque:");
    tft.setTextColor(WHITE);
    tft.setCursor(20, 200); tft.print(destaque);

    centralizarTexto("Press CONFIRMA", 270, YELLOW, 2);
}

void displayVamosComecar() {
    limparTela(BLUE);
    centralizarTexto("VAMOS COMECAR", 100, WHITE, 4);
    centralizarTexto("O JOGO!", 150, YELLOW, 4);
}

// Corresponde ao Frame 13 (Jogador X pressione "A" para prosseguir)
void displayTurnoInformativo() {
    limparTela(GRAY);
    String nomeJogador = jogadoresAtivos[jogadorAtual].nomePersonagem;
    
    // As cores e tamanhos estão ajustados para a imagem de referência
    tft.setTextSize(3);
    centralizarTexto("Turno de", 80, WHITE, 3);
    
    tft.setTextSize(4);
    centralizarTexto(nomeJogador, 130, YELLOW, 4);
    
    tft.setTextSize(3);
    centralizarTexto("Pressione CONFIRMA", 220, WHITE, 3);
    centralizarTexto("Para Prosseguir", 260, WHITE, 3);
}

// Corresponde ao Frame da Pergunta (Frame 2)
void displayPergunta() {
    limparTela(GRAY);

    Evento e = Eventos[eventosDaRodada[jogadorAtual]]; // Evento do jogador atual
    String nomeJogador = jogadoresAtivos[jogadorAtual].nomePersonagem;
    
    // --- Header (Nome do Jogador e Pontuação Fictícia) ---
    tft.setTextSize(3);
    tft.setTextColor(WHITE);
    tft.setCursor(10, 10);
    tft.print(nomeJogador);
    tft.setCursor(tft.width() - 90, 10);
    // Pontuação Fixa de 5pts para o momento
    tft.print("5 pts"); 

    // --- Timer (4 minutos) ---
    long tempoRestante_ms = DURACAO_TURNO_MS - (millis() - inicioTurnoMillis);
    if (tempoRestante_ms < 0) tempoRestante_ms = 0;
    
    int minutos = tempoRestante_ms / (60 * 1000);
    int segundos = (tempoRestante_ms / 1000) % 60;
    
    // Verifica se o tempo está abaixo de 1 minuto para deixar vermelho
    uint16_t corTimer = (tempoRestante_ms < 60000) ? RED : WHITE;

    tft.setTextSize(2);
    tft.setTextColor(corTimer);
    tft.setCursor(tft.width() - 90, 45); // Posição abaixo da pontuação
    tft.print(minutos); tft.print(":");
    if (segundos < 10) tft.print("0");
    tft.print(segundos);
    
    // --- Pergunta Principal (Retângulo Superior) ---
    // Desenha o retângulo de fundo para a pergunta (Roxo)
    tft.fillRect(10, 70, tft.width() - 20, 40, PURPLE);
    
    tft.setTextSize(2);
    tft.setTextColor(WHITE);
    centralizarTexto(e.titulo, 80, WHITE, 2);
    
    // --- Opções (A, B, C) ---
    tft.setTextSize(2);
    int yStart = 120;
    int ySpacing = 65;
    
    // Função auxiliar para desenhar a opção
    auto desenharOpcao = [&](int indice, int y) {
        bool selecionada = (opcaoSelecionada == indice);
        uint16_t corFundo = selecionada ? GREEN : PURPLE;
        String textoOpcao;
        if (indice == 0) textoOpcao = "A - " + e.opcaoA;
        else if (indice == 1) textoOpcao = "B - " + e.opcaoB;
        else textoOpcao = "C - " + e.opcaoC;
        
        tft.fillRect(10, y, tft.width() - 20, 50, corFundo);
        tft.setTextColor(WHITE);
        tft.setCursor(20, y + 15);
        tft.print(textoOpcao);
    };

    desenharOpcao(0, yStart);
    desenharOpcao(1, yStart + ySpacing);
    desenharOpcao(2, yStart + 2 * ySpacing);
    
    if (tempoRestante_ms == 0) {
        tft.setTextSize(3);
        centralizarTexto("TEMPO ESGOTADO!", 300, RED, 3);
    }
}


// -------------------------------------------------------------------------
// --- FUNÇÕES UTILITÁRIAS ---

void enviarInicializacao(int idJogador) {
    // I,ID,NOME,DESTAQUE\n
    ComuColetivo.print("I,"); 
    ComuColetivo.print(idJogador + 1);
    ComuColetivo.print(",");
    ComuColetivo.print(jogadoresAtivos[idJogador].nomePersonagem);
    ComuColetivo.print(",");
    ComuColetivo.print(getNomeDestaque(jogadoresAtivos[idJogador].habilidadeDestaque));
    ComuColetivo.print('\n'); 
}

bool lerBotao(int pino) {
    if (digitalRead(pino) == LOW) { 
        if (millis() - lastDebounceTime > debounceDelay) {
            lastDebounceTime = millis();
            return true;
        }
    }
    return false;
}

void avancarProximoTurno() {
    // Lógica para avançar a rodada e reiniciar o ciclo
    jogadorAtual = (jogadorAtual + 1) % numJogadores;
    // TODO: AQUI É ONDE SERIA FEITO O CÁLCULO DA PONTUAÇÃO ESG NO ARDUINO CENTRAL
    estadoTela = 4; // Volta para o Turno Informativo (Frame 13)
}

// Função para processar a escolha e enviar o comando ao Coletivo
void processarEscolha(char escolha) {
    // Formato do comando: [Ação][ID do Evento]\n
    // Exemplo: A00\n (Ação A, Evento ID 0)
    
    int idEvento = Eventos[eventosDaRodada[jogadorAtual]].idGlobal;
    
    // Envia a escolha para o Controle Coletivo
    ComuColetivo.print(escolha);
    ComuColetivo.print(idEvento); 
    ComuColetivo.print('\n');
    
    Serial.print("Acao Enviada: "); Serial.print(escolha); Serial.println(idEvento);
    
    // O jogo espera o Arduino Coletivo processar e enviar o feedback de volta (opcional)
    // Por enquanto, avança o turno para simplificar o loop.
    avancarProximoTurno();
}

// -------------------------------------------------------------------------
void setup() {
    Serial.begin(9600);
    ComuColetivo.begin(9600); 

    pinMode(PINO_BOTAO_CIMA, INPUT_PULLUP);
    pinMode(PINO_BOTAO_CONFIRMA, INPUT_PULLUP);
    pinMode(PINO_BOTAO_BAIXO, INPUT_PULLUP);

    uint16_t ID = tft.readID();
    if (ID == 0xD3D3) ID = 0x9481;
    tft.begin(ID);
    tft.setRotation(1);

    randomSeed(analogRead(A0));
    inicializarEventos(); // Popula o array de 60 eventos

    displayInicializacao();
    delay(2000);
    estadoTela = 0; // Transiciona para a fase de coleta de jogadores
}

// -------------------------------------------------------------------------
void loop() {
    
    // --- ESTADO 0 & 1: COLETA DE JOGADORES (SETUP) ---
    if (estadoTela == 0) {
        displayPerguntarJogadores();
        estadoTela = 1;
    }
    
    if (estadoTela == 1) {
        if (lerBotao(PINO_BOTAO_CIMA)) { if (numJogadores < MAX_JOGADORES) numJogadores++; displayPerguntarJogadores(); }
        if (lerBotao(PINO_BOTAO_BAIXO)) { if (numJogadores > 2) numJogadores--; displayPerguntarJogadores(); }

        if (lerBotao(PINO_BOTAO_CONFIRMA) && !sorteioConcluido) {
            // Lógica de Sorteio de Personagens (Fisher-Yates)
            int cartasDisponiveis[TOTAL_PERSONAGENS];
            for (int i = 0; i < TOTAL_PERSONAGENS; i++) cartasDisponiveis[i] = i;
            for (int i = TOTAL_PERSONAGENS - 1; i > 0; i--) { int j = random(i + 1); int temp = cartasDisponiveis[i]; cartasDisponiveis[i] = cartasDisponiveis[j]; cartasDisponiveis[j] = temp; }
            
            // Atribui Personagens
            for (int i = 0; i < numJogadores; i++) {
                int indiceCarta = cartasDisponiveis[i];
                jogadoresAtivos[i].nomePersonagem = NOMES_PERSONAGENS[indiceCarta];
                jogadoresAtivos[i].habilidadeDestaque = MAPA_DESTAQUES[indiceCarta];
                for (int h = 0; h < TOTAL_HABILIDADES_PADRAO; h++) jogadoresAtivos[i].pontuacao[h] = 0;
            }
            
            sorteioConcluido = true;
            jogadorAtual = 0; 
            displaySorteio(jogadorAtual);
            estadoTela = 2; // Transiciona para exibição sequencial
        }
    }
    
    // --- ESTADO 2: EXIBIÇÃO SEQUENCIAL DO SORTEIO ---
    else if (estadoTela == 2 && sorteioConcluido) {
        
        // CORREÇÃO CRÍTICA: Garantir o envio da informação ao Controle Coletivo
        enviarInicializacao(jogadorAtual); 
        displaySorteio(jogadorAtual); // Mantém a tela de sorteio atualizada
        
        if (lerBotao(PINO_BOTAO_CONFIRMA)) {
            jogadorAtual++;
            if (jogadorAtual < numJogadores) {
                // Continua exibindo o próximo personagem
                // displaySorteio será chamado no próximo loop() após o incremento
            } else {
                // Todos os jogadores foram sorteados
                sortearEventosRodada(); // Sorteia os N eventos para a 1ª rodada
                displayVamosComecar();
                delay(2500);
                jogadorAtual = 0; // Volta para o Jogador 1 para iniciar o turno
                estadoTela = 4; // Transiciona para o Frame Informativo do 1o Turno
            }
        }
    }
    
    // --- ESTADO 4: TURNO INFORMATIVO (FRAME 13) ---
    else if (estadoTela == 4) {
        displayTurnoInformativo();
        
        if (lerBotao(PINO_BOTAO_CONFIRMA)) {
            inicioTurnoMillis = millis(); // Inicia a contagem do tempo
            opcaoSelecionada = 0; // Reseta a seleção para 'A'
            estadoTela = 5; // Transiciona para a Pergunta
        }
    }
    
    // --- ESTADO 5: PERGUNTA ALEATÓRIA (FRAME MR. DUCKS) ---
    else if (estadoTela == 5) {
        
        // 1. Lógica de Seleção de Opção (CIMA/BAIXO)
        if (lerBotao(PINO_BOTAO_CIMA)) {
            opcaoSelecionada = (opcaoSelecionada - 1 + 3) % 3; // +3 para garantir positivo
        }
        if (lerBotao(PINO_BOTAO_BAIXO)) {
            opcaoSelecionada = (opcaoSelecionada + 1) % 3; 
        }

        // 2. Exibe a Pergunta (com timer e destaque de seleção)
        displayPergunta();
        
        // 3. Lógica de Submissão (CONFIRMA ou Fim do Tempo)
        long tempoRestante_ms = DURACAO_TURNO_MS - (millis() - inicioTurnoMillis);
        
        if (lerBotao(PINO_BOTAO_CONFIRMA) || tempoRestante_ms <= 0) {
            
            char escolha;
            if (opcaoSelecionada == 0) escolha = 'A';
            else if (opcaoSelecionada == 1) escolha = 'B';
            else escolha = 'C';
            
            // Submissão
            processarEscolha(escolha); // Envia A, B ou C ao Coletivo e avança
        }
    }
    
    // O ESTADO 3 está reservado para o futuro loop de jogo
}