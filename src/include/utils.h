#ifndef LOADER_H
#define LOADER_H

//Windows headers
#include <Windows.h>
#include <shellscalingapi.h>    //used to get DPI scaling
#include <winerror.h>
#include <wingdi.h>
#include <time.h>
#include <math.h>

//raylib headers
#include "raylib.h"
#include "raymath.h"

//C headers
#include <stdint.h>
#include <stdlib.h>
#include "string.h"

//Program headers
#include "constants.h"
#include "types.h"
#include <GLFW/glfw3.h>

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

static void LoadTextureFromResource(Texture *pTexture, const char *name)
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

    *pTexture = LoadTextureFromImage(iImage);
    UnloadImage(iImage);
    UnlockResource(hGlobal);
    FreeResource(hGlobal);
    free(pData);
}

static void Loader_InitializeBackgroundTextures()
{
    //Set the window icon
    HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APPICON));
    HWND hwnd = GetWindowHandle();

    SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
    SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);

    g_font = LoadFontEx("C:\\Windows\\Fonts\\segoeui.ttf", 18, 0, 250);

    SetTextureFilter(g_font.texture, TEXTURE_FILTER_POINT); // Texture scale filter to use

    LoadBackgroundImages();

}

static bool SetProcessDpiAware(void) {
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

#endif // LOADER_H