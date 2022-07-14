#define N 6
#define MAX 256
#define MAXSIZE 20

typedef struct {
	int id;
	int larg; //x
	int comp; //y
	TCHAR nomeJog[MAX];
	TCHAR modoJogo[MAX];
	TCHAR mapa_jogo[MAXSIZE][MAXSIZE];
	TCHAR simb[N]; //Simbolos
	int posX;
	int posY;
	TCHAR tubo; //Jogada
	int resultado; //Resultado da jogada (1-ganhou,2-perdeu,3-sair)
	BOOL conectado;
	int continua;
}jogador;

typedef struct {
	jogador jog;
	HANDLE threadComunicacao;
	HANDLE hMutex;
	HANDLE hMutexAux;
	HANDLE hPipe;
	OVERLAPPED ov;
	HANDLE evento;
	HWND hWnd;
	//Bitmaps & HDCs
	HBITMAP hBitMapAgua;
	HDC hdcBitMapAgua;
	HBITMAP hBitMapParede;
	HDC hdcBitMapParede;
	HBITMAP hBitMapVazio;
	HDC hdcBitMapVazio;
	HBITMAP hBitMapTubo1;
	HDC hdcBitMapTubo1;
	HBITMAP hBitMapTubo2;
	HDC hdcBitMapTubo2;
	HBITMAP hBitMapTubo3;
	HDC hdcBitMapTubo3;
	HBITMAP hBitMapTubo4;
	HDC hdcBitMapTubo4;
	HBITMAP hBitMapTubo5;
	HDC hdcBitMapTubo5;
	HBITMAP hBitMapTubo6;
	HDC hdcBitMapTubo6;
	HBITMAP hBitMapInicio;
	HDC hdcBitMapInicio;
	HBITMAP hBitMapFim;
	HDC hdcBitMapFim;
}cliente;