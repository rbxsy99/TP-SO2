#include <Windows.h>
#include <tchar.h>
#include <string.h>
#include "../utils.h"
#include "cliente.h"
#include "resource2.h"

#define PIPE_NAME TEXT("\\\\.\\pipe\\tp_pipe") //-> pipe


void inicializaBitmaps(cliente* pd, HDC hdc);			//Inicializar todos os bitmaps e HDCs e guardar na struct
TCHAR getSimb(cliente* pd, int x, int y);				//Auxiliar
int getPosX(int x,int comp);							//Posição X no tabuleiro convertido de coordenadas para o mapa do serv
int getPosY(int y,int larg);							//Posição Y no tabuleiro convertido de coordenadas para o mapa do serv
BOOL checkPoint(int x, int y, int larg, int comp);		//Verificar se o Ponto está no mapa
HDC getHDC(cliente* pd, TCHAR c);						//Retornar o HDC consoante o TCHAR /TUBO/SIMB
void desenhaTabuleiro(cliente* pd, HDC hdc);			//Inicializar o tabuleiro+simbolos
LRESULT CALLBACK TrataEventos(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK TrataCaixa(HWND, UINT, WPARAM, LPARAM);//Dialog

TCHAR szProgName[] = TEXT("Jogo dos Tubos - Cliente");

void CALLBACK timer(HWND hWnd, UINT uMsg, UINT timerId, DWORD dwTime) //Atualiza janela de 2 em 2segs
{
	InvalidateRect(hWnd, NULL, TRUE); //Atualizar janela
}

DWORD WINAPI comunicacaoServidor(LPVOID param) {
	cliente* cl = (cliente*)param;
	jogador aux;
	DWORD n = sizeof(jogador);
	BOOL ret;
	do {
		if (cl->jog.conectado) {
			WaitForSingleObject(cl->hMutex, INFINITE);

			ZeroMemory(&cl->ov, sizeof(&cl->ov));
			ResetEvent(cl->evento);
			cl->ov.hEvent = cl->evento;
			if (!WriteFile(cl->hPipe, &cl->jog, sizeof(jogador), &n, &cl->ov)) {
				MessageBox(cl->hWnd, _T("Erro ao escrever"), _T("Status"), MB_OK);
				exit(-1);
			}
			WaitForSingleObject(cl->evento, INFINITE);
			GetOverlappedResult(cl->hPipe,&cl->ov, &n, FALSE);

			ZeroMemory(&aux, sizeof(jogador));
			ZeroMemory(&cl->ov, sizeof(&cl->ov));
			ResetEvent(cl->evento);
			cl->ov.hEvent = cl->evento;
			if (!ReadFile(cl->hPipe, &aux, sizeof(jogador), &n, &cl->ov)) {
				MessageBox(cl->hWnd, _T("Erro ao ler"), _T("Status"), MB_OK);
				exit(-1);
			}

			WaitForSingleObject(cl->evento, INFINITE);
			GetOverlappedResult(cl->hPipe, &cl->ov, &n, FALSE);
			
			//Trata os dados que recebe do servidor
			if (aux.comp != 0) { //Dados alterados
				cl->jog.comp = aux.comp;
				cl->jog.larg = aux.larg;
				for (int j = 0; j < aux.comp; j++) { //Copia o mapa
					for (int z = 0; z < aux.larg; z++) {
						cl->jog.mapa_jogo[j][z] = aux.mapa_jogo[j][z];
					}
				}
				for (int j = 0; j < N; j++) { //Copia os simbolos
					cl->jog.simb[j] = aux.simb[j];
				}
				cl->jog.continua = aux.continua;
				cl->jog.resultado = aux.resultado;
			}

			ReleaseMutex(cl->hMutex);
		}
	} while (cl->jog.continua != 1);
	if (cl->jog.resultado == 1) {
		if (MessageBox(cl->hWnd, _T("Ganhou o jogo"), _T("Status"), MB_OK) == IDOK) {
			cl->jog.resultado = 3;
		}
	}
	else if (cl->jog.resultado == 2) {
		if (MessageBox(cl->hWnd, _T("Perdeu o jogo"), _T("Status"), MB_OK) == IDOK) {
			cl->jog.resultado = 3;
		}
	}
	CloseHandle(cl->hPipe);
	CloseHandle(cl->hMutex);
	return 0;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow) {
	HWND hWnd; // hWnd é o handler da janela, gerado mais abaixo por CreateWindow()
	MSG lpMsg; 
	WNDCLASSEX wcApp; 
	jogador jog;
	cliente c;
	HANDLE threadComunicacao;


	//Inicializacao da estrutura
	c.jog.id = GetCurrentProcess();
	c.jog.conectado = FALSE;
	c.jog.continua = 0;
	c.jog.posX = 0;
	c.jog.posY = 0;
	c.jog.tubo = _T('?');
	c.jog.comp = 0;
	c.jog.larg = 0;

	

	if (!WaitNamedPipe(PIPE_NAME, NMPWAIT_WAIT_FOREVER)) {
		exit(-1);
	}

	c.hPipe = CreateFile(PIPE_NAME, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (c.hPipe == NULL) {
		exit(-1);
	}
	c.evento = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (c.evento == NULL) {
		exit(-1);
	}
	ZeroMemory(&c.ov, sizeof(c.ov));
	c.ov.hEvent = c.evento;
	//Criar thread para comunicação com o servidor
	c.threadComunicacao = CreateThread(NULL, 0, comunicacaoServidor, &c, 0, NULL);
	if(c.threadComunicacao == NULL){
		exit(-1);
	}
	c.hMutex = CreateMutex(NULL, FALSE, NULL);
	if (c.hMutex == NULL) {
		exit(-1);
	}
	c.hMutexAux = CreateMutex(NULL, FALSE, NULL);
	if (c.hMutexAux == NULL) {
		exit(-1);
	}

	wcApp.cbSize = sizeof(WNDCLASSEX); 
	wcApp.hInstance = hInst; 

	wcApp.lpszClassName = szProgName; 
	wcApp.lpfnWndProc = TrataEventos; // Endereço da função de processamento da janela
	wcApp.style = CS_HREDRAW | CS_VREDRAW;
	wcApp.hIcon = LoadIcon(NULL, IDI_SHIELD); //Shield - Icon barra de ferramentas
	wcApp.hIconSm = LoadIcon(NULL, IDI_ERROR); //Erro - Icon app
	wcApp.hCursor = LoadCursor(NULL, IDC_HAND);
	wcApp.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
	wcApp.cbClsExtra = 0;
	wcApp.cbWndExtra = sizeof(cliente);
	wcApp.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);


	if (!RegisterClassEx(&wcApp))
		return(0);

	hWnd = CreateWindow(
		szProgName, // Nome da janela (programa) definido acima
		TEXT("Jogo dos Tubos - Cliente"), // Texto que figura na barra do título
		WS_OVERLAPPEDWINDOW, // Estilo da janela (WS_OVERLAPPED= normal)
		50, // Posição x pixels (default=à direita da última)
		50, // Posição y pixels (default=abaixo da última)
		500, // Largura da janela (em pixels)
		700, // Altura da janela (em pixels)
		(HWND)HWND_DESKTOP, // handle da janela pai (se se criar uma a partir de
		// outra) ou HWND_DESKTOP se a janela for a primeira,
		// criada a partir do "desktop"
		(HMENU)NULL, // handle do menu da janela (se tiver menu)
		(HINSTANCE)hInst, // handle da instância do programa actual ("hInst" é
		// passado num dos parâmetros de WinMain()
		0); // Não há parâmetros adicionais para a janela

	c.hWnd = hWnd;

	SetTimer(hWnd, 0, 2000, (TIMERPROC)&timer); //A cada 2 seg. atualiza a janela
	SetWindowLongPtr(hWnd, 0, (LONG_PTR)&c);
	
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	

	while (GetMessage(&lpMsg, NULL, 0, 0)) {
		TranslateMessage(&lpMsg);
		DispatchMessage(&lpMsg);
	}

	return((int)lpMsg.wParam);
}

LRESULT CALLBACK TrataCaixa(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam) {
	static cliente* pd;
	switch (messg) {
		case WM_INITDIALOG:
			pd = (LPARAM)lParam;
			return 1;
		case WM_CLOSE:
			EndDialog(hWnd, 0);
			return 1;
		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK) { //Quando clicado no OK
				//Ler dados da edit text
				GetDlgItemText(hWnd, IDC_EDIT1, pd->jog.nomeJog, _countof(pd->jog.nomeJog));
				if (IsDlgButtonChecked(hWnd, IDC_RADIO1)) {
					_tcscpy_s(pd->jog.modoJogo, MAX, _T("Modo Velocidade"));
					pd->jog.conectado = TRUE;
				}
				if (IsDlgButtonChecked(hWnd, IDC_RADIO2)) {
					_tcscpy_s(pd->jog.modoJogo, MAX, _T("Modo Competitivo 1v1"));
					pd->jog.conectado = TRUE;
				}
				EndDialog(hWnd, 0);
			}
			else if (LOWORD(wParam) == IDCANCEL) {
				EndDialog(hWnd, 0);
			}
			return 1;
	}
	return 0;
}

void inicializaBitmaps(cliente * pd, HDC hdc) {
	pd->hBitMapAgua = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP_AGUA));
	pd->hdcBitMapAgua = CreateCompatibleDC(hdc);
	SelectObject(pd->hdcBitMapAgua, pd->hBitMapAgua);
	pd->hBitMapParede = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP7));
	pd->hdcBitMapParede = CreateCompatibleDC(hdc);
	SelectObject(pd->hdcBitMapParede, pd->hBitMapParede);
	pd->hBitMapVazio = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP_VAZIO));
	pd->hdcBitMapVazio = CreateCompatibleDC(hdc);
	SelectObject(pd->hdcBitMapVazio, pd->hBitMapVazio);
	pd->hBitMapTubo1 = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP1));
	pd->hdcBitMapTubo1 = CreateCompatibleDC(hdc);
	SelectObject(pd->hdcBitMapTubo1, pd->hBitMapTubo1);
	pd->hBitMapTubo2 = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP2));
	pd->hdcBitMapTubo2 = CreateCompatibleDC(hdc);
	SelectObject(pd->hdcBitMapTubo2, pd->hBitMapTubo2);
	pd->hBitMapTubo3 = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP3));
	pd->hdcBitMapTubo3 = CreateCompatibleDC(hdc);
	SelectObject(pd->hdcBitMapTubo3, pd->hBitMapTubo3);
	pd->hBitMapTubo4 = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP4));
	pd->hdcBitMapTubo4 = CreateCompatibleDC(hdc);
	SelectObject(pd->hdcBitMapTubo4, pd->hBitMapTubo4);
	pd->hBitMapTubo5 = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP5));
	pd->hdcBitMapTubo5 = CreateCompatibleDC(hdc);
	SelectObject(pd->hdcBitMapTubo5, pd->hBitMapTubo5);
	pd->hBitMapTubo6 = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP6));
	pd->hdcBitMapTubo6 = CreateCompatibleDC(hdc);
	SelectObject(pd->hdcBitMapTubo6, pd->hBitMapTubo6);
	pd->hBitMapInicio = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAPINICIO));
	pd->hdcBitMapInicio = CreateCompatibleDC(hdc);
	SelectObject(pd->hdcBitMapInicio, pd->hBitMapInicio);
	pd->hBitMapFim = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAPFIM));
	pd->hdcBitMapFim = CreateCompatibleDC(hdc);
	SelectObject(pd->hdcBitMapFim, pd->hBitMapFim);
}

TCHAR getSimb(cliente* pd, int x, int y) {
	int yaux = 0;
	if (pd->jog.comp > 10) {
		yaux = 550;
	}
	else {
		yaux = 350;
	}
	if (y >= yaux && y <= (yaux + 15)) {
		if (x >= 150 && x <= 165) { //Primeiro Simb
			return pd->jog.simb[0];
		}
		else if (x >= 190 && x <= 205) {
			return pd->jog.simb[1];
		}
		else if (x >= 230 && x <= 245) {
			return pd->jog.simb[2];
		}
		else if (x >= 270 && x <= 285) {
			return pd->jog.simb[3];
		}
		else if (x >= 310 && x <= 325) {
			return pd->jog.simb[4];
		}
		else if (x >= 350 && x <= 365) {
			return pd->jog.simb[5];
		}
	}
	return NULL;
}

int getPosX(int x, int comp) {
	int xInicial = 0;
	if (comp <= 10) { //p0: (140,60)
		xInicial = 140;
		for (int i = 0; i < comp; i++) {
			if (x >= xInicial && x <= (xInicial + 15)) {
				return i;
			}
			xInicial += 20;
		}
	}
	else { //p0: (50,60)
		xInicial = 50;
		for (int i = 0; i < comp; i++) {
			if (x >= xInicial && x <= (xInicial + 15)) {
				return i;
			}
			xInicial += 20;
		}
	}
	return 0;
}

int getPosY(int y,int larg) {
	int yInicial = 60;
	for (int i = 0; i < larg; i++) {
		if (y >= yInicial && y <= (yInicial + 15)) {
			return i;
		}
		yInicial += 20;
	}
	return 0;
}

BOOL checkPoint(int x, int y,int larg,int comp) {
	int yInicial = 60,yMax = 0;
	int xInicial, xMax;
	if (comp > 10) {
		yMax = 550;
	}
	else {
		yMax = 350;
	}
	if (y < yInicial && y > yMax) { //Verificar o Y
		return FALSE;
	}
	if (comp <= 10) {
		xInicial = 140;
	}
	else {
		xInicial = 50;
	}
	xMax = xInicial + 20 * larg;
	if (x < xInicial && x > xMax) {
		return FALSE;
	}

	return TRUE;
}

HDC getHDC(cliente* pd,TCHAR c) {
	if (c == _T('A')) { //Água
		return pd->hdcBitMapAgua;
	}
	else if (c == _T('X')) { //Parede
		return pd->hdcBitMapParede;
	}
	else if (c == _T('■')) { //Vazio
		return pd->hdcBitMapVazio;
	}
	else if (c == _T('━')) { //Tubo1
		return pd->hdcBitMapTubo1;
	}
	else if (c == _T('┃')) { //Tubo2
		return pd->hdcBitMapTubo2;
	}
	else if (c == _T('┏')) { //Tubo3
		return pd->hdcBitMapTubo3;
	}
	else if (c == _T('┓')) { //Tubo4
		return pd->hdcBitMapTubo4;
	}
	else if (c == _T('┛')) { //Tubo5
		return pd->hdcBitMapTubo5;
	}
	else if (c == _T('┗')) { //Tubo6
		return pd->hdcBitMapTubo6;
	}
	else if (c == _T('I')) { //Inicio
		return pd->hdcBitMapInicio;
	}
	else if (c == _T('F')) { //Fim
		return pd->hdcBitMapFim;
	}
	return NULL;
}

void desenhaTabuleiro(cliente* pd, HDC hdc) {
	int x = 140, y = 60;
	int i;
	HDC hdcAux;
	if (pd->jog.comp > 10) {
		x = 50;
	}
	for (i = 0; i < pd->jog.comp; i++) {
		for (int j = 0; j < pd->jog.larg; j++) {
			hdcAux = getHDC(pd,pd->jog.mapa_jogo[i][j]);
			BitBlt(hdc,x , y, 15, 15, hdcAux, 0, 0, SRCCOPY);
			x += 20;
		}
		if (pd->jog.comp > 10) {
			x = 50;
		}
		else {
			x = 140;
		}
		y += 20;
	}
	//Mostrar as opções
	if (pd->jog.comp > 10) {
		y = 550;
	}
	else {
		y = 350;
	}
	x = 150;
	//Coordenadas 1º: (150,y) 2º: (190,y) 3º: (230,y) 4º: (270,y) 5º: (310,y) 6º: (350,y)
	for(i = 0; i < N; i++) {
		hdcAux = getHDC(pd, pd->jog.simb[i]);
		BitBlt(hdc, x, y, 15, 15, hdcAux, 0, 0, SRCCOPY);
		x += 40;
	}
}

LRESULT CALLBACK TrataEventos(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam) {
	HDC hdc;
	PAINTSTRUCT ps;
	static int x = 0, y = 0;
	cliente* pd = (cliente*)GetWindowLongPtr(hWnd, 0);
	static TCHAR proxJogada;
	static int countSimb = 0;
	
	switch (messg) {
		case WM_CREATE:
			break;
		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			SetBkMode(hdc, TRANSPARENT);
			TextOut(hdc, 190, 30, _T("Jogo dos Tubos"), _tcslen(_T("Jogo dos Tubos")));
			if (pd->jog.comp <= 10) {
				TextOut(hdc, 190, 300, _T("Opções de Jogadas"), _tcslen(_T("Opções de Jogadas")));
			}
			else {
				TextOut(hdc, 190, 500, _T("Opções de Jogadas"), _tcslen(_T("Opções de Jogadas")));
			}
			SetTextAlign(hdc, TA_CENTER);
			if (pd->jog.comp > 0) { //Desenha tabuleiro
				desenhaTabuleiro(pd, hdc);
			}
			if (pd->jog.resultado == 3) {
				PostQuitMessage(0);
			}
			EndPaint(hWnd, &ps);
			break;
		case WM_KEYDOWN:
			if (wParam == VK_F1) {
				InvalidateRect(hWnd, NULL, TRUE); //Atualizar janela
			}
		case WM_COMMAND: //Tratar menu
			if (!pd->jog.conectado) { //Se o jogador ainda não tiver conectado ao servidor
				if (LOWORD(wParam) == ID_INICIAR_DADOS) {
					//DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG1), hWnd, TrataCaixa);
					DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG1), hWnd, TrataCaixa, (LPARAM)pd);
					hdc = GetDC(hWnd);
					inicializaBitmaps(pd, hdc);
					DeleteDC(hdc);
					InvalidateRect(hWnd, NULL, TRUE); //Atualizar janela
				}
			}
			break;
		case WM_RBUTTONDOWN: //Escolher tubo
			if (pd->jog.comp > 0) {
				//TCHAR teste[MAX];
				x = LOWORD(lParam);
				y = HIWORD(lParam);
				pd->jog.posX = getPosX(x, pd->jog.comp);
				pd->jog.posY = getPosY(y, pd->jog.larg);
				if ((pd->jog.posX != 0 || pd->jog.posY != 0) && checkPoint(pd->jog.posX, pd->jog.posY, pd->jog.larg, pd->jog.comp)) {
					pd->jog.tubo = _T('■');
					InvalidateRect(hWnd, NULL, TRUE); //Atualizar janela
				}
				break;
			}
			break;
		case WM_LBUTTONDOWN: //Realizar jogada
			x = LOWORD(lParam);
			y = HIWORD(lParam);
			pd->jog.posX = getPosX(x, pd->jog.comp);
			pd->jog.posY = getPosY(y, pd->jog.larg);
			if (pd->jog.simb[countSimb] != NULL && (pd->jog.posX != 0 || pd->jog.posY != 0) && checkPoint(pd->jog.posX,pd->jog.posY,pd->jog.larg,pd->jog.comp)) {
				pd->jog.tubo = pd->jog.simb[countSimb];
				countSimb++;
				if (countSimb == 6) {
					countSimb = 0;
				}
				InvalidateRect(hWnd, NULL, TRUE); //Atualizar janela
			}
			//InvalidateRect(hWnd, NULL, TRUE); //Atualizar janela
			break;
		case WM_CLOSE:
			if (MessageBox(hWnd, _T("Pretende mesmo sair?"), _T("Confirme"), MB_YESNO | MB_ICONINFORMATION) == IDYES) {
				_tcscpy_s(pd->jog.resultado, MAX, _T("sair"));
				pd->jog.conectado = FALSE;
				pd->jog.continua = 1;
				CloseHandle(pd->hPipe);
				PostQuitMessage(0);
			}
		case WM_DESTROY: // Destruir a janela e terminar o programa
		// "PostQuitMessage(Exit Status)"
			//PostQuitMessage(0);
			break;
		default:
			// Neste exemplo, para qualquer outra mensagem (p.e. "minimizar","maximizar","restaurar")
			// não é efectuado nenhum processamento, apenas se segue o "default" do Windows
			return(DefWindowProc(hWnd, messg, wParam, lParam));
			break; // break tecnicamente desnecessário por causa do return
	}

	return(0);
}







