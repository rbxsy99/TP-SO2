#include <io.h>
#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
#include <fcntl.h>
#include "monitor.h"
#include "../utils.h"

void mostraMapa(jogo* j) {
    WaitForSingleObject(j->mutexMapa, INFINITE);
    _tprintf(_T("\n-Mapa do jogo (Jogador 1)-\n"));
    for (int i = 0; i < j->msgMemPart->comp; i++) {
        for (int x = 0; x < j->msgMemPart->larg; x++) {
            _tprintf(_T(" %c "), j->msgMemPart->mapa_jogo[i][x][0]);
        }
        _tprintf(_T("\n"));
    }
    _tprintf(_T("\n-Mapa do jogo (Jogador 2)-\n"));
    for (int i = 0; i < j->msgMemPart->comp; i++) {
        for (int x = 0; x < j->msgMemPart->larg; x++) {
            _tprintf(_T(" %c "), j->msgMemPart->mapa_jogo[i][x][1]);
        }
        _tprintf(_T("\n"));
    }
    _tprintf(_T("\n-Simbolos-\n"));
    _tprintf(_T("\n[ %c | %c | %c | %c | %c | %c ]\n\n"), j->msgMemPart->simb[0], j->msgMemPart->simb[1], j->msgMemPart->simb[2], j->msgMemPart->simb[3], j->msgMemPart->simb[4], j->msgMemPart->simb[5]);
    _tprintf(_T("-Legenda-\n\nA -> Água \t X -> Parede intransponíveis \t I -> Início \t F -> Fim \n\n"));
    ReleaseMutex(j->mutexMapa);
}

DWORD WINAPI ThreadLeMemoria(LPVOID param) {
    jogo* p = (jogo*)param;
    while (p->termina != 1) {
        WaitForSingleObject(p->mutexMem, INFINITE);
        WaitForSingleObject(p->evento, INFINITE);
        if (p->msgMemPart->joga) {
            mostraMapa(p);
        }
        //ResetEvent(p->evento);
        Sleep(p->msgMemPart->t_agua * 1000);
        ReleaseMutex(p->mutexMem);
    }
    return 0;
}


DWORD WINAPI ThreadMonitorServidor(LPVOID param) {
    jogo* p = (jogo*)param;
    TCHAR comando[BUFFERSIZE];
    BufferCirc buf;
    _tprintf(_T("Comandos:\n parar x -> Para água durante x segundos \n inserir x y j-> Inserir paredes intransponíveis no mapa (x -> linha & y-> coluna & j ->jogador(1/2))\n "));
    _tprintf(_T("aleatorio ativar/desativar -> Ativar ou desativar o modo aleatório para a sequeência de peças/tubos a mostrar\n "));
    p->msgMemPart->joga = 0;
    do{
        if (!_tcscmp(p->msgMemPart->cmd_server, _T("parar")) == 0 && p->msgMemPart->joga) {
            //_tprintf(_T("Comp:%d"), p->msgMemPart->comp);
            _tprintf(_T("Comando:"));
            _fgetts(comando, BUFFERSIZE, stdin); //Bloqueante 
            comando[_tcslen(comando) - 1] = '\0'; //strlen - wcslen
            //Copiar o comando para o buffer
            _tcscpy_s(buf.comando, BUFFERSIZE, comando);
            //Produtor-Consumidor //ex 3 f6
            WaitForSingleObject(p->sem_escrita, INFINITE);
            WaitForSingleObject(p->sem_mutex_p, INFINITE);
            CopyMemory(&p->msgProdCons->buffer[p->msgProdCons->posE], &buf, sizeof(BufferCirc));
            p->msgProdCons->posE++;
            //Logica Buffer Circular
            if (p->msgProdCons->posE == TAMBUFFERCIRC) {
                p->msgProdCons->posE = 0;
            }
            ReleaseMutex(p->sem_mutex_p);
            ReleaseSemaphore(p->sem_leitura, 1, NULL);
            Sleep(1000);
        }
    } while ((_tcscmp(p->msgMemPart->cmd_server, _T("terminar")) != 0));
    _tprintf(_T("Jogo terminado pelo servidor.\n"));
    p->termina = 1;
    return 0;
}



int _tmain(int argc, TCHAR* argv[]) {
    jogo jg;
    HANDLE mapping;
    HANDLE mapping2;
	
#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif
	
    //Verificar se o servidor está a funcionar
    if (!OpenMutex(MUTEX_ALL_ACCESS, 0, _T("servidor"))) {
        return 0;
    }

    //Produtor-Consumidor
    jg.sem_mutex_p = OpenMutex(NULL, FALSE, _T("mutex_produtor"));
    if (jg.sem_mutex_p == NULL) {
        jg.sem_mutex_p = CreateMutex(NULL, FALSE, _T("mutex_produtor"));
        if (jg.sem_mutex_p == NULL) {
            _tprintf(_T("Erro a criar o mutex: %d\n"), GetLastError());
            
            return -1;
        }
    }
    jg.sem_escrita = CreateSemaphore(NULL, TAMBUFFERCIRC, TAMBUFFERCIRC, _T("semaforo_escrita"));
    if (jg.sem_escrita == NULL) {
        _tprintf(_T("Erro a criar o semaforo: %d\n"), GetLastError());
        CloseHandle(jg.sem_mutex_p);
        return -1;
    }
    jg.sem_leitura = CreateSemaphore(NULL, 0, TAMBUFFERCIRC, _T("semaforo_leitura"));
    if (jg.sem_leitura == NULL) {
        _tprintf(_T("Erro a criar o semaforo: %d\n"), GetLastError());
        CloseHandle(jg.sem_mutex_p);
        CloseHandle(jg.sem_escrita);
        return -1;
    }
    mapping = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, _T("Memoria_Partilhada"));
    if (mapping == NULL) {
        _tprintf(_T("Erro a criar o FileMapping: %d\n"), GetLastError());
        CloseHandle(jg.sem_mutex_p);
        CloseHandle(jg.sem_escrita);
        CloseHandle(jg.sem_leitura);
        return -1;
    }
    jg.msgProdCons = (Memoria*)MapViewOfFile(mapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (jg.msgProdCons == NULL) {
        _tprintf(_T("Erro a criar MapViewOfFile: %d\n"), GetLastError());
        CloseHandle(jg.sem_mutex_p);
        CloseHandle(jg.sem_escrita);
        CloseHandle(jg.sem_leitura);
        UnmapViewOfFile(mapping);
        return -1;
    }
    jg.msgProdCons->nConsumidores = 0;
    jg.msgProdCons->nProdutores = 0;
    jg.msgProdCons->posE = 0;
    jg.msgProdCons->posL = 0;

    //---------Monitor - Memoria Partilhada
    mapping2 = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, _T("mapeamento"));
    if (mapping2 == NULL) {
        _tprintf(_T("Erro a criar o FileMapping: %d\n"), GetLastError());
        CloseHandle(jg.sem_mutex_p);
        CloseHandle(jg.sem_escrita);
        CloseHandle(jg.sem_leitura);
        return -1;
    }
    jg.msgMemPart = (MsgStruct*)MapViewOfFile(mapping2, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(MsgStruct));
    if (jg.msgMemPart == NULL) {
        _tprintf(TEXT("<Control> Erro a criar vista para a memória partilhada (%d)\n"), GetLastError());
        return 1;
    }
    jg.thLeMemoria = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadLeMemoria, &jg, 0, NULL);
    if (jg.thLeMemoria == NULL) {
        _tprintf(_T("Erro na criação da thread %d\n"), GetLastError());
        return -1;
    }
    jg.mutexMem = CreateMutex(NULL, FALSE, _T("mutex_memoria"));
    if (jg.mutexMem == NULL) {
        _tprintf(_T("Erro na criação do mutex %d\n"), GetLastError());
        return -1;
    }

    //Mutex mapa
    jg.mutexMapa = CreateMutex(NULL, FALSE, _T("mutex_mapa"));
    if (jg.mutexMapa == NULL) {
        _tprintf(_T("Erro na criação do mutex %d\n"), GetLastError());
        return -1;
    }

    WaitForSingleObject(jg.sem_mutex_p, INFINITE);
    jg.msgProdCons->nProdutores++;
    ReleaseMutex(jg.sem_mutex_p);

    //Thread MonitorServidor
    jg.hThreadMonServ = CreateThread(NULL, 0, ThreadMonitorServidor, &jg, 0, NULL);
    if (jg.hThreadMonServ == NULL) {
        _tprintf(_T("Erro a criar a thread.\n"));
        return -1;
    }

    do {
        WaitForSingleObject(jg.hThreadMonServ, INFINITE);
    }while (jg.termina != 1);


    //Fechar Handles+
    CloseHandle(jg.sem_mutex_p);
    CloseHandle(jg.sem_escrita);
    CloseHandle(jg.sem_leitura);
    CloseHandle(jg.thLeMemoria);
    CloseHandle(jg.mutexMem);
    CloseHandle(jg.hThreadMonServ);
    UnmapViewOfFile(mapping);
    UnmapViewOfFile(mapping2);
    return 0;

}