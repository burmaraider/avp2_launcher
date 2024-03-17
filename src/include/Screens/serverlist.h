#ifndef SERVERLIST_H
#define SERVERLIST_H

#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "..\types.h"
#include "..\constants.h"
#include "..\utils.h"

void ServerlistSetupScreen(void *pRenderLoop, void *pUpdateLoop);
void ServerlistRenderScreen(void);
void ServerlistUpdateLoop(void);
void ServerlistUnloadScreen(void);


DWORD WINAPI ServerCommunicationThread(LPVOID lpParam);
void StartServerCommunication(const char* server, const char* port);

typedef struct serverData_t
{
    char *pszServerIP;
    char *pszServerPort;
}ServerData;

#endif // SERVERLIST_H