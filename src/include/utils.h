#ifndef UTILS_H
#define UTILS_H

// Windows headers
#include <winsock2.h>
#include <ws2tcpip.h>
#include <Windows.h>
#include <shellscalingapi.h> //used to get DPI scaling
#include <winerror.h>
#include <wingdi.h>
#include <time.h>
#include <math.h>

// raylib headers
#include "raylib.h"
#include "raymath.h"

// C headers
#include <stdint.h>
#include <stdlib.h>
#include "string.h"

// Program headers
#include "constants.h"
#include "types.h"
#include <GLFW/glfw3.h>

// SOUND STUFF
void PlaySoundResource(const char *szName);
void PlayRandomIntroSound(void);

// TEXTURE STUFF
void LoadBackgroundImages(void);
void LoadTextureFromResource(Texture *pTexture, const char *name, bool bIsPNG);
void Loader_InitializeBackgroundTextures();
void DrawTextureTiled(Texture2D texture, Rect source, Rect dest, Vector2 origin, float rotation, float scale, Color tint);

// DPI SCALING STUFF
typedef BOOL(WINAPI *SETPROCESSDPIAWARE_T)(void);
typedef HRESULT(WINAPI *SETPROCESSDPIAWARENESS_T)(PROCESS_DPI_AWARENESS);
bool SetProcessDpiAware(void);

// TEXT HELPERS
char *FormatStringWithNewLines(const char *szString, Rect rTextDrawArea);

// GUI STUFF
void DragWindow(void);
void UnloadButton(Button *button);
void UnloadCheckBox(Checkbox *checkbox);

// CONFIG STUFF
void AddToList(AutoexecCfg *list, int nType, char *szKey, char *szValue);
void FreeList(AutoexecCfg *list);
AutoexecCfg *FindKeyInList(AutoexecCfg *list, char *szSearchKey);
void UpdateKeyInList(AutoexecCfg *list, char *szSearchKey, char *szNewValue);
void LoadAutoexecCfg(AutoexecCfg *config);
void SaveConfig(AutoexecCfg *list);

// LAUNCH STUFF
bool Launch(DWORD nExecutable);

//SERVERLIST STUFF - NETWORKING


#endif // UTILS_H