#ifndef LOADER_H
#define LOADER_H
#include <Windows.h>
#include "raylib.h"


//C headers
#include <stdint.h>
#include <stdlib.h>
#include "string.h"

#include <shellscalingapi.h>    //used to get DPI scaling
#include <winerror.h>
#include <wingdi.h>
#include <time.h>

//Program headers
#include "constants.h"

#define GET_X_LPARAM(lp)    ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp)    ((int)(short)HIWORD(lp))
typedef BOOL (WINAPI * SETPROCESSDPIAWARE_T)(void);
typedef HRESULT (WINAPI * SETPROCESSDPIAWARENESS_T)(PROCESS_DPI_AWARENESS);

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

    //get size of resource
    DWORD dwSize = SizeofResource(hInst, hResSource);

    LPSTR lpRes = LockResource(hRes);
    if (lpRes == NULL)
    {
        // handle error
        FreeResource(hRes);
        return;
    }

    //Use raylib to play the sound, since it has easier volume controls
    Wave wave = {0};
    wave = LoadWaveFromMemory(".wav", lpRes, dwSize);
    Sound sound = LoadSoundFromWave(wave);
    SetSoundVolume(sound, 0.3f);
    PlayAudioStream(sound.stream);

    UnlockResource(hRes);
    FreeResource(hRes);
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

        //construct a bitmap header since it gets stripped from the resource
        BITMAPFILEHEADER bitmapFileHeader = {0};
        bitmapFileHeader.bfType = 0x4D42; // BM
        bitmapFileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + nFileSizeLessHeader;
        bitmapFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

        void *pData = calloc(1, nFileSizeLessHeader + sizeof(BITMAPFILEHEADER));

        if (!pData) continue;

        memcpy(pData, &bitmapFileHeader, sizeof(BITMAPFILEHEADER));
        memcpy((char*)pData + sizeof(BITMAPFILEHEADER), pResource, nFileSizeLessHeader);

        // convert bitmap to image
        Image iImage = LoadImageFromMemory(".bmp", pData, nFileSizeLessHeader + sizeof(BITMAPFILEHEADER));

        g_backgroundImage[i] = LoadTextureFromImage(iImage);
        UnloadImage(iImage);
        UnlockResource(hGlobal);
        FreeResource(hGlobal);
        free(pData);
    }
}

static void LoadTextureFromResource(Texture *texture, const char *name)
{
    HMODULE hInst = GetModuleHandle(NULL);

    HRSRC hRes = FindResource(hInst, name, RT_BITMAP);
    HGLOBAL hGlobal = LoadResource(hInst, hRes);
    void *pResource = LockResource(hGlobal);
    int nFileSizeLessHeader = SizeofResource(NULL, hRes);

    //construct a bitmap header since it gets stripped from the resource
    BITMAPFILEHEADER bitmapFileHeader = {0};
    bitmapFileHeader.bfType = 0x4D42; // BM
    bitmapFileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + nFileSizeLessHeader;
    bitmapFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    void *pData = calloc(1, nFileSizeLessHeader + sizeof(BITMAPFILEHEADER));

    if (!pData) return;

    memcpy(pData, &bitmapFileHeader, sizeof(BITMAPFILEHEADER));
    memcpy((char*)pData + sizeof(BITMAPFILEHEADER), pResource, nFileSizeLessHeader);

    // convert bitmap to image
    Image iImage = LoadImageFromMemory(".bmp", pData, nFileSizeLessHeader + sizeof(BITMAPFILEHEADER));

    *texture = LoadTextureFromImage(iImage);
    UnloadImage(iImage);
    UnlockResource(hGlobal);
    FreeResource(hGlobal);
    free(pData);
}

static void Loader_InitializeAllTextures()
{
    //Set the window icon
    HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APPICON));
    HWND hwnd = GetWindowHandle();

    SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
    SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);

    g_font = LoadFontEx("C:\\Windows\\Fonts\\segoeui.ttf", 16, 0, 250);

    SetTextureFilter(g_font.texture, TEXTURE_FILTER_POINT); // Texture scale filter to use

    LoadBackgroundImages();

    // play button
    LoadTextureFromResource(&g_playButton.texture[0], "PLAYU");
    LoadTextureFromResource(&g_playButton.texture[1], "PLAYF");
    LoadTextureFromResource(&g_playButton.texture[2], "PLAYD");
    g_playButton.position = (Vector2){417, 19};
    g_playButton.onPress = ButtonPressCallback;
    g_playButton.onUnload = UnloadButton;
    g_playButton.isEnabled = TRUE;
    strcpy(g_playButton.szName, "play");

    // server button
    LoadTextureFromResource(&g_serverButton.texture[0], "SERVERU");
    LoadTextureFromResource(&g_serverButton.texture[1], "SERVERF");
    LoadTextureFromResource(&g_serverButton.texture[2], "SERVERD");
    g_serverButton.position = (Vector2){417, 55};
    g_serverButton.onPress = ButtonPressCallback;
    g_serverButton.onUnload = UnloadButton;
    g_serverButton.isEnabled = TRUE;
    strcpy(g_serverButton.szName, "server");

    // display button
    LoadTextureFromResource(&g_displayButton.texture[0], "DISPLAYU");
    LoadTextureFromResource(&g_displayButton.texture[1], "DISPLAYF");
    LoadTextureFromResource(&g_displayButton.texture[2], "DISPLAYD");
    g_displayButton.position = (Vector2){417, 91};
    g_displayButton.onPress = ButtonPressCallback;
    g_displayButton.onUnload = UnloadButton;
    g_displayButton.isEnabled = TRUE;
    strcpy(g_displayButton.szName, "display");

    // options button
    LoadTextureFromResource(&g_optionsButton.texture[0], "OPTIONSU");
    LoadTextureFromResource(&g_optionsButton.texture[1], "OPTIONSF");
    LoadTextureFromResource(&g_optionsButton.texture[2], "OPTIONSD");
    g_optionsButton.position = (Vector2){417, 127};
    g_optionsButton.onPress = ButtonPressCallback;
    g_optionsButton.onUnload = UnloadButton;
    g_optionsButton.isEnabled = TRUE;
    strcpy(g_optionsButton.szName, "options");

    // exit button
    LoadTextureFromResource(&g_exitButton.texture[0], "QUITU");
    LoadTextureFromResource(&g_exitButton.texture[1], "QUITF");
    LoadTextureFromResource(&g_exitButton.texture[2], "QUITD");
    g_exitButton.position = (Vector2){417, 199};
    g_exitButton.onPress = ButtonPressCallback;
    g_exitButton.onUnload = UnloadButton;
    g_exitButton.isEnabled = TRUE;
    strcpy(g_exitButton.szName, "exit");

    // minimize button
    LoadTextureFromResource(&g_minimizeButton.texture[0], "MINIMIZEU");
    LoadTextureFromResource(&g_minimizeButton.texture[1], "MINIMIZEF");
    LoadTextureFromResource(&g_minimizeButton.texture[2], "MINIMIZED");
    g_minimizeButton.position = (Vector2){485, 1};
    g_minimizeButton.onPress = ButtonPressCallback;
    g_minimizeButton.onUnload = UnloadButton;
    g_minimizeButton.isEnabled = TRUE;
    strcpy(g_minimizeButton.szName, "minimize");

    // x button
    LoadTextureFromResource(&g_xButton.texture[0], "CLOSEU");
    LoadTextureFromResource(&g_xButton.texture[1], "CLOSEF");
    LoadTextureFromResource(&g_xButton.texture[2], "CLOSED");
    g_xButton.position = (Vector2){505, 1};
    g_xButton.onPress = ButtonPressCallback;
    g_xButton.onUnload = UnloadButton;
    g_xButton.isEnabled = TRUE;
    strcpy(g_xButton.szName, "x");

    // generic
    LoadTextureFromResource(&g_okButton.texture[0], "OKU");
    LoadTextureFromResource(&g_okButton.texture[1], "OKF");
    LoadTextureFromResource(&g_okButton.texture[2], "OKD");
    g_okButton.position = (Vector2){0, 0};
    g_okButton.onPress = ButtonPressCallback;
    g_okButton.onUnload = UnloadButton;
    g_okButton.isEnabled = TRUE;
    strcpy(g_okButton.szName, "ok");

    LoadTextureFromResource(&g_cancelButton.texture[0], "CANCELU");
    LoadTextureFromResource(&g_cancelButton.texture[1], "CANCELF");
    LoadTextureFromResource(&g_cancelButton.texture[2], "CANCELD");
    g_cancelButton.position = (Vector2){0, 0};
    g_cancelButton.onPress = ButtonPressCallback;
    g_cancelButton.onUnload = UnloadButton;
    g_cancelButton.isEnabled = TRUE;
    strcpy(g_cancelButton.szName, "cancel");

    LoadTextureFromResource(&g_installButton.texture[0], "INSTALLU");
    LoadTextureFromResource(&g_installButton.texture[1], "INSTALLF");
    LoadTextureFromResource(&g_installButton.texture[2], "INSTALLD");
    g_installButton.position = (Vector2){417, 19};
    g_installButton.onPress = ButtonPressCallback;
    g_installButton.onUnload = UnloadButton;
    strcpy(g_installButton.szName, "install");


    //InstallAVP2Registry

    g_buttons = (Button **)malloc(sizeof(Button *) * BUTTON_COUNT);

    if (!g_buttons)
    {
        MessageBox(NULL, "Failed to allocate memory for buttons!", "Error", MB_OK | MB_ICONERROR);
        exit(1);
    }

    for (size_t i = 0; i < BUTTON_COUNT; i++)
    {
        g_buttons[i] = (Button *)malloc(sizeof(Button));
        
        if (!g_buttons[i])
        {
            MessageBox(NULL, "Failed to allocate memory for buttons!", "Error", MB_OK | MB_ICONERROR);
            exit(1);
        }
    }

    g_buttons[0] = &g_playButton;
    g_buttons[1] = &g_serverButton;
    g_buttons[2] = &g_displayButton;
    g_buttons[3] = &g_optionsButton;
    g_buttons[4] = &g_exitButton;
    g_buttons[5] = &g_minimizeButton;
    g_buttons[6] = &g_xButton;
    g_buttons[7] = &g_okButton;
    g_buttons[8] = &g_cancelButton;
    g_buttons[9] = &g_installButton;

    if(g_bAVP2Installed)
    {
        g_installButton.isEnabled = FALSE;
    }
    else
    {
        g_installButton.isEnabled = TRUE;
        g_playButton.isEnabled = FALSE;
        g_serverButton.isEnabled = FALSE;
        g_displayButton.isEnabled = FALSE;
        g_optionsButton.isEnabled = FALSE;
    }
}

static bool win32_SetProcessDpiAware(void) {
    HMODULE shcore = LoadLibraryA("Shcore.dll");
    SETPROCESSDPIAWARENESS_T SetProcessDpiAwareness = NULL;
    if (shcore) {
        SetProcessDpiAwareness = (SETPROCESSDPIAWARENESS_T) GetProcAddress(shcore, "SetProcessDpiAwareness");
    }
    HMODULE user32 = LoadLibraryA("User32.dll");
    SETPROCESSDPIAWARE_T SetProcessDPIAware = NULL;
    if (user32) {
        SetProcessDPIAware = (SETPROCESSDPIAWARE_T) GetProcAddress(user32, "SetProcessDPIAware");
    }

    bool ret = false;
    if (SetProcessDpiAwareness) {
        ret = SetProcessDpiAwareness(PROCESS_SYSTEM_DPI_AWARE) == S_OK;
    } else if (SetProcessDPIAware) {
        ret = SetProcessDPIAware() != 0;
    }

    if (user32) {
        FreeLibrary(user32);
    }
    if (shcore) {
        FreeLibrary(shcore);
    }
    return ret;
}

static char* FormatStringWithNewLines(const char* szString, Rect rTextDrawArea)
{
    //copy string to buffer
    char* szStringTempBuffer = (char*)malloc(strlen(szString) + 1);
    strcpy(szStringTempBuffer, szString);

    //calculate max chars per line
    int nMaxCharactersPerLine = rTextDrawArea.width / MeasureTextEx(g_font, "A", 14, 1).x;

    char* words[1024];
    int numWords = 0;
    char* token = strtok(szStringTempBuffer, " ");
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

    char* szFormattedString = (char*)malloc(strlen(szBuffer) + 1);
    strcpy(szFormattedString, szBuffer);

    free(szStringTempBuffer);

    return szFormattedString;
}

#endif // LOADER_H