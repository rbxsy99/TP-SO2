#include <io.h>
#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include "../utils.h"
#include "servidor.h"

#define PIPE_NAME TEXT("\\\\.\\pipe\\tp_pipe") //-> pipe

static double PerfCounterFreq; // n ticks por seg.

void initClock() {
    LARGE_INTEGER aux;
    if (!QueryPerformanceFrequency(&aux))
        _tprintf(TEXT("\nSorry - No can do em QueryPerfFreq\n"));
    PerfCounterFreq = (double)(aux.QuadPart); // / 1000.0;
    //_tprintf(TEXT("\nTicks por sec.%f\n"), PerfCounterFreq);
}

__int64 startClock() {
    LARGE_INTEGER aux;
    QueryPerformanceCounter(&aux);
    return aux.QuadPart;
}

double stopClock(__int64 from) {
    LARGE_INTEGER aux;
    QueryPerformanceCounter(&aux);
    return (double)(aux.QuadPart - from) / PerfCounterFreq;
}

void gameOver(jogo* dados, int est,int jogador) {
    _tprintf(_T("Est: %d Jogador: %d"), est, jogador);
    if (dados->tab[jogador] == 0) {
        if (dados->total_jog == 0) { //Jogo offline s/jogadores
            dados->termina = 1;
            _tcscpy_s(dados->msgMemPart->cmd_server, BUFFERSIZE, _T("terminar"));
            SetEvent(dados->evento);
            Sleep(1000);
            for (int i = 0; i < 2; i++) { //Desbloquear WaitForMultipleObject
                SetEvent(dados->npDados.evento[i]);
            }
        }
        else {
            if (_tcscmp(dados->jogadores[0].modoJogo, _T("Modo Velocidade")) == 0 && jogador == 0) { //Modo Sozinho
                if (est == 1) { //Ganhou
                    _tprintf(_T("\n[JOGO] O jogador %s ganhou o jogo.\n"), dados->jogadores[jogador].nomeJog);
                    dados->jogadores[jogador].continua = 1;
                    dados->jogadores[jogador].resultado = 1;
                }
                else if (est == 0) { //Perdeu
                    _tprintf(_T("\n[JOGO] O jogador %s perdeu o jogo.\n"), dados->jogadores[jogador].nomeJog);
                    dados->jogadores[jogador].continua = 1;
                    dados->jogadores[jogador].resultado = 2;
                }
            }
            else if(_tcscmp(dados->jogadores[jogador].modoJogo, _T("Modo Competitivo 1v1")) == 0){ //Modo 1v1
                if (est == 1) { //Ganhou
                    _tprintf(_T("\n[JOGO] O jogador %s ganhou o jogo.\n"), dados->jogadores[jogador].nomeJog);
                    dados->jogadores[jogador].continua = 1;
                    dados->jogadores[jogador].resultado = 1;
                }
                else if (est == 0) { //Perdeu
                    _tprintf(_T("\n[JOGO] O jogador %s perdeu o jogo.\n"), dados->jogadores[jogador].nomeJog);
                    dados->jogadores[jogador].continua = 1;
                    dados->jogadores[jogador].resultado = 2;
                }
            }
            dados->tab[jogador] = 1;
        }
    }
}

void carregaMapaDefault(jogo* dados) {
    dados->msgMemPart->comp = 10;
    dados->msgMemPart->larg = 10;
    for (int i = 0; i < 2; i++) {
        dados->msgMemPart->mapa_jogo[0][0][i] = _T('I');
        dados->msgMemPart->mapa_jogo[1][0][i] = _T('┗');
        dados->msgMemPart->mapa_jogo[1][1][i] = _T('━');
        dados->msgMemPart->mapa_jogo[1][2][i] = _T('━');
        dados->msgMemPart->mapa_jogo[1][3][i] = _T('━');
        dados->msgMemPart->mapa_jogo[1][4][i] = _T('┓');
        dados->msgMemPart->mapa_jogo[2][4][i] = _T('┃');
        dados->msgMemPart->mapa_jogo[3][4][i] = _T('┃');
        dados->msgMemPart->mapa_jogo[4][4][i] = _T('┃');
        dados->msgMemPart->mapa_jogo[5][4][i] = _T('┗');
        dados->msgMemPart->mapa_jogo[5][5][i] = _T('━');
        dados->msgMemPart->mapa_jogo[5][6][i] = _T('━');
        dados->msgMemPart->mapa_jogo[5][7][i] = _T('┓');
        dados->msgMemPart->mapa_jogo[6][7][i] = _T('┗');
        dados->msgMemPart->mapa_jogo[6][8][i] = _T('━');
        dados->msgMemPart->mapa_jogo[6][9][i] = _T('┓');
        dados->msgMemPart->mapa_jogo[7][9][i] = _T('┃');
        dados->msgMemPart->mapa_jogo[8][9][i] = _T('┃');
        dados->msgMemPart->mapa_jogo[9][9][i] = _T('F');
    }
    
}

void moveAgua(jogo* dados) {
    TCHAR c;
    //Mover a agua nos dois mapas
    for (int i = 0; i < dados->total_jog; i++) {
        c = (TCHAR)dados->msgMemPart->mapa_jogo[dados->msgMemPart->p_atual_agY[i]][dados->msgMemPart->p_atual_agX[i]][i];
        if (c == _T('━')) {
            if ((dados->msgMemPart->p_anterior_agX[i] - dados->msgMemPart->p_atual_agX[i]) == -1) {//Agua vem da esquerda
                dados->msgMemPart->p_anterior_agX[i] = dados->msgMemPart->p_atual_agX[i];
                dados->msgMemPart->mapa_jogo[dados->msgMemPart->p_atual_agY[i]][dados->msgMemPart->p_atual_agX[i]][i] = _T('A');
                dados->msgMemPart->p_atual_agX[i]++;
            }
            else if ((dados->msgMemPart->p_anterior_agX[i] - dados->msgMemPart->p_atual_agX[i]) == 1) {//Agua vem da direita
                dados->msgMemPart->p_anterior_agX[i] = dados->msgMemPart->p_atual_agX[i];
                dados->msgMemPart->mapa_jogo[dados->msgMemPart->p_atual_agY[i]][dados->msgMemPart->p_atual_agX[i]][i] = _T('A');
                dados->msgMemPart->p_atual_agX[i]--;
            }
            else {
                gameOver(dados, 0,i);
            }
        }
        else if (c == _T('┃')) {
            if ((dados->msgMemPart->p_anterior_agY[i] - dados->msgMemPart->p_atual_agY[i]) == 1) {//Agua vem de baixo
                dados->msgMemPart->p_anterior_agY[i] = dados->msgMemPart->p_atual_agY[i];
                dados->msgMemPart->mapa_jogo[dados->msgMemPart->p_atual_agY[i]][dados->msgMemPart->p_atual_agX[i]][i] = _T('A');
                dados->msgMemPart->p_atual_agY[i]--;
            }
            else if ((dados->msgMemPart->p_anterior_agY[i] - dados->msgMemPart->p_atual_agY[i]) == -1) {//Agua vem da cima
                dados->msgMemPart->p_anterior_agY[i] = dados->msgMemPart->p_atual_agY[i];
                dados->msgMemPart->mapa_jogo[dados->msgMemPart->p_atual_agY[i]][dados->msgMemPart->p_atual_agX[i]][i] = _T('A');
                dados->msgMemPart->p_atual_agY[i]++;
            }
            else {
                gameOver(dados, 0,i);
            }
        }
        else if (c == _T('┏')) {
            if ((dados->msgMemPart->p_anterior_agY[i] - dados->msgMemPart->p_atual_agY[i]) == 1) {//Agua vem de baixo
                dados->msgMemPart->p_anterior_agX[i] = dados->msgMemPart->p_atual_agX[i];
                dados->msgMemPart->mapa_jogo[dados->msgMemPart->p_atual_agY[i]][dados->msgMemPart->p_atual_agX[i]][i] = _T('A');
                dados->msgMemPart->p_atual_agX[i]++;
            }
            else if ((dados->msgMemPart->p_anterior_agX[i] - dados->msgMemPart->p_atual_agX[i]) == 1) {//Agua vem da direita
                dados->msgMemPart->p_anterior_agY[i] = dados->msgMemPart->p_atual_agY[i];
                dados->msgMemPart->mapa_jogo[dados->msgMemPart->p_atual_agY[i]][dados->msgMemPart->p_atual_agX[i]][i] = _T('A');
                dados->msgMemPart->p_atual_agY[i]++;
            }
            else {
                gameOver(dados, 0,i);
            }
        }
        else if (c == _T('┓')) {
            if ((dados->msgMemPart->p_anterior_agY[i] - dados->msgMemPart->p_atual_agY[i]) == 1) {//Agua vem de baixo
                dados->msgMemPart->p_anterior_agX[i] = dados->msgMemPart->p_atual_agX[i];
                dados->msgMemPart->mapa_jogo[dados->msgMemPart->p_atual_agY[i]][dados->msgMemPart->p_atual_agX[i]][i] = _T('A');
                dados->msgMemPart->p_atual_agX[i]--;
            }
            else if ((dados->msgMemPart->p_anterior_agX[i] - dados->msgMemPart->p_atual_agX[i]) == -1) {//Agua vem da esquerda
                dados->msgMemPart->p_anterior_agY[i] = dados->msgMemPart->p_atual_agY[i];
                dados->msgMemPart->mapa_jogo[dados->msgMemPart->p_atual_agY[i]][dados->msgMemPart->p_atual_agX[i]][i] = _T('A');
                dados->msgMemPart->p_atual_agY[i]++;
            }
            else {
                gameOver(dados, 0,i);
            }
        }
        else if (c == _T('┛')) {
            if ((dados->msgMemPart->p_anterior_agY[i] - dados->msgMemPart->p_atual_agY[i]) == -1) {//Agua vem de cima
                dados->msgMemPart->p_anterior_agX[i] = dados->msgMemPart->p_atual_agX[i];
                dados->msgMemPart->mapa_jogo[dados->msgMemPart->p_atual_agY[i]][dados->msgMemPart->p_atual_agX[i]][i] = _T('A');
                dados->msgMemPart->p_atual_agX[i]--;
            }
            else if ((dados->msgMemPart->p_anterior_agX[i] - dados->msgMemPart->p_atual_agX[i]) == -1) {//Agua vem da esquerda
                dados->msgMemPart->p_anterior_agY[i] = dados->msgMemPart->p_atual_agY[i];
                dados->msgMemPart->mapa_jogo[dados->msgMemPart->p_atual_agY[i]][dados->msgMemPart->p_atual_agX[i]][i] = _T('A');
                dados->msgMemPart->p_atual_agY[i]--;
            }
            else {
                gameOver(dados, 0,i);
            }
        }
        else if (c == _T('┗')) {
            if ((dados->msgMemPart->p_anterior_agY[i] - dados->msgMemPart->p_atual_agY[i]) == -1) {//Agua vem de cima
                dados->msgMemPart->p_anterior_agX[i] = dados->msgMemPart->p_atual_agX[i];
                dados->msgMemPart->mapa_jogo[dados->msgMemPart->p_atual_agY[i]][dados->msgMemPart->p_atual_agX[i]][i] = _T('A');
                dados->msgMemPart->p_atual_agX[i]++;
            }
            else if ((dados->msgMemPart->p_anterior_agX[i] - dados->msgMemPart->p_atual_agX[i]) == 1) {//Agua vem da direita
                dados->msgMemPart->p_anterior_agY[i] = dados->msgMemPart->p_atual_agY[i];
                dados->msgMemPart->mapa_jogo[dados->msgMemPart->p_atual_agY[i]][dados->msgMemPart->p_atual_agX[i]][i] = _T('A');
                dados->msgMemPart->p_atual_agY[i]--;
            }
            else {
                gameOver(dados, 0,i);
            }

        }
        else if (c == _T('I')) {
            if (dados->msgMemPart->p_atual_agY[i] == (dados->msgMemPart->larg - 1)) {//Ponto inicial está na parte inferior do mapa então água sobe
                dados->msgMemPart->p_anterior_agY[i] = dados->msgMemPart->p_atual_agY[i];
                dados->msgMemPart->mapa_jogo[dados->msgMemPart->p_atual_agY[i]][dados->msgMemPart->p_atual_agX[i]][i] = _T('A');
                dados->msgMemPart->p_atual_agY[i]--;
            }
            else
            {                                                                     //Ponto inicial não está na parte inferior do mapa então água irá descer
                dados->msgMemPart->p_anterior_agY[i] = dados->msgMemPart->p_atual_agY[i];
                dados->msgMemPart->mapa_jogo[dados->msgMemPart->p_atual_agY[i]][dados->msgMemPart->p_atual_agX[i]][i] = _T('A');
                dados->msgMemPart->p_atual_agY[i]++;
            }
        }
        else if (c == _T('F')) {
            gameOver(dados, 1,i);
        }
        else { //Parede intransponível
            gameOver(dados, 0,i);
        }
    }
}

//Testes
void randomPreench(jogo* jg) {
    int rand_c = rand() % jg->msgMemPart->comp;
    int rand_l = rand() % jg->msgMemPart->larg;
    int rand_cano = rand() % 6;
}

void preencheSimb(jogo* s) {
    s->msgMemPart->simb[0] = _T('━');
    s->msgMemPart->simb[1] = _T('┃');
    s->msgMemPart->simb[2] = _T('┏');
    s->msgMemPart->simb[3] = _T('┓');
    s->msgMemPart->simb[4] = _T('┛');
    s->msgMemPart->simb[5] = _T('┗');
}

void randomSimb(jogo* jg) {
    TCHAR aux;
    for (int i = 0; i < DIM - 1; i++)
    {
        int j = rand() % 6;
        aux = jg->msgMemPart->simb[j];
        jg->msgMemPart->simb[j] = jg->msgMemPart->simb[i];
        jg->msgMemPart->simb[i] = aux;
    }
}

void preencheMapa(jogo* jg, int jogador) {
    if (jogador == 0) {
        //Preencher os dois mapas
        for (int i = 0; i < jg->msgMemPart->comp; i++) {
            for (int j = 0; j < jg->msgMemPart->larg; j++) {
                jg->msgMemPart->mapa_jogo[i][j][0] = _T('■');
                jg->msgMemPart->mapa_jogo[i][j][1] = _T('■');
            }
        }
        //Preencher ponto inicial e final - Mapa1
        jg->msgMemPart->mapa_jogo[jg->msgMemPart->pontoinicialX[0]][jg->msgMemPart->pontoinicialY[0]][0] = _T('I');
        jg->msgMemPart->mapa_jogo[jg->msgMemPart->destinoaguaX[0]][jg->msgMemPart->destinoaguaY[0]][0] = _T('F');
        //Preencher ponto inicial e final - Mapa2
        jg->msgMemPart->mapa_jogo[jg->msgMemPart->pontoinicialX[1]][jg->msgMemPart->pontoinicialY[1]][1] = _T('I');
        jg->msgMemPart->mapa_jogo[jg->msgMemPart->destinoaguaX[1]][jg->msgMemPart->destinoaguaY[1]][1] = _T('F');
    }
    else { //Preencher de novo apenas o mapa do jogador indicado
        for (int i = 0; i < jg->msgMemPart->comp; i++) {
            for (int j = 0; j < jg->msgMemPart->larg; j++) {
                jg->msgMemPart->mapa_jogo[i][j][jogador-1] = _T('■');
            }
        }
        //Preencher ponto inicial e final - Mapa
        jg->msgMemPart->mapa_jogo[jg->msgMemPart->pontoinicialX[jogador - 1]][jg->msgMemPart->pontoinicialY[0]][jogador - 1] = _T('I');
        jg->msgMemPart->mapa_jogo[jg->msgMemPart->destinoaguaX[jogador - 1]][jg->msgMemPart->destinoaguaY[0]][jogador - 1] = _T('F');
    }
    
}

boolean inserirParede(int x, int y,jogo *jg, int jogador) {
    if (jogador != 1 && jogador != 2) {
        return FALSE;
    }
    jogador--;
    //_tprintf(_T("Comando inserir.Comp: %d Larg: %d\n"),x,y);
    if (!(x > 0 && x < jg->msgMemPart->comp) || !(y > 0 && y < jg->msgMemPart->larg)) {
        return FALSE;
    }
    for (int i = 0; i < N; i++) {
        if (jg->msgMemPart->mapa_jogo[x][y][jogador] == jg->msgMemPart->simb[i]) {
            return FALSE;
        }
    }
    if (jg->msgMemPart->mapa_jogo[x][y][jogador] == _T('I') || jg->msgMemPart->mapa_jogo[x][y][jogador] == _T('F')) {
        return FALSE;
    }
    if (jg->msgMemPart->mapa_jogo[x][y][jogador] == _T('A')) {
        return FALSE;
    }
    jg->msgMemPart->mapa_jogo[x][y][jogador] = _T('X');
    SetEvent(jg->evento);
    Sleep(500);
    ResetEvent(jg->evento);
    return TRUE;
}

DWORD WINAPI ThreadComunicacaoNP(LPVOID param) {
    jogo* p = (jogo*)param;
    DWORD n ;
    jogador aux;
    do {
        for (int i = 0; i < p->total_jog; i++) {
            WaitForSingleObject(p->npDados.hMutex, INFINITE);
            if (p->npDados.hPipe[i].ligado) {
                ZeroMemory(&aux, sizeof(jogador));
                if (!ReadFile(p->npDados.hPipe[i].hPipe, &aux, sizeof(jogador), &n,NULL)) {
                    if (p->total_jog == 2 && i == 0) {
                        ZeroMemory(&p->jogadores[i], sizeof(jogador)); //Apaga memória desse jogador
                        p->jogadores[i + 1] = p->jogadores[i];
                        p->total_jog--;
                    }
                    else {
                        ZeroMemory(&p->jogadores[i], sizeof(jogador)); //Apaga memória desse jogador
                        p->total_jog--;
                    }
                    //CloseHandle(p->npDados.hPipe[i].hPipe);
                    ReleaseMutex(p->npDados.hMutex);
                    break;
                }

                WaitForSingleObject(p->mutexJogada, INFINITE);
                //Verificar modo de jogo
                if (_tcscmp(aux.modoJogo, _T("Modo Velocidade")) == 0) {
                    p->msgMemPart->joga = TRUE;
                    SetEvent(p->evento);
                }
                else if (_tcscmp(p->jogadores[1].modoJogo, _T("Modo Competitivo 1v1")) == 0 && p->total_jog == 2) {
                    p->msgMemPart->joga = TRUE;
                    SetEvent(p->evento);
                }
                //Verifica jogada do jogador e coloca no mapa do jogo indicado para o jogador
                if (aux.posX <= p->msgMemPart->larg && aux.posY <= p->msgMemPart->comp) {
                    if (aux.posX == 0 && aux.posY == 0 && aux.tubo == _T('?')) { //Ainda não realizada jogada
                        aux.comp = p->msgMemPart->comp;
                        aux.larg = p->msgMemPart->larg;
                        for (int j = 0; j < p->msgMemPart->comp;j++) { //Copia o mapa
                            for (int z = 0; z < p->msgMemPart->larg; z++) {
                                aux.mapa_jogo[j][z] = p->msgMemPart->mapa_jogo[j][z][i];
                            }
                        }
                        for (int j = 0; j < N; j++) { //Copia os simbolos
                            aux.simb[j] = p->msgMemPart->simb[j];
                        }
                    }
                    else { //Faz Jogada
                        if (aux.tubo != _T('?') && (aux.posX != 0 || aux.posY != 0)) {
                            TCHAR auxTubo = p->msgMemPart->mapa_jogo[aux.posY][aux.posX][i];
                            if (auxTubo != _T('■') || auxTubo != _T('A') || auxTubo != _T('X') || auxTubo != _T('I') || auxTubo != _T('F')) {
                                p->msgMemPart->mapa_jogo[aux.posY][aux.posX][i] = aux.tubo;
                                aux.posX = 0;
                                aux.posY = 0;
                                aux.tubo = _T('?');
                            }
                        }
                        //Envia tabuleiro atualizado e os simbolos
                        for (int j = 0; j < p->msgMemPart->comp; j++) { //Copia o mapa
                            for (int z = 0; z < p->msgMemPart->larg; z++) {
                                aux.mapa_jogo[j][z] = p->msgMemPart->mapa_jogo[j][z][i];
                            }
                        }
                        for (int j = 0; j < N; j++) { //Copia os simbolos
                            aux.simb[j] = p->msgMemPart->simb[j];
                        }
                    }
                }
                aux.continua = p->jogadores[i].continua;
                aux.resultado = p->jogadores[i].resultado;
                //Atualiza os dados do jogador
                p->jogadores[i] = aux;
                ReleaseMutex(p->mutexJogada);

                
                if (!WriteFile(p->npDados.hPipe[i].hPipe, &aux, sizeof(jogador), &n, NULL)) {
                    //_tprintf(_T("Erro a escrever no pipe.\n"));
                    if (p->total_jog == 2 && i == 0) {
                        ZeroMemory(&p->jogadores[i], sizeof(jogador)); //Apaga memória desse jogador
                        p->jogadores[i + 1] = p->jogadores[i];
                        p->total_jog--;
                    }
                    else {
                        ZeroMemory(&p->jogadores[i], sizeof(jogador)); //Apaga memória desse jogador
                        p->total_jog--;
                    }
                    //CloseHandle(p->npDados.hPipe[i].hPipe);
                    ReleaseMutex(p->npDados.hMutex);
                    break;
                }
            }
            ReleaseMutex(p->npDados.hMutex);
        }
    } while (p->termina != 1);
    return 0;
}

DWORD WINAPI ThreadServerControl(LPVOID param) {
    jogo* p = (jogo*)param;
    TCHAR comando[BUFFERSIZE];
    do{
        WaitForSingleObject(p->mutexSvCtrl, INFINITE);
        _fgetts(comando, BUFFERSIZE, stdin);
        comando[_tcslen(comando) - 1] = '\0'; //strlen - wcslen
        if (_tcsncmp(comando, _T("terminar"), 7) == 0) {
            _tprintf(_T("Jogo terminado pelo servidor.\n"));
            _tcscpy_s(p->msgMemPart->cmd_server, BUFFERSIZE, comando);
            for (int i = 0; i < p->total_jog; i++) { //Desligar jogadores ativos
                p->jogadores[i].continua = 1;
            }
            Sleep(1000);
            p->termina = 1;
            for (int i = 0; i < 2; i++) { //Desbloquear WaitForMultipleObject
                SetEvent(p->npDados.evento[i]);
            }
        }
        else if (_tcsncmp(comando, _T("parar"), 5) == 0) {
            _tcscpy_s(p->msgMemPart->cmd_server, BUFFERSIZE, comando);
            _tprintf(_T("Jogo parado pelo servidor.\n"));
            SetEvent(p->evento);
            Sleep(500);
            //ResetEvent(p->evento);
        }
        else if (_tcsncmp(comando, _T("retomar"), 7) == 0) {
            _tcscpy_s(p->msgMemPart->cmd_server, BUFFERSIZE, comando);
            _tprintf(_T("Jogo retomado pelo servidor.\n"));
            SetEvent(p->evento);
            Sleep(500);
            //ResetEvent(p->evento);
        }
        else if (_tcsncmp(comando, _T("iniciar"), 7) == 0) { //Iniciar o jogo
            _tcscpy_s(p->msgMemPart->cmd_server, BUFFERSIZE, comando);
            p->msgMemPart->joga = TRUE;
            SetEvent(p->evento);
            Sleep(500);
            //ResetEvent(p->evento);
        }
        ReleaseMutex(p->mutexSvCtrl);
    }while (p->termina != 1);
    return 0;
}

DWORD WINAPI AguaTimer(LPVOID param) {
    jogo* dados = (jogo*)param;
    __int64 clockticks;
    //Esperar o timer da agua
    //Mutex
    do{
        WaitForSingleObject(dados->mutexTimer, INFINITE);
        WaitForSingleObject(dados->sema, INFINITE);
        if (!_tcscmp(dados->msgMemPart->cmd_server, _T("parar")) == 0 && dados->msgMemPart->joga == 1) {
            if (dados->msgMemPart->t_agua > 0 && dados->msgMemPart->seg_stopagua == 0) {
                dados->li.QuadPart = -dados->msgMemPart->t_agua * 10000000LL;
                //Programar o WaitableTimer
                if (!SetWaitableTimer(dados->timer, &dados->li, 0, NULL, NULL, 0)) {
                    _tprintf(_T("Erro na criação do WaitableTimer %d\n"), GetLastError());
                    return -1;
                }
                clockticks = startClock();
                WaitForSingleObject(dados->timer, INFINITE);
                stopClock(clockticks);
                //Move Agua
                moveAgua(dados);
                _tprintf(_T("\n[TIMER] Água em movimento após %d segundos de intervalo.\n"), dados->msgMemPart->t_agua);
                SetEvent(dados->evento);
                Sleep(500);
                //ResetEvent(dados->evento);
            }
            else if (dados->msgMemPart->seg_stopagua > 0) {
                //Parar a agua por x segundos
                dados->li.QuadPart = -dados->msgMemPart->seg_stopagua * 10000000LL;
                if (!SetWaitableTimer(dados->timer, &dados->li, 0, NULL, NULL, 0)) {
                    _tprintf(_T("Erro na criação do WaitableTimer %d\n"), GetLastError());
                    return -1;
                }
                clockticks = startClock();
                WaitForSingleObject(dados->timer, INFINITE);
                stopClock(clockticks);
                _tprintf(_T("\n[TIMER] Após %d segundos a água foi retomada de %d em %d segundos.\n"), dados->msgMemPart->seg_stopagua,dados->msgMemPart->t_agua, dados->msgMemPart->t_agua);
                dados->msgMemPart->seg_stopagua = 0;
                SetEvent(dados->evento);
                Sleep(500);
                //ResetEvent(dados->evento);
            }
        }
        ReleaseSemaphore(dados->sema, 1, NULL);
        ReleaseMutex(dados->mutexTimer);
    } while (dados->termina != 1);
    //Releasemutex
    return 0;
}


DWORD WINAPI ThreadServidorMonitor(LPVOID param) {
    jogo* dados = (jogo*)param;
    int total = 0;
    int posx, posy,jogador;
    BufferCirc buf;
    do {
        if (!_tcscmp(dados->msgMemPart->cmd_server, _T("parar")) == 0 && dados->msgMemPart->joga == 1) {
            //Produtor-Consumidor
            WaitForSingleObject(dados->sem_leitura, INFINITE);
            WaitForSingleObject(dados->sem_mutex_c, INFINITE);
            CopyMemory(&buf, &dados->msgProdCons->buffer[dados->msgProdCons->posL], sizeof(BufferCirc));
            dados->msgProdCons->posL++;
            //Logica buffercircular
            if (dados->msgProdCons->posL == TAMBUFFERCIRC) {
                dados->msgProdCons->posL = 0;
            }
            ReleaseMutex(dados->sem_mutex_c);
            ReleaseSemaphore(dados->sem_escrita, 1, NULL);
            _tcscpy_s(dados->comando, BUFFERSIZE, buf.comando);
            //----------------------------------
            _tprintf(_T("\n[Monitor] Comando: %s\n"), dados->comando);
            if (_tcsncmp(dados->comando, _T("parar"), 5) == 0) { //Parar a agua por x segundos
                TCHAR* aux;
                TCHAR* next_token = NULL;
                aux = _tcstok_s(dados->comando, _T(" "), &next_token);
                aux = _tcstok_s(NULL, _T(" "), &next_token);
                if (_tstoi(aux) > 0) {
                    dados->msgMemPart->seg_stopagua = _tstoi(aux);
                }
                _tprintf(_T("\n[AÇÃO] Água parada por %d segundos.\n"), dados->msgMemPart->seg_stopagua);
            }
            else if (_tcsncmp(dados->comando, _T("inserir"), 7) == 0) {
                TCHAR* aux;
                TCHAR* next_token = NULL;
                aux = _tcstok_s(dados->comando, _T(" "), &next_token);
                aux = _tcstok_s(NULL, _T(" "), &next_token);
                posx = _tstoi(aux);
                aux = _tcstok_s(NULL, _T(" "), &next_token);
                posy = _tstoi(aux);
                aux = _tcstok_s(NULL, _T(" "), &next_token);
                jogador = _tstoi(aux); //Indica em qual mapa do jogador se insere a parede
                if (inserirParede(posx, posy, dados,jogador)) {
                    _tprintf(_T("\n[AÇÃO] Parede inserida com sucesso.\n"));
                }
                else {
                    _tprintf(_T("\n[AÇÃO] Erro ao inserir a parede, tente novamente.\n"));
                }
                //_tprintf(_T("%s - comp:%d larg: %d\n"), aux,posx,posy);
            }
            else if (_tcsncmp(dados->comando, _T("aleatorio"), 9) == 0) {
                if (_tcscmp(dados->comando, _T("aleatorio ativar")) == 0) {
                    _tprintf(_T("\n[AÇÃO] Modo aleatorio ativado.\n"));
                    dados->msgMemPart->aleatorio_flag = 1;
                    randomSimb(dados);
                }
                else if (_tcscmp(dados->comando, _T("aleatorio desativar")) == 0) {
                    _tprintf(_T("\n[AÇÃO] Modo aleatorio desativado.\n"));
                    dados->msgMemPart->aleatorio_flag = 0;
                    preencheSimb(dados);
                }
            }
            else {
                _tprintf(_T("\n[AÇÃO] Comando não detetado.\n"));
            }
        }
    } while (dados->termina != 1);
    _tprintf(_T("[JOGO] Saí do jogo.\n"));
    return 0;
}

//Servidor



int _tmain(int argc, TCHAR* argv[]) {
    srand(time(NULL));
    jogo jg;
    HANDLE hMutex;
    HANDLE mapping;
    HANDLE mapping2;
    HKEY chave;
    int what = ERROR_SUCCESS;
    TCHAR comandos[BUFFERSIZE];
    //Valores locais
    int comprimento, largura;
    //Iniciar o timer
    initClock();
    DWORD offset, nbytes;
#ifdef UNICODE
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
    _setmode(_fileno(stderr), _O_WTEXT);
#endif

    hMutex = OpenMutex(MUTEX_ALL_ACCESS, 0, _T("servidor"));


    if (!hMutex) {
        // Mutex doesn’t exist. This is the first instance so create the mutex.
        hMutex = CreateMutex(0, 0, _T("servidor"));
    }else{
        // The mutex exists so this is the second instance so return.
        return 0;
    }
    
    //Produtor-Consumidor
    jg.sem_mutex_c = OpenMutex(NULL,FALSE, _T("mutex_consumidor"));
    if (jg.sem_mutex_c == NULL) {
        jg.sem_mutex_c = CreateMutex(NULL, FALSE, _T("mutex_consumidor"));
        if (jg.sem_mutex_c == NULL) {
            _tprintf(_T("Erro a criar o mutex: %d\n"), GetLastError());
            return -1;
        }
    }
    jg.sem_escrita = CreateSemaphore(NULL, TAMBUFFERCIRC, TAMBUFFERCIRC, _T("semaforo_escrita"));
    if (jg.sem_escrita == NULL) {
        _tprintf(_T("Erro a criar o semaforo: %d\n"), GetLastError());
        CloseHandle(jg.sem_mutex_c);
        return -1;
    }
    jg.sem_leitura = CreateSemaphore(NULL, 0, TAMBUFFERCIRC, _T("semaforo_leitura"));
    if (jg.sem_leitura == NULL) {
        _tprintf(_T("Erro a criar o semaforo: %d\n"), GetLastError());
        CloseHandle(jg.sem_mutex_c);
        CloseHandle(jg.sem_escrita);
        return -1;
    }
    mapping = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Memoria), _T("Memoria_Partilhada"));
    if (mapping == NULL) {
        _tprintf(_T("Erro a criar o FileMapping: %d\n"), GetLastError());
        CloseHandle(jg.sem_mutex_c);
        CloseHandle(jg.sem_escrita);
        CloseHandle(jg.sem_leitura);
        return -1;
    }
    jg.msgProdCons = (Memoria*)MapViewOfFile(mapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (jg.msgProdCons == NULL) {
        _tprintf(_T("Erro a criar MapViewOfFile: %d\n"), GetLastError());
        CloseHandle(jg.sem_mutex_c);
        CloseHandle(jg.sem_escrita);
        CloseHandle(jg.sem_leitura);
        UnmapViewOfFile(mapping);
        return -1;
    }
    jg.msgProdCons->nConsumidores = 0;
    jg.msgProdCons->nProdutores = 0;
    jg.msgProdCons->posE = 0;
    jg.msgProdCons->posL = 0;
    // Timer da Agua
    jg.mutexTimer = CreateMutex(NULL, FALSE, _T("mutex_timer"));
    if (jg.mutexTimer == NULL) {
        _tprintf(_T("Erro na criação do mutex %d\n"), GetLastError());
        CloseHandle(jg.sem_mutex_c);
        CloseHandle(jg.sem_escrita);
        CloseHandle(jg.sem_leitura);
        UnmapViewOfFile(mapping);
        CloseHandle(jg.msgProdCons);
        return -1;
    }
    jg.thTimer = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)AguaTimer,&jg,0,NULL);
    if (jg.thTimer == NULL) {
        _tprintf(_T("Erro na criação da thread %d\n"), GetLastError());
        CloseHandle(jg.sem_mutex_c);
        CloseHandle(jg.sem_escrita);
        CloseHandle(jg.sem_leitura);
        UnmapViewOfFile(mapping);
        CloseHandle(jg.msgProdCons);
        CloseHandle(jg.mutexTimer);
        return -1;
    }
    jg.sema = CreateSemaphore(NULL, 1, 1, _T("semaforo_timer"));
    if (jg.sema == NULL) {
        _tprintf(_T("Erro na criação do semaforo %d\n"), GetLastError());
        CloseHandle(jg.sem_mutex_c);
        CloseHandle(jg.sem_escrita);
        CloseHandle(jg.sem_leitura);
        UnmapViewOfFile(mapping);
        CloseHandle(jg.msgProdCons);
        CloseHandle(jg.mutexTimer);
        CloseHandle(jg.thTimer);
        return -1;
    }
    jg.timer = CreateWaitableTimer(NULL, TRUE, NULL); //Cada processo tem o seu tiimer
    if (jg.timer == NULL) {
        _tprintf(_T("Erro na criação do timer %d\n"), GetLastError());
        CloseHandle(jg.sem_mutex_c);
        CloseHandle(jg.sem_escrita);
        CloseHandle(jg.sem_leitura);
        UnmapViewOfFile(mapping);
        CloseHandle(jg.msgProdCons);
        CloseHandle(jg.mutexTimer);
        CloseHandle(jg.thTimer);
        CloseHandle(jg.sema);
        return -1;
    }
    mapping2 = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(MsgStruct), _T("mapeamento"));
    if (mapping == NULL) {
        _tprintf(_T("Erro ao criar o objeto de mapeamento: %d\n"), GetLastError());
        CloseHandle(jg.sem_mutex_c);
        CloseHandle(jg.sem_escrita);
        CloseHandle(jg.sem_leitura);
        UnmapViewOfFile(mapping);
        CloseHandle(jg.msgProdCons);
        CloseHandle(jg.mutexTimer);
        CloseHandle(jg.thTimer);
        CloseHandle(jg.sema);
        CloseHandle(jg.timer);
        return -1;
    }
    jg.msgMemPart = (MsgStruct*)MapViewOfFile(mapping2, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(MsgStruct));
    if (jg.msgMemPart == NULL) {
        _tprintf(TEXT("<Control> Erro a criar vista para a memória partilhada (%d)\n"), GetLastError());
        CloseHandle(jg.sem_mutex_c);
        CloseHandle(jg.sem_escrita);
        CloseHandle(jg.sem_leitura);
        UnmapViewOfFile(mapping2);
        UnmapViewOfFile(mapping);
        CloseHandle(jg.msgProdCons);
        CloseHandle(jg.mutexTimer);
        CloseHandle(jg.thTimer);
        CloseHandle(jg.sema);
        CloseHandle(jg.timer);
        //UnmapViewOfFile(mapping);
        return 1;
    }
    jg.evento = CreateEvent(NULL, FALSE, FALSE, _T("Evento"));
    
    //Servidor Control - Comandos
    jg.thServerCtrl = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadServerControl, &jg, 0, NULL);
    if (jg.thServerCtrl == NULL) {
        _tprintf(_T("Erro na criação da thread %d\n"), GetLastError());
        CloseHandle(jg.sem_mutex_c);
        CloseHandle(jg.sem_escrita);
        CloseHandle(jg.sem_leitura);
        CloseHandle(jg.msgProdCons);
        CloseHandle(jg.mutexTimer);
        CloseHandle(jg.thTimer);
        CloseHandle(jg.sema);
        CloseHandle(jg.timer);
        UnmapViewOfFile(mapping);
        CloseHandle(jg.msgMemPart);
        //CloseHandle(jg.thEscreveMemoria);
        //CloseHandle(jg.mutexMem);
        return -1;
    }
    jg.mutexSvCtrl = CreateMutex(NULL, FALSE, _T("mutex_servidor"));
    if (jg.mutexSvCtrl == NULL) {
        _tprintf(_T("Erro na criação do mutex %d\n"), GetLastError());
        CloseHandle(jg.sem_mutex_c);
        CloseHandle(jg.sem_escrita);
        CloseHandle(jg.sem_leitura);
        CloseHandle(jg.msgProdCons);
        CloseHandle(jg.mutexTimer);
        CloseHandle(jg.thTimer);
        CloseHandle(jg.sema);
        CloseHandle(jg.timer);
        UnmapViewOfFile(mapping);
        CloseHandle(jg.msgMemPart);
        return -1;
    }
    
	//dimensões do mapa de jogo e o tempo que a água demora a fluir passado na linha de comandos
	if (argc == 3) {
        //Criar/Abrir chave do Registry
        RegCreateKeyEx(HKEY_CURRENT_USER, TEXT("SOFTWARE\\TPSO2"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &chave, &what);
        //_tprintf(_T("Recebi dados atraves da linha de comandos.\n"));
        TCHAR* aux;
        TCHAR* next_token = NULL;
        aux = _tcstok_s(argv[1],_T("x"),&next_token);
        jg.msgMemPart->comp = _tstoi(aux);
        RegSetValueEx(chave, TEXT("Comp"), 0, REG_SZ, (LPBYTE)aux, (wcslen(aux) + 1) * sizeof(TCHAR));
        aux = _tcstok_s(NULL, _T("x"), &next_token);
        jg.msgMemPart->larg = _tstoi(aux);
        RegSetValueEx(chave, TEXT("Larg"), 0, REG_SZ, (LPBYTE)aux, (wcslen(aux) + 1) * sizeof(TCHAR));
        //Verificar se o comprimento ou largura não é superior a 20x20
        if(jg.msgMemPart->comp > 20 || jg.msgMemPart->larg > 20){
            do {
                _tprintf(_T("O comprimento ou a largura não pode ser superior a 20. Introduza novos dados no formato compXlarg:\n"));
                _fgetts(comandos, BUFFERSIZE, stdin);
                aux = _tcstok_s(comandos, _T("x"), &next_token);
                jg.msgMemPart->comp = _tstoi(aux);
                RegSetValueEx(chave, TEXT("Comp"), 0, REG_SZ, (LPBYTE)aux, (wcslen(aux) + 1) * sizeof(TCHAR));
                aux = _tcstok_s(NULL, _T("x"), &next_token);
                jg.msgMemPart->larg = _tstoi(aux);
                RegSetValueEx(chave, TEXT("Larg"), 0, REG_SZ, (LPBYTE)aux, (wcslen(aux) + 1) * sizeof(TCHAR));
            } while (jg.msgMemPart->comp > 20 || jg.msgMemPart->larg > 20);
        }
        jg.msgMemPart->t_agua = _tstoi(argv[2]);
        RegSetValueEx(chave, TEXT("Time"), 0, REG_SZ, (LPBYTE)argv[2], (wcslen(argv[2]) + 1) * sizeof(TCHAR));
        /*jg.msgMemPart->pontoinicialX = 0;
        jg.msgMemPart->pontoinicialY = rand() % (jg.msgMemPart->larg - 1);
        jg.msgMemPart->destinoaguaX = jg.msgMemPart->comp - 1;
        jg.msgMemPart->destinoaguaY = rand() % (jg.msgMemPart->larg - 1);
        jg.msgMemPart->p_atual_agX = 0;
        jg.msgMemPart->p_atual_agY = jg.msgMemPart->pontoinicialY;
        jg.msgMemPart->p_anterior_agY = jg.msgMemPart->p_atual_agY;
        jg.msgMemPart->p_anterior_agX = jg.msgMemPart->p_atual_agX;*/
	}
	else {
		//Consultar o registry
        if (RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("SOFTWARE\\TPSO2"), 0, KEY_READ, &chave) == ERROR_SUCCESS) {
            TCHAR buffer[MAX];
            int tamanho = sizeof(buffer);
            if (RegQueryValueEx(chave, TEXT("Comp"), NULL, NULL, buffer, &tamanho) == ERROR_SUCCESS) {
                jg.msgMemPart->comp = _tstoi(buffer);
            }
            if(RegQueryValueEx(chave, TEXT("Larg"), NULL, NULL, buffer, &tamanho) == ERROR_SUCCESS){
                jg.msgMemPart->larg = _tstoi(buffer);
            }
            if (RegQueryValueEx(chave, TEXT("Time"), NULL, NULL, buffer, &tamanho) == ERROR_SUCCESS) {
                jg.msgMemPart->t_agua = _tstoi(buffer);
            }
            //_tprintf(_T("Comp: %d Larg: %d T_Agua: %d \n"), j.comp, j.larg, j.t_agua);
            /*jg.msgMemPart->pontoinicialX = 0;
            jg.msgMemPart->pontoinicialY = rand() % (jg.msgMemPart->larg - 1);
            jg.msgMemPart->destinoaguaX = jg.msgMemPart->comp - 1;
            jg.msgMemPart->destinoaguaY = rand() % (jg.msgMemPart->larg - 1); 
            jg.msgMemPart->p_atual_agX = 0;
            jg.msgMemPart->p_atual_agY = jg.msgMemPart->pontoinicialY;
            jg.msgMemPart->p_anterior_agY = jg.msgMemPart->p_atual_agY;
            jg.msgMemPart->p_anterior_agX = jg.msgMemPart->p_atual_agX;*/
            _tprintf(_T("Chave no Registry lida com sucesso.\n"));
        }
        else {
            _tprintf(_T("Erro a ler a chave.\n"));
            return -1;
        }
	}

    //Mapa Default - Meta1
    for (int i = 0; i < 2; i++) {
        jg.msgMemPart->pontoinicialX[i] = 0;
        jg.msgMemPart->pontoinicialY[i] = 0;
        jg.msgMemPart->destinoaguaX[i] = jg.msgMemPart->comp - 1;
        jg.msgMemPart->destinoaguaY[i] = jg.msgMemPart->larg - 1;
        jg.msgMemPart->p_atual_agX[i] = 0;
        jg.msgMemPart->p_atual_agY[i] = 0;
        jg.msgMemPart->p_anterior_agY[i] = 0;
        jg.msgMemPart->p_anterior_agX[i] = 0;
    }
    //Guardar nos valores locais
    comprimento = jg.msgMemPart->comp;
    largura = jg.msgMemPart->larg;
    jg.tab[0] = 0;
    jg.tab[1] = 0;
    //Preencher simbolos por ordem predefinida
    preencheSimb(&jg);
    preencheMapa(&jg,0);
    carregaMapaDefault(&jg);
    //---------------------------------------

    jg.termina = 0;
    jg.msgMemPart->joga = FALSE;

    WaitForSingleObject(jg.sem_mutex_c, INFINITE);
    jg.msgProdCons->nConsumidores++;
    ReleaseMutex(jg.sem_mutex_c);

    //Thread ServidorMonitor
    jg.hThreadServidorMonitor = CreateThread(NULL, 0, ThreadServidorMonitor, &jg, 0, NULL);
    if (jg.hThreadServidorMonitor == NULL) {
        _tprintf(_T("Erro a criar a thread.\n"));
        CloseHandle(jg.sem_mutex_c);
        CloseHandle(jg.sem_escrita);
        CloseHandle(jg.sem_leitura);
        CloseHandle(jg.msgProdCons);
        CloseHandle(jg.mutexTimer);
        CloseHandle(jg.thTimer);
        CloseHandle(jg.sema);
        CloseHandle(jg.timer);
        UnmapViewOfFile(mapping);
        CloseHandle(jg.msgMemPart);
        CloseHandle(jg.mutexSvCtrl);
        return -1;
    }
    
    //Iniciar namedpipes
    //Preparar os namedpipes
    jg.npDados.hMutex = CreateMutex(NULL, FALSE, NULL);
    if (jg.npDados.hMutex == NULL) {
        CloseHandle(jg.sem_mutex_c);
        CloseHandle(jg.sem_escrita);
        CloseHandle(jg.sem_leitura);
        CloseHandle(jg.msgProdCons);
        CloseHandle(jg.mutexTimer);
        CloseHandle(jg.thTimer);
        CloseHandle(jg.sema);
        CloseHandle(jg.timer);
        UnmapViewOfFile(mapping);
        CloseHandle(jg.msgMemPart);
        CloseHandle(jg.mutexSvCtrl);
        CloseHandle(jg.hThreadServidorMonitor);
    }
    jg.total_jog = 0;

    for (int i = 0; i < 2; i++) {
        jg.pipeLocal = CreateNamedPipe(PIPE_NAME,
            PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
            PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE,
            2,
            sizeof(jogador),
            sizeof(jogador), 1000, NULL);

        if (jg.pipeLocal == NULL) {
            CloseHandle(jg.sem_mutex_c);
            CloseHandle(jg.sem_escrita);
            CloseHandle(jg.sem_leitura);
            CloseHandle(jg.msgProdCons);
            CloseHandle(jg.mutexTimer);
            CloseHandle(jg.thTimer);
            CloseHandle(jg.sema);
            CloseHandle(jg.timer);
            UnmapViewOfFile(mapping);
            CloseHandle(jg.msgMemPart);
            CloseHandle(jg.mutexSvCtrl);
            CloseHandle(jg.npDados.hMutex);
            CloseHandle(jg.hThreadServidorMonitor);
            return -1;
        }

        jg.evento_pipe = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (jg.evento_pipe == NULL) {
            _tprintf(TEXT("[ERRO] Criar Evento\n"));
            CloseHandle(jg.sem_mutex_c);
            CloseHandle(jg.sem_escrita);
            CloseHandle(jg.sem_leitura);
            CloseHandle(jg.msgProdCons);
            CloseHandle(jg.mutexTimer);
            CloseHandle(jg.thTimer);
            CloseHandle(jg.sema);
            CloseHandle(jg.timer);
            UnmapViewOfFile(mapping);
            CloseHandle(jg.msgMemPart);
            CloseHandle(jg.mutexSvCtrl);
            CloseHandle(jg.npDados.hMutex);
            CloseHandle(jg.hThreadServidorMonitor);
            CloseHandle(jg.pipeLocal);
            return -1;
        }

        jg.npDados.hPipe[i].hPipe = jg.pipeLocal;
        jg.npDados.hPipe[i].ligado = FALSE;
        
        ZeroMemory(&jg.npDados.hPipe[i].ov, sizeof(jg.npDados.hPipe[i].ov));
        jg.npDados.hPipe[i].ov.hEvent = jg.evento_pipe;
        jg.npDados.evento[i] = jg.evento_pipe;

        ConnectNamedPipe(jg.pipeLocal, &jg.npDados.hPipe[i].ov);
    }
    
    while (jg.termina == 0 && jg.msgMemPart->joga == FALSE) {
        offset = WaitForMultipleObjects(2, jg.npDados.evento, FALSE, INFINITE);
        int i = offset - WAIT_OBJECT_0;
        if (i >= 0 && jg.termina != 1 && jg.msgMemPart->joga == FALSE) {
            _tprintf(_T("Novo cliente ligado.\n"));
            if (GetOverlappedResult(jg.npDados.hPipe[i].hPipe, &jg.npDados.hPipe[i].ov, &nbytes, FALSE)) {
                ResetEvent(jg.npDados.evento[i]);

                WaitForSingleObject(jg.npDados.hMutex, INFINITE);
                jg.npDados.hPipe[i].ligado = TRUE;
                ReleaseMutex(jg.npDados.hMutex);
                jg.total_jog++;
                jg.msgMemPart->comp = comprimento;
                jg.msgMemPart->larg = largura;
                //Preenche mapa ao novo jogador
                preencheMapa(&jg, jg.total_jog);
                if (jg.total_jog == 1) { //Se for o primeiro jogador cria as threads
                    jg.hThreadComunicacaoNP = CreateThread(NULL, 0, ThreadComunicacaoNP, &jg, 0, NULL);
                    if (jg.hThreadComunicacaoNP == NULL) {
                        CloseHandle(jg.sem_mutex_c);
                        CloseHandle(jg.sem_escrita);
                        CloseHandle(jg.sem_leitura);
                        CloseHandle(jg.msgProdCons);
                        CloseHandle(jg.mutexTimer);
                        CloseHandle(jg.thTimer);
                        CloseHandle(jg.sema);
                        CloseHandle(jg.timer);
                        UnmapViewOfFile(mapping);
                        CloseHandle(jg.msgMemPart);
                        CloseHandle(jg.mutexSvCtrl);
                        CloseHandle(jg.npDados.hMutex);
                        CloseHandle(jg.hThreadServidorMonitor);
                        CloseHandle(jg.pipeLocal);
                        return -1;
                    }
                    jg.mutexJogada = CreateMutex(NULL, FALSE, _T("mutex_jogada"));
                    if (jg.mutexJogada == NULL) {
                        CloseHandle(jg.sem_mutex_c);
                        CloseHandle(jg.sem_escrita);
                        CloseHandle(jg.sem_leitura);
                        CloseHandle(jg.msgProdCons);
                        CloseHandle(jg.mutexTimer);
                        CloseHandle(jg.thTimer);
                        CloseHandle(jg.sema);
                        CloseHandle(jg.timer);
                        UnmapViewOfFile(mapping);
                        CloseHandle(jg.msgMemPart);
                        CloseHandle(jg.mutexSvCtrl);
                        CloseHandle(jg.npDados.hMutex);
                        CloseHandle(jg.hThreadServidorMonitor);
                        CloseHandle(jg.pipeLocal);
                        CloseHandle(jg.hThreadComunicacaoNP);
                        return -1;
                    }
                    
                }
            }
        }
    }

    
    Sleep(5000);
    WaitForSingleObject(jg.hThreadServidorMonitor, INFINITE);
    

    //Fechar handles & pipes
    for (int i = 0; i < 2; i++) {
        if (!DisconnectNamedPipe(jg.npDados.hPipe[i].hPipe)) {
            _tprintf(TEXT("[ERRO] Desligar o pipe! (DisconnectNamedPipe)"));
            return -1;
        }

        CloseHandle(jg.npDados.hPipe[i].hPipe);
    }
    CloseHandle(jg.hThreadComunicacaoNP);
    CloseHandle(jg.mutexJogada);
    CloseHandle(jg.npDados.hMutex);
    CloseHandle(jg.pipeLocal);

    //Fechar Handles+
    CloseHandle(jg.sem_mutex_c);
    CloseHandle(jg.sem_escrita);
    CloseHandle(jg.sem_leitura);
    CloseHandle(jg.thTimer);
    CloseHandle(jg.sema);
    CloseHandle(jg.mutexTimer);
    CloseHandle(jg.mutexSvCtrl);
    CloseHandle(jg.thServerCtrl);
    CloseHandle(jg.hThreadServidorMonitor);
    ReleaseMutex(hMutex);
    UnmapViewOfFile(mapping);
    UnmapViewOfFile(mapping2);
    return 0;
}