#include "include\utils.h"

#include <stdio.h>

void PlaySoundResource(const char *szName)
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

void PlayRandomIntroSound(void)
{
    InitAudioDevice();

    srand(time(NULL));
    int random = rand() % AVP2_LAUNCHER_INTRO_COUNT;

    PlaySoundResource(AVP2_LAUNCHER_INTRO[random]);
}

void LoadBackgroundImages(void)
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

void LoadTextureFromResource(Texture *pTexture, const char *name, bool bIsPNG)
{
    HMODULE hInst = GetModuleHandle(NULL);

    // Find the resource
    HRSRC hRes = FindResource(hInst, name, bIsPNG ? RT_RCDATA : RT_BITMAP);
    
    // Load the resource
    HGLOBAL hGlobal = LoadResource(hInst, hRes);
    void *pResource = LockResource(hGlobal);
    int nFileSizeLessHeader = SizeofResource(NULL, hRes);

    // Allocate memory for the data
    int nDataSize = nFileSizeLessHeader + (bIsPNG ? 0 : sizeof(BITMAPFILEHEADER));
    void *pData = calloc(1, nDataSize);
    if (!pData)
        return;

    // Copy the resource data to pData
    if (!bIsPNG)
    {
        // Construct a bitmap header since it gets stripped from the resource
        BITMAPFILEHEADER bitmapFileHeader = {0};
        bitmapFileHeader.bfType = 0x4D42; // BM
        bitmapFileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + nFileSizeLessHeader;
        bitmapFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

        memcpy(pData, &bitmapFileHeader, sizeof(BITMAPFILEHEADER));
        memcpy((char *)pData + sizeof(BITMAPFILEHEADER), pResource, nFileSizeLessHeader);
    }
    else
    {
        memcpy(pData, pResource, nFileSizeLessHeader);
    }

    // Convert the data to an image
    Image iImage = LoadImageFromMemory(bIsPNG ? ".png" : ".bmp", pData, nDataSize);

    // Load the texture from the image
    *pTexture = LoadTextureFromImage(iImage);

    // Clean up
    UnloadImage(iImage);
    UnlockResource(hGlobal);
    FreeResource(hGlobal);
    free(pData);
}

// Draw part of a texture (defined by a Rect) with rotation and scale tiled into dest.
void DrawTextureTiled(Texture2D texture, Rect source, Rect dest, Vector2 origin, float rotation, float scale, Color tint)
{
    if ((texture.id <= 0) || (scale <= 0.0f)) return;  // Wanna see a infinite loop?!...just delete this line!
    if ((source.width == 0) || (source.height == 0)) return;

    int tileWidth = (int)(source.width*scale), tileHeight = (int)(source.height*scale);
    if ((dest.width < tileWidth) && (dest.height < tileHeight))
    {
        // Can fit only one tile
        DrawTexturePro(texture, (Rect){source.x, source.y, ((float)dest.width/tileWidth)*source.width, ((float)dest.height/tileHeight)*source.height},
                    (Rect){dest.x, dest.y, dest.width, dest.height}, origin, rotation, tint);
    }
    else if (dest.width <= tileWidth)
    {
        // Tiled vertically (one column)
        int dy = 0;
        for (;dy+tileHeight < dest.height; dy += tileHeight)
        {
            DrawTexturePro(texture, (Rect){source.x, source.y, ((float)dest.width/tileWidth)*source.width, source.height}, (Rect){dest.x, dest.y + dy, dest.width, (float)tileHeight}, origin, rotation, tint);
        }

        // Fit last tile
        if (dy < dest.height)
        {
            DrawTexturePro(texture, (Rect){source.x, source.y, ((float)dest.width/tileWidth)*source.width, ((float)(dest.height - dy)/tileHeight)*source.height},
                        (Rect){dest.x, dest.y + dy, dest.width, dest.height - dy}, origin, rotation, tint);
        }
    }
    else if (dest.height <= tileHeight)
    {
        // Tiled horizontally (one row)
        int dx = 0;
        for (;dx+tileWidth < dest.width; dx += tileWidth)
        {
            DrawTexturePro(texture, (Rect){source.x, source.y, source.width, ((float)dest.height/tileHeight)*source.height}, (Rect){dest.x + dx, dest.y, (float)tileWidth, dest.height}, origin, rotation, tint);
        }

        // Fit last tile
        if (dx < dest.width)
        {
            DrawTexturePro(texture, (Rect){source.x, source.y, ((float)(dest.width - dx)/tileWidth)*source.width, ((float)dest.height/tileHeight)*source.height},
                        (Rect){dest.x + dx, dest.y, dest.width - dx, dest.height}, origin, rotation, tint);
        }
    }
    else
    {
        // Tiled both horizontally and vertically (rows and columns)
        int dx = 0;
        for (;dx+tileWidth < dest.width; dx += tileWidth)
        {
            int dy = 0;
            for (;dy+tileHeight < dest.height; dy += tileHeight)
            {
                DrawTexturePro(texture, source, (Rect){dest.x + dx, dest.y + dy, (float)tileWidth, (float)tileHeight}, origin, rotation, tint);
            }

            if (dy < dest.height)
            {
                DrawTexturePro(texture, (Rect){source.x, source.y, source.width, ((float)(dest.height - dy)/tileHeight)*source.height},
                    (Rect){dest.x + dx, dest.y + dy, (float)tileWidth, dest.height - dy}, origin, rotation, tint);
            }
        }

        // Fit last column of tiles
        if (dx < dest.width)
        {
            int dy = 0;
            for (;dy+tileHeight < dest.height; dy += tileHeight)
            {
                DrawTexturePro(texture, (Rect){source.x, source.y, ((float)(dest.width - dx)/tileWidth)*source.width, source.height},
                        (Rect){dest.x + dx, dest.y + dy, dest.width - dx, (float)tileHeight}, origin, rotation, tint);
            }

            // Draw final tile in the bottom right corner
            if (dy < dest.height)
            {
                DrawTexturePro(texture, (Rect){source.x, source.y, ((float)(dest.width - dx)/tileWidth)*source.width, ((float)(dest.height - dy)/tileHeight)*source.height},
                    (Rect){dest.x + dx, dest.y + dy, dest.width - dx, dest.height - dy}, origin, rotation, tint);
            }
        }
    }
}

void Loader_InitializeBackgroundTextures(void)
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

bool SetProcessDpiAware(void)
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

char *FormatStringWithNewLines(const char *szString, Rect rTextDrawArea)
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

void DragWindow(void)
{
    static Vector2 initialMousePosition = {0, 0};
    static bool bIsDragging = FALSE;

    // Check if user is dragging the window
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && GetMousePosition().y < g_nDragHeight && GetMousePosition().x < GetScreenWidth() - 40)
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

void UnloadButton(Button *button)
{
    for (size_t i = 0; i < 3; i++)
    {
        UnloadTexture(button->texture[i]);
    }
}

void UnloadCheckBox(Checkbox *checkbox)
{
    for (size_t i = 0; i < 2; i++)
    {
        UnloadTexture(checkbox->texture[i]);
    }
}

void AddToList(AutoexecCfg *list, int nType, char *szKey, char *szValue)
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

void FreeList(AutoexecCfg *list)
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

AutoexecCfg *FindKeyInList(AutoexecCfg *list, char *szSearchKey)
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

void UpdateKeyInList(AutoexecCfg *list, char *szSearchKey, char *szNewValue)
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

void LoadAutoexecCfg(AutoexecCfg *config)
{
    // open autoexec.cfg
    FILE *fp = fopen("autoexec.cfg", "r");
    if (fp == NULL)
    {
        // TraceLog(LOG_WARNING, "AUTOEXEC: Failed to open autoexec.cfg");
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
                // TraceLog(LOG_WARNING, "AUTOEXEC: Failed to parse autoexec.cfg");
                continue;
            }
            strncpy(szName, szLine + 1, p - szLine - 1);
            szName[p - szLine - 1] = '\0';

            // get value
            p = strchr(p + 1, '"');
            if (p == NULL)
            {
                // TraceLog(LOG_WARNING, "AUTOEXEC: Failed to parse autoexec.cfg");
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
                // TraceLog(LOG_WARNING, "AUTOEXEC: Failed to parse autoexec.cfg");
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

void SaveConfig(AutoexecCfg *list)
{
    // open autoexec.cfg
    FILE *fp = fopen("autoexec.cfg", "w");
    if (fp == NULL)
    {
        // TraceLog(LOG_WARNING, "AUTOEXEC: Failed to open autoexec.cfg");
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

bool Launch(DWORD nExecutable)
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