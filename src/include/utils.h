#ifndef LOADER_H
#define LOADER_H

// Windows headers
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

#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))
typedef BOOL(WINAPI *SETPROCESSDPIAWARE_T)(void);
typedef HRESULT(WINAPI *SETPROCESSDPIAWARENESS_T)(PROCESS_DPI_AWARENESS);

static void PlaySoundResource(const char *szName)
{
    HMODULE hInst = GetModuleHandle(NULL);
    HRSRC hResSource = FindResourceA(hInst, szName, "WAVE");

    if (hResSource == NULL)
    {
        // handle error
        return;
    }

    HGLOBAL hRes = LoadResource(hInst, hResSource);
    if (hRes == NULL)
    {
        // handle error
        return;
    }

    // get size of resource
    DWORD dwSize = SizeofResource(hInst, hResSource);

    LPSTR lpRes = LockResource(hRes);
    if (lpRes == NULL)
    {
        // handle error
        FreeResource(hRes);
        return;
    }

    // Use raylib to play the sound, since it has easier volume controls
    Wave wave = {0};
    wave = LoadWaveFromMemory(".wav", lpRes, dwSize);
    Sound sound = LoadSoundFromWave(wave);
    SetSoundVolume(sound, 0.3f);
    PlayAudioStream(sound.stream);

    UnlockResource(hRes);
    FreeResource(hRes);

    UnloadWave(wave);
}

static void PlayRandomIntroSound()
{
    InitAudioDevice();

    srand(time(NULL));
    int random = rand() % AVP2_LAUNCHER_INTRO_COUNT;

    PlaySoundResource(AVP2_LAUNCHER_INTRO[random]);
}

static void LoadBackgroundImages()
{
    HMODULE hInst = GetModuleHandle(NULL);

    for (size_t i = 0; i < AVP2_LAUNCHER_IMAGES_COUNT; i++)
    {
        HRSRC hRes = FindResourceA(hInst, AVP2_LAUNCHER_IMAGES[i], RT_BITMAP);
        HGLOBAL hGlobal = LoadResource(hInst, hRes);
        void *pResource = LockResource(hGlobal);
        int nFileSizeLessHeader = SizeofResource(NULL, hRes);

        // construct a bitmap header since it gets stripped from the resource
        BITMAPFILEHEADER bitmapFileHeader = {0};
        bitmapFileHeader.bfType = 0x4D42; // BM
        bitmapFileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + nFileSizeLessHeader;
        bitmapFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

        void *pData = calloc(1, nFileSizeLessHeader + sizeof(BITMAPFILEHEADER));

        if (!pData)
            continue;

        memcpy(pData, &bitmapFileHeader, sizeof(BITMAPFILEHEADER));
        memcpy((char *)pData + sizeof(BITMAPFILEHEADER), pResource, nFileSizeLessHeader);

        // convert bitmap to image
        Image iImage = LoadImageFromMemory(".bmp", pData, nFileSizeLessHeader + sizeof(BITMAPFILEHEADER));

        g_backgroundImage[i] = LoadTextureFromImage(iImage);
        UnloadImage(iImage);
        UnlockResource(hGlobal);
        FreeResource(hGlobal);
        free(pData);
    }
}

static void LoadTextureFromResource(Texture *pTexture, const char *name)
{
    HMODULE hInst = GetModuleHandle(NULL);

    HRSRC hRes = FindResource(hInst, name, RT_BITMAP);
    HGLOBAL hGlobal = LoadResource(hInst, hRes);
    void *pResource = LockResource(hGlobal);
    int nFileSizeLessHeader = SizeofResource(NULL, hRes);

    // construct a bitmap header since it gets stripped from the resource
    BITMAPFILEHEADER bitmapFileHeader = {0};
    bitmapFileHeader.bfType = 0x4D42; // BM
    bitmapFileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + nFileSizeLessHeader;
    bitmapFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    void *pData = calloc(1, nFileSizeLessHeader + sizeof(BITMAPFILEHEADER));

    if (!pData)
        return;

    memcpy(pData, &bitmapFileHeader, sizeof(BITMAPFILEHEADER));
    memcpy((char *)pData + sizeof(BITMAPFILEHEADER), pResource, nFileSizeLessHeader);

    // convert bitmap to image
    Image iImage = LoadImageFromMemory(".bmp", pData, nFileSizeLessHeader + sizeof(BITMAPFILEHEADER));

    *pTexture = LoadTextureFromImage(iImage);
    UnloadImage(iImage);
    UnlockResource(hGlobal);
    FreeResource(hGlobal);
    free(pData);
}

static void Loader_InitializeBackgroundTextures()
{
    // Set the window icon
    HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APPICON));
    HWND hwnd = GetWindowHandle();

    SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
    SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);

    g_font = LoadFontEx("C:\\Windows\\Fonts\\segoeui.ttf", 18, 0, 250);

    SetTextureFilter(g_font.texture, TEXTURE_FILTER_POINT); // Texture scale filter to use

    LoadBackgroundImages();
}

static bool SetProcessDpiAware(void)
{
    HMODULE shcore = LoadLibraryA("Shcore.dll");
    SETPROCESSDPIAWARENESS_T SetProcessDpiAwareness = NULL;
    if (shcore)
    {
        SetProcessDpiAwareness = (SETPROCESSDPIAWARENESS_T)GetProcAddress(shcore, "SetProcessDpiAwareness");
    }
    HMODULE user32 = LoadLibraryA("User32.dll");
    SETPROCESSDPIAWARE_T SetProcessDPIAware = NULL;
    if (user32)
    {
        SetProcessDPIAware = (SETPROCESSDPIAWARE_T)GetProcAddress(user32, "SetProcessDPIAware");
    }

    bool ret = false;
    if (SetProcessDpiAwareness)
    {
        ret = SetProcessDpiAwareness(PROCESS_SYSTEM_DPI_AWARE) == S_OK;
    }
    else if (SetProcessDPIAware)
    {
        ret = SetProcessDPIAware() != 0;
    }

    if (user32)
    {
        FreeLibrary(user32);
    }
    if (shcore)
    {
        FreeLibrary(shcore);
    }
    return ret;
}

static char *FormatStringWithNewLines(const char *szString, Rect rTextDrawArea)
{
    // copy string to buffer
    char *szStringTempBuffer = (char *)malloc(strlen(szString) + 1);
    strcpy(szStringTempBuffer, szString);

    // calculate max chars per line
    int nMaxCharactersPerLine = rTextDrawArea.width / MeasureTextEx(g_font, "A", 14, 1).x;

    char *words[1024];
    int numWords = 0;
    char *token = strtok(szStringTempBuffer, " ");
    while (token != NULL && numWords < 1024)
    {
        words[numWords++] = token;
        token = strtok(NULL, " ");
    }

    char szBuffer[1024];
    int nChars = 0;
    int nCharsThisLine = 0;
    for (int i = 0; i < numWords; i++)
    {
        int nWordLength = strlen(words[i]);
        if (nCharsThisLine + nWordLength + 1 > nMaxCharactersPerLine)
        {
            szBuffer[nChars++] = '\n';
            nCharsThisLine = 0;
        }
        strcpy(szBuffer + nChars, words[i]);
        nChars += nWordLength;
        szBuffer[nChars++] = ' ';
        nCharsThisLine += nWordLength + 1;
    }

    szBuffer[nChars] = '\0';

    char *szFormattedString = (char *)malloc(strlen(szBuffer) + 1);
    strcpy(szFormattedString, szBuffer);

    free(szStringTempBuffer);

    return szFormattedString;
}

static void FreeModes(Monitor **mon)
{
    if (mon == NULL)
        return;

    for (size_t monitorIndex = 0; monitorIndex < 1; monitorIndex++)
    {
        if (mon[monitorIndex] == NULL)
            continue;

        for (size_t modeIndex = 0; modeIndex < mon[monitorIndex]->modeCount; modeIndex++)
        {
            if (mon[monitorIndex]->modes[modeIndex] == NULL)
                continue;

            TraceLog(LOG_INFO, "MODE: [ID %i] Unloaded Mode", modeIndex);
            free(mon[monitorIndex]->modes[modeIndex]);
            mon[monitorIndex]->modes[modeIndex] = NULL;
        }

        if (mon[monitorIndex]->modes != NULL)
        {
            TraceLog(LOG_INFO, "MONITOR: [ID %i] Freeing modes", monitorIndex);
            free(mon[monitorIndex]->modes);
            mon[monitorIndex]->modes = NULL;
        }

        TraceLog(LOG_INFO, "MONITOR: [ID %i] Freeing monitor", monitorIndex);
        free(mon[monitorIndex]);
        mon[monitorIndex] = NULL;
    }

    free(mon);
    mon = NULL;
}

static void DragWindow()
{
    static Vector2 initialMousePosition = {0, 0};
    static bool bIsDragging = FALSE;

    // Check if user is dragging the window
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && GetMousePosition().y < 16 && GetMousePosition().x < GetScreenWidth() - 40)
    {
        bIsDragging = TRUE;
        initialMousePosition = GetMousePosition();
    }
    else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
    {
        bIsDragging = FALSE;
    }

    // Move the window if user is dragging it
    if (bIsDragging)
    {
        Vector2 mousePosition = GetMousePosition();
        Vector2 windowPosition = GetWindowPosition();
        Vector2 screenPosition = Vector2Add(mousePosition, windowPosition);
        // Set the new window position
        SetWindowPosition((int)screenPosition.x - initialMousePosition.x, (int)screenPosition.y - initialMousePosition.y);
    }
}

static void UnloadButton(Button *button)
{
    for (size_t i = 0; i < 3; i++)
    {
        UnloadTexture(button->texture[i]);
    }
}

static void UnloadCheckBox(Checkbox *checkbox)
{
    for (size_t i = 0; i < 2; i++)
    {
        UnloadTexture(checkbox->texture[i]);
    }
}

static void AddToList(AutoexecCfg *list, int nType, char *szKey, char *szValue)
{
    // check if list is empty
    if (list == NULL)
    {
        // create new list
        list = (AutoexecCfg *)malloc(sizeof(AutoexecCfg));
        list->nType = 0;
        list->pNext = NULL;
        list->szKey = NULL;
        list->szValue = NULL;
    }

    // find end of list
    AutoexecCfg *p = list;
    while (p->pNext != NULL)
    {
        // null check
        if (p->pNext == NULL)
            break;

        // move to next node
        p = p->pNext;
    }

    // create new node
    AutoexecCfg *node = (AutoexecCfg *)malloc(sizeof(AutoexecCfg));

    // set node data
    node->nType = nType;
    node->pNext = NULL;
    node->szKey = (char *)malloc(strlen(szKey) + 1);
    strcpy(node->szKey, szKey);
    node->szValue = (char *)malloc(strlen(szValue) + 1);
    strcpy(node->szValue, szValue);

    // add node to list
    p->pNext = node;
}

static void FreeList(AutoexecCfg *list)
{
    // check if list is empty
    if (list == NULL)
        return;

    // free list
    AutoexecCfg *p = list;
    while (p != NULL)
    {
        AutoexecCfg *pNext = p->pNext;
        free(p->szKey);
        free(p->szValue);
        free(p);
        p = pNext;
    }
}

static AutoexecCfg *FindKeyInList(AutoexecCfg *list, char *szSearchKey)
{
    // check if list is empty
    if (list == NULL)
        return 0;

    // find key in list and return index
    AutoexecCfg *p = list;
    while (p != NULL)
    {
        if (p->szKey == NULL)
        {
            p = p->pNext;
            continue;
        }
        if (strcmp(p->szKey, szSearchKey) == 0)
        {
            return p;
        }
        p = p->pNext;
    }
}

static void UpdateKeyInList(AutoexecCfg *list, char *szSearchKey, char *szNewValue)
{
    // check if list is empty
    if (list == NULL)
        return;

    // find key in list and return index
    AutoexecCfg *p = list;
    while (p != NULL)
    {
        if (strcmp(p->szKey, szSearchKey) == 0)
        {
            // free old value
            free(p->szValue);

            // set new value
            p->szValue = (char *)malloc(strlen(szNewValue) + 1);
            strcpy(p->szValue, szNewValue);
            return;
        }
        p = p->pNext;
    }
}

static void LoadAutoexecCfg(AutoexecCfg *config)
{
    // open autoexec.cfg
    FILE *fp = fopen("autoexec.cfg", "r");
    if (fp == NULL)
    {
        TraceLog(LOG_WARNING, "AUTOEXEC: Failed to open autoexec.cfg");
        return;
    }

    // read autoexec.cfg
    char szLine[128];

    while (fgets(szLine, sizeof(szLine), fp))
    {
        // ignore blank lines
        if (strlen(szLine) == 0)
            continue;
        // check if line starts with quotes
        if (szLine[0] == '"')
        {
            // data looks like this "name" "value"
            char szName[128];
            char szValue[128];

            // get name
            char *p = strchr(szLine + 1, '"');
            if (p == NULL)
            {
                TraceLog(LOG_WARNING, "AUTOEXEC: Failed to parse autoexec.cfg");
                continue;
            }
            strncpy(szName, szLine + 1, p - szLine - 1);
            szName[p - szLine - 1] = '\0';

            // get value
            p = strchr(p + 1, '"');
            if (p == NULL)
            {
                TraceLog(LOG_WARNING, "AUTOEXEC: Failed to parse autoexec.cfg");
                continue;
            }

            strncpy(szValue, p + 1, strchr(p + 1, '"') - p - 1);
            szValue[strchr(p + 1, '"') - p - 1] = '\0';

            // add entry to config array
            AddToList(config, CONFIG_TYPE_STRING, szName, szValue);
        }
        else
        {
            // data looks like this:  name value value2 value3
            char szName[128];
            char szValue[128];

            // get name
            char *p = strchr(szLine, ' ');
            if (p == NULL)
            {
                TraceLog(LOG_WARNING, "AUTOEXEC: Failed to parse autoexec.cfg");
                continue;
            }
            strncpy(szName, szLine, p - szLine);
            szName[p - szLine] = '\0';

            // copy the rest of the line
            strcpy(szValue, p + 1);

            // remove newline
            szValue[strlen(szValue) - 1] = '\0';

            // add entry to config array
            AddToList(config, CONFIG_TYPE_WITHOUT_QUOTES, szName, szValue);
        }
    }

    // close autoexec.cfg
    fclose(fp);
}

static void SaveConfig(AutoexecCfg *list)
{
    // open autoexec.cfg
    FILE *fp = fopen("autoexec.cfg", "w");
    if (fp == NULL)
    {
        TraceLog(LOG_WARNING, "AUTOEXEC: Failed to open autoexec.cfg");
        return;
    }

    // check if list is empty
    if (list == NULL)
        return;

    // print list
    AutoexecCfg *p = list;
    while (p != NULL)
    {
        // bail if key or value is null
        if (p->szKey == NULL || p->szValue == NULL)
        {
            p = p->pNext;
            continue;
        }

        // check if value is a string
        if (p->nType == CONFIG_TYPE_STRING)
        {
            fprintf(fp, "\"%s\" \"%s\"\n", p->szKey, p->szValue);
        }
        else if (p->nType == CONFIG_TYPE_WITHOUT_QUOTES)
        {
            fprintf(fp, "%s %s\n", p->szKey, p->szValue);
        }
        p = p->pNext;
    }

    // close autoexec.cfg
    fclose(fp);
}

static void PrintAll(AutoexecCfg *list)
{
    DWORD dwIndex = 0;
    // check if list is empty
    if (list == NULL)
        return;

    // print list
    AutoexecCfg *p = list;
    while (p != NULL)
    {
        TraceLog(LOG_INFO, "AUTOEXEC: %s %s", p->szKey, p->szValue);
        p = p->pNext;
        dwIndex++;
    }
    TraceLog(LOG_INFO, "AUTOEXEC: %d entries", dwIndex);
}

static bool Launch(DWORD nExecutable)
{
    // launch lithtech.exe
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // get current directory
    char szCurrentDir[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, szCurrentDir);

    // get lithtech.exe path
    char szLithtechPath[MAX_PATH];
    strcpy(szLithtechPath, szCurrentDir);

    if (nExecutable == LITHTECH)
        strcat(szLithtechPath, "\\lithtech.exe");
    else if (nExecutable == SERVER)
        strcat(szLithtechPath, "\\AVP2Serv.exe");

    // get command line
    char szCommandLine[1024];
    strcpy(szCommandLine, szCurrentDir);

    if (nExecutable == LITHTECH)
    {
        strcat(szCommandLine, "\\lithtech.exe -cmdfile avp2cmds.txt ");
        // Append commands
        strcat(szCommandLine, g_Settings.szCommands);
    }

    // start lithtech.exe
    if (!CreateProcess(szLithtechPath, szCommandLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    {
        if (nExecutable == LITHTECH)
        {
            MessageBox(NULL, "Failed to launch lithtech.exe, is this launcher in the right directory?", "Error", MB_OK);
            // close handles
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
            return false;
        }
        else if (nExecutable == SERVER)
        {
            MessageBox(NULL, "Failed to launch AVP2Serv.exe, is this launcher in the right directory?", "Error", MB_OK);
            // close handles
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
            return false;
        }

        return false;
    }

    // close handles
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return true;
}

#endif // LOADER_H