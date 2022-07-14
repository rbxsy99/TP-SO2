#define N 6
#define MAX 256
#define MAXSIZE 20
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
	MsgStruct* msgMemPart; //Ponteiro para a memoria partilhada
	Memoria* msgProdCons; //Ponteiro para produtor-consumidor
	int termina; // Controla se o jogo acaba ou se o user pretendeu sair
	//produtor-consumidor
	HANDLE sem_mutex_p;
	HANDLE sem_leitura, sem_escrita;
	//Aceder à memoria partilhada
	HANDLE evento;
	HANDLE thLeMemoria; //Ler memoria partilhada
	HANDLE mutexMem;
	TCHAR comando[BUFFERSIZE];
	//Comunicação monitor-servidor
	HANDLE hThreadMonServ;
	//Mutex mapa
	HANDLE mutexMapa;
}jogo;

//Funções & Threads
void mostraMapa(jogo* j);								//Mostra o mapa atual
DWORD WINAPI ThreadLeMemoria(LPVOID param);				//Thread responsável por ler da memória partilhada
DWORD WINAPI ThreadMonitorServidor(LPVOID param);		//Thread onde lê e envia comandos ao servidor



