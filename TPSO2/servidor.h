#define N 6
#define MAXSIZE 20
#define MAX 256
#define TAM 20
#define BUFFERSIZE 60
#define TAMBUFFERCIRC 10

typedef struct {
	int larg; //x
	int comp; //y
	int pontoinicialX[2]; //0
	int pontoinicialY[2];
	int destinoaguaX[2]; // comp-1
	int destinoaguaY[2];
	int t_agua; //tempo em segundos da agua fluir
	int seg_stopagua; //Numero de segundos em que a agua esta parada
	int p_atual_agX[2];
	int p_atual_agY[2];
	int p_anterior_agX[2];
	int p_anterior_agY[2];
	TCHAR mapa_jogo[MAXSIZE][MAXSIZE][2];
	TCHAR simb[N]; //Simbolos
	int aleatorio_flag; // 0->Modo Predefinido 1-> Modo Aleatorio (Simbolos)
	TCHAR cmd_server[BUFFERSIZE];
	boolean joga; // 0 - até cmd "iniciar" | 1 - começa jogo
	int njogadores; //Total de jogadores
}MsgStruct;

typedef struct {
	TCHAR comando[BUFFERSIZE];
}BufferCirc;

typedef struct {
	BufferCirc buffer[TAMBUFFERCIRC]; //Buffer circular (estruturas)
	int posE; //proxima posicao de escrita
	int posL; //proxima posicao de leitura
	int nProdutores;
	int nConsumidores;
}Memoria;

typedef struct {
	int id;
	int larg; //x
	int comp; //y
	TCHAR nomeJog[MAX];
	TCHAR modoJogo[MAX];
	TCHAR mapa_jogo[MAXSIZE][MAXSIZE];
	TCHAR simb[N]; //Simbolos
	int posX; //Pos no tabuleiro
	int posY; //Pos no tabuleiro
	TCHAR tubo; //Jogada
	int resultado; //Resultado da jogada (1-ganhou,2-perdeu,3-sair)
	BOOL conectado;
	int continua;
}jogador;

typedef struct {
	HANDLE hPipe;
	OVERLAPPED ov;
	BOOL ligado;
}PipeDados;

typedef struct {
	PipeDados hPipe[2];
	HANDLE evento[2];
	HANDLE hMutex;
}ThreadNPDados;

typedef struct {
	MsgStruct* msgMemPart; //Ponteiro para a memoria partilhada
	Memoria* msgProdCons; //Ponteiro para produtor-consumidor
	int termina; // Controla se o jogo acaba ou se o user pretendeu sair
	//produtor-consumidor
	HANDLE sem_mutex_c;
	HANDLE sem_leitura, sem_escrita;
	//Escreve na memoria partilhada
	HANDLE evento;
	//Agua Timer
	HANDLE mutexTimer;
	HANDLE thTimer;
	HANDLE sema;
	HANDLE timer;
	LARGE_INTEGER li;
	TCHAR comando[BUFFERSIZE];
	//Controlo do Servidor e do jogo
	HANDLE thServerCtrl;
	HANDLE mutexSvCtrl;
	HANDLE eventoCMDSv;
	//Comunicacao Servidor-Monitor
	HANDLE hThreadServidorMonitor;
	//Interação Cliente-Servidor
	HANDLE hThreadComunicacaoNP;
	HANDLE hMutexSaida;
	HANDLE pipeLocal;
	HANDLE evento_pipe;
	jogador jogadores[2]; //Informação dos jogadores
	int tab[2]; //Controla mapas
	int total_jog;
	HANDLE mutexJogada;
	ThreadNPDados npDados;
}jogo;




//Funções & Threads
//Funções para o timer
void initClock();
__int64 startClock();
double stopClock(__int64 from);
//------
void gameOver(jogo* dados, int est, int jogador);				//Mostrar mensagem e indicar ao monitor que termina
void carregaMapaDefault(jogo* dados);							//Inicializa um mapa 10x10 com um circuito default
void moveAgua(jogo* dados);										//Move a agua consoante os objetos encontrados
void randomPreench(jogo* jg);									//Função de teste para introduzir simbolos random no mapa
void preencheSimb(jogo* s);										//Inicializa na forma predefinida os canos
void randomSimb(jogo* jg);										//Baralha a ordem dos canos
void preencheMapa(jogo* jg, int jogador);						//Inicializa o mapa em vazio apenas com o objeto de inicio (I) e fim (F)
boolean inserirParede(int x, int y, jogo* jg, int jogador);		//Insere uma parede indicada pelo monitor
DWORD WINAPI ThreadServerControl(LPVOID param);					//Thread usada para ler comandos do server
DWORD WINAPI AguaTimer(LPVOID param);							//Contabiliza os segundos de intervalo enquanto a agua se mexe
DWORD WINAPI ThreadServidorMonitor(LPVOID param);				//Thread responsável por receber os comandos do monitor e interpreta-los
