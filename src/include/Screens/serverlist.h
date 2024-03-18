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
void StartServerCommunication(char* server, char* port);



void AddStatusText(char* text);
void UpdateStatusText(void);
static void OnButtonPressCancel(Button *button);
static void OnButtonPressJoin(Button *button);
static void OnButtonPressRefresh(Button *button);

typedef struct serverData_t
{
    char *pszServerIP;
    char *pszServerPort;
}ServerData;

#endif // SERVERLIST_H