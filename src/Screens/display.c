#include "raylib.h"
#include <string.h>

// Windows Headers
#include <richedit.h>
#include <CommCtrl.h>

// Program Headers
#include "..\include\registry.h"
#include "..\include\utils.h"
#include "..\include\Screens\display.h"

// Win32 Stuff
static HWND g_hWnd;
static HWND hWndResolutionListBox;
static HWND hWndRendererListBox;
static HWND hWndDisplayAdapterListBox;
static WNDPROC oldWndProc;

// BUTTONS
static Button xButton;
static Button okButton;
static Button cancelButton;
static Button **buttons;

uint8_t nResolutionSelection = 0;
uint8_t nRendererSelection = 0;
AutoexecCfg *autoexec;

// LOCAL VARIABLES
static uint32_t nCurrentToolTip = 0;

// Function pointers for the original render and update loops
static void *pOldRenderLoop;
static void *pOldUpdateLoop;

// static RendererInfo pRendererInfo[6];
static uint8_t nDisplaySelection = 0;

HANDLE hThread1, renderEnumThread;
static bool bRenderThreadDone = false;
CRITICAL_SECTION g_csRendererInfo;

static int nRendererCount = 0;
RendererInfo **pRendererInfo;

DWORD WINAPI LoadRendererInfoThread(LPVOID lpParam)
{
    InitializeCriticalSection(&g_csRendererInfo);

    ThreadParam *pThreadParam = (ThreadParam *)lpParam;

    HWND hWndResolutionList = pThreadParam->hWndResolutionLB;
    HWND hWndRendererListBox = pThreadParam->hWndRendererLB;
    HWND hWndDisplayListBox = pThreadParam->hWndDisplayLB;

    // load each .ren file in the directory
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    char szDir[MAX_PATH];

    GetCurrentDirectory(MAX_PATH, szDir);
    strcat(szDir, "\\*.ren");

    hFind = FindFirstFile(szDir, &FindFileData);

    if (hFind == INVALID_HANDLE_VALUE)
    {
        MessageBox(NULL, "Failed to find any .ren files!", "Error", MB_OK | MB_ICONERROR);
        return 0;
    }
    nRendererCount = 0;

    EnterCriticalSection(&g_csRendererInfo);

    pRendererInfo = (RendererInfo **)malloc(sizeof(RendererInfo *) * 1);

    LeaveCriticalSection(&g_csRendererInfo);

    do
    {
        bool bIsValidRenderer = true;
        // load library
        HMODULE hdll = LoadLibrary(FindFileData.cFileName);
        if (hdll == NULL)
        {
            MessageBox(NULL, "Failed to load .ren file!", "Error", MB_OK | MB_ICONERROR);
            continue;
        }

        EnterCriticalSection(&g_csRendererInfo);
        if (nRendererCount > 0)
            pRendererInfo = (RendererInfo **)realloc(pRendererInfo, sizeof(RendererInfo *) * (nRendererCount + 1));

        pRendererInfo[nRendererCount] = (RendererInfo *)malloc(sizeof(RendererInfo));

        LoadString(hdll, RDLL_DESCRIPTION_STRINGID, pRendererInfo[nRendererCount]->szModuleName, 256);
        strcpy(pRendererInfo[nRendererCount]->szModuleFileName, FindFileData.cFileName);

        // use GetSupportedModes in the d3d.ren file to get the supported modes
        typedef struct RMode *(*GetSupportedModes)(void *);
        GetSupportedModes pGetSupportedModes = (GetSupportedModes)GetProcAddress(hdll, "GetSupportedModes");

        if (pGetSupportedModes == NULL)
        {
            // MessageBox(NULL, "Failed to get GetSupportedModes function!", "Error", MB_OK | MB_ICONERROR);
            FreeLibrary(hdll);
            bIsValidRenderer = false;
            continue;
        }

        struct RMode *pRenderer = pGetSupportedModes(NULL);

        if (pRenderer == NULL)
        {
            // MessageBox(NULL, "Failed to get supported modes!", "Error", MB_OK | MB_ICONERROR);
            FreeLibrary(hdll);
            bIsValidRenderer = false;
            continue;
        }

        if ((int)pRenderer->m_bHardware != 0 && (int)pRenderer->m_bHardware != 1)
        {
            // char szError[256];
            // sprintf(szError, "Invalid renderer: %s", FindFileData.cFileName);
            // MessageBox(NULL, szError, "Error", MB_OK | MB_ICONERROR);
            bIsValidRenderer = false;
            FreeLibrary(hdll);
            continue;
        }

        char szDisplayName[256];
        memset(szDisplayName, 0, sizeof(szDisplayName));

        int nDisplayCount = 0;
        int nResolutionCount = 0;

        struct RMode *pMode = pRenderer;

        pRendererInfo[nRendererCount]->nNumDisplays = nDisplayCount + 1;
        pRendererInfo[nRendererCount]->pDisplays = (Displays **)malloc(sizeof(Displays *) * 1);

        while (pMode != NULL && bIsValidRenderer)
        {
            if (pMode == NULL)
                break;

            // compare display name to szDisplayName
            if (strcmp(szDisplayName, pMode->m_InternalName) == 0)
            {
                // reaalloc the resolutions array
                pRendererInfo[nRendererCount]->pDisplays[nDisplayCount - 1]->pResolutions = (Resolution **)realloc(pRendererInfo[nRendererCount]->pDisplays[nDisplayCount - 1]->pResolutions, sizeof(Resolution *) * (nResolutionCount + 1));
                pRendererInfo[nRendererCount]->pDisplays[nDisplayCount - 1]->pResolutions[nResolutionCount] = (Resolution *)malloc(sizeof(Resolution));
                pRendererInfo[nRendererCount]->pDisplays[nDisplayCount - 1]->pResolutions[nResolutionCount]->nWidth = pMode->m_Width;
                pRendererInfo[nRendererCount]->pDisplays[nDisplayCount - 1]->pResolutions[nResolutionCount]->nHeight = pMode->m_Height;

                nResolutionCount++;
                pRendererInfo[nRendererCount]->pDisplays[nDisplayCount - 1]->nNumResolutions = nResolutionCount;
            }
            else
            {
                // this display name is not in the list, so add it to the list
                strcpy(szDisplayName, pMode->m_InternalName);

                nResolutionCount = 0;

                if (nDisplayCount > 0)
                {
                    pRendererInfo[nRendererCount]->nNumDisplays = nDisplayCount + 1;
                    pRendererInfo[nRendererCount]->pDisplays = (Displays **)realloc(pRendererInfo[nRendererCount]->pDisplays, sizeof(Displays *) * (nDisplayCount + 1));
                }

                pRendererInfo[nRendererCount]->pDisplays[nDisplayCount] = (Displays *)malloc(sizeof(Displays));
                strcpy(pRendererInfo[nRendererCount]->pDisplays[nDisplayCount]->szDisplayName, pMode->m_Description);
                strcpy(pRendererInfo[nRendererCount]->pDisplays[nDisplayCount]->szInternalName, pMode->m_InternalName);

                pRendererInfo[nRendererCount]->pDisplays[nDisplayCount]->pResolutions = (Resolution **)malloc(sizeof(Resolution *) * 1);

                pRendererInfo[nRendererCount]->pDisplays[nDisplayCount]->nNumResolutions = nResolutionCount + 1;

                pRendererInfo[nRendererCount]->pDisplays[nDisplayCount]->pResolutions[nResolutionCount] = (Resolution *)malloc(sizeof(Resolution));

                pRendererInfo[nRendererCount]->pDisplays[nDisplayCount]->pResolutions[nResolutionCount]->nHeight = pMode->m_Height;
                pRendererInfo[nRendererCount]->pDisplays[nDisplayCount]->pResolutions[nResolutionCount]->nWidth = pMode->m_Width;

                nResolutionCount++;
                pRendererInfo[nRendererCount]->pDisplays[nDisplayCount]->nNumResolutions = nResolutionCount;

                nDisplayCount++;
            }

            pMode = pMode->m_pNext;
        }

        LeaveCriticalSection(&g_csRendererInfo);

        // free library
        FreeLibrary(hdll);

        nRendererCount++;

    } while (FindNextFile(hFind, &FindFileData) != 0);

    FindClose(hFind);

    EnterCriticalSection(&g_csRendererInfo);

    if (nRendererCount > 0)
    {
        // loop through all the renderers and displays and resolutions and add them to the listbox
        for (int i = 0; i < nRendererCount; i++)
        {
            char szRenderer[256];
            sprintf(szRenderer, "%s (%s)", pRendererInfo[i]->szModuleName, pRendererInfo[i]->szModuleFileName);
            SendMessage(hWndRendererListBox, LB_ADDSTRING, 0, (LPARAM)szRenderer);
        }

        // select the first renderer
        SendMessage(hWndRendererListBox, LB_SETCURSEL, 0, 0);

        // add displays to the display listbox
        for (int i = 0; i < pRendererInfo[nRendererSelection]->nNumDisplays; i++)
        {
            char szDisplay[256];
            sprintf(szDisplay, "%s (%s)", pRendererInfo[nRendererSelection]->pDisplays[i]->szDisplayName, pRendererInfo[nRendererSelection]->pDisplays[i]->szInternalName);
            SendMessage(hWndDisplayListBox, LB_ADDSTRING, 0, (LPARAM)szDisplay);
        }

        // select the first display
        SendMessage(hWndDisplayListBox, LB_SETCURSEL, 0, 0);

        // add resolutions to the resolution listbox based on nDisplaySelection
        for (int j = 0; j < pRendererInfo[nRendererSelection]->pDisplays[nDisplaySelection]->nNumResolutions; j++)
        {
            char szResolution[256];
            sprintf(szResolution, "%dx%d",
                    pRendererInfo[nRendererSelection]->pDisplays[nDisplaySelection]->pResolutions[j]->nWidth,
                    pRendererInfo[nRendererSelection]->pDisplays[nDisplaySelection]->pResolutions[j]->nHeight);
            SendMessage(hWndResolutionList, LB_ADDSTRING, 0, (LPARAM)szResolution);
        }

        // select the first resolution
        SendMessage(hWndResolutionList, LB_SETCURSEL, 0, 0);
    }
    bRenderThreadDone = TRUE;
    free(lpParam);
    LeaveCriticalSection(&g_csRendererInfo);

    return 0;
}

// callback
static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
    {
        if ((HWND)lParam == hWndResolutionListBox)
        {
            if (HIWORD(wParam) == LBN_SELCHANGE)
            {
                nResolutionSelection = (uint8_t)SendMessage(hWndResolutionListBox, LB_GETCURSEL, 0, 0);
            }
        }
        else if ((HWND)lParam == hWndRendererListBox) // hWndDisplayListBox
        {
            if (HIWORD(wParam) == LBN_SELCHANGE)
            {
                nRendererSelection = (uint8_t)SendMessage(hWndRendererListBox, LB_GETCURSEL, 0, 0);

                // clear display listbox
                SendMessage(hWndDisplayAdapterListBox, LB_RESETCONTENT, 0, 0);

                // clear resolution listbox
                SendMessage(hWndResolutionListBox, LB_RESETCONTENT, 0, 0);

                // add displays to the display listbox from the selected renderer
                for (int i = 0; i < pRendererInfo[nRendererSelection]->nNumDisplays; i++)
                {
                    char szDisplay[256];
                    sprintf(szDisplay, "%s (%s)", pRendererInfo[nRendererSelection]->pDisplays[i]->szDisplayName, pRendererInfo[nRendererSelection]->pDisplays[i]->szInternalName);
                    SendMessage(hWndDisplayAdapterListBox, LB_ADDSTRING, 0, (LPARAM)szDisplay);
                }

                // select the first display
                SendMessage(hWndDisplayAdapterListBox, LB_SETCURSEL, 0, 0);

                // add resolutions to the resolution listbox based on nDisplaySelection
                for (int j = 0; j < pRendererInfo[nRendererSelection]->pDisplays[nDisplaySelection]->nNumResolutions; j++)
                {
                    char szResolution[256];
                    sprintf(szResolution, "%dx%d",
                            pRendererInfo[nRendererSelection]->pDisplays[nDisplaySelection]->pResolutions[j]->nWidth,
                            pRendererInfo[nRendererSelection]->pDisplays[nDisplaySelection]->pResolutions[j]->nHeight);
                    SendMessage(hWndResolutionListBox, LB_ADDSTRING, 0, (LPARAM)szResolution);
                }

                SendMessage(hWndResolutionListBox, LB_SETCURSEL, 0, 0);
            }
        }
        else if ((HWND)lParam == hWndDisplayAdapterListBox)
        {
            nDisplaySelection = (uint8_t)SendMessage(hWndDisplayAdapterListBox, LB_GETCURSEL, 0, 0);

            // clear resolution listbox
            SendMessage(hWndResolutionListBox, LB_RESETCONTENT, 0, 0);

            // add resolutions to the resolution listbox based on nDisplaySelection
            for (int j = 0; j < pRendererInfo[nRendererSelection]->pDisplays[nDisplaySelection]->nNumResolutions; j++)
            {
                char szResolution[256];
                sprintf(szResolution, "%dx%d",
                        pRendererInfo[nRendererSelection]->pDisplays[nDisplaySelection]->pResolutions[j]->nWidth,
                        pRendererInfo[nRendererSelection]->pDisplays[nDisplaySelection]->pResolutions[j]->nHeight);
                SendMessage(hWndResolutionListBox, LB_ADDSTRING, 0, (LPARAM)szResolution);
            }
            SendMessage(hWndResolutionListBox, LB_SETCURSEL, 0, 0);
        }
    }
    case WM_CTLCOLORLISTBOX:
    {

        if ((HWND)lParam == hWndResolutionListBox || (HWND)lParam == hWndRendererListBox || (HWND)lParam == hWndDisplayAdapterListBox)
        {
            HDC dc = (HDC)wParam;
            SetBkMode(dc, OPAQUE);
            SetTextColor(dc, RGB(0, 255, 0));
            SetBkColor(dc, RGB(0, 0, 0));
            HBRUSH comboBrush = CreateSolidBrush(RGB(0, 0, 0));
            return (LRESULT)comboBrush;
        }
    }
    case WM_CTLCOLOREDIT:
    {
        HDC dc = (HDC)wParam;
        if ((HWND)lParam == hWndResolutionListBox || (HWND)lParam == hWndRendererListBox || (HWND)lParam == hWndDisplayAdapterListBox)
        {
            SetBkMode(dc, OPAQUE);
            SetTextColor(dc, RGB(0, 255, 0));
            SetBkColor(dc, RGB(0, 0, 0)); // 0x383838
            HBRUSH comboBrush = CreateSolidBrush(RGB(0, 0, 0));
            return (LRESULT)comboBrush;
        }
    }
    }
    return CallWindowProc(oldWndProc, hWnd, message, wParam, lParam);
}

static void OnButtonPressOK(Button *button)
{
    SetWindowSize(AVP2_MAIN_SCREEN_WIDTH, AVP2_MAIN_SCREEN_HEIGHT);
    g_nCurrentScreen = SCREEN_SPLASH;

    PlaySoundResource("OK");

    ScreenUpdateLoop = pOldUpdateLoop;
    ScreenRenderLoop = pOldRenderLoop;

    // save the settings to autoexec.cfg
    UpdateKeyInList(autoexec, "RENDERDLL", pRendererInfo[nRendererSelection]->szModuleFileName);

    // convert int to string
    char szWidth[8];
    char szHeight[8];
    itoa(pRendererInfo[nRendererSelection]->pDisplays[nDisplaySelection]->pResolutions[nResolutionSelection]->nWidth, szWidth, 10);
    itoa(pRendererInfo[nRendererSelection]->pDisplays[nDisplaySelection]->pResolutions[nResolutionSelection]->nHeight, szHeight, 10);

    UpdateKeyInList(autoexec, "SCREENWIDTH", szWidth);
    UpdateKeyInList(autoexec, "SCREENHEIGHT", szHeight);
    UpdateKeyInList(autoexec, "GameScreenHeight", szHeight);
    UpdateKeyInList(autoexec, "GameScreenWidth", szWidth);
    UpdateKeyInList(autoexec, "CARDDESC", pRendererInfo[nRendererSelection]->pDisplays[nDisplaySelection]->szInternalName);

    SaveConfig(autoexec);

    ShowWindow(hWndResolutionListBox, SW_HIDE);
    ShowWindow(hWndRendererListBox, SW_HIDE);
    ShowWindow(hWndDisplayAdapterListBox, SW_HIDE);
}

static void OnButtonPressCancel(Button *button)
{
    SetWindowSize(AVP2_MAIN_SCREEN_WIDTH, AVP2_MAIN_SCREEN_HEIGHT);

    g_nCurrentScreen = SCREEN_SPLASH;

    PlaySoundResource("BACK");

    ScreenUpdateLoop = pOldUpdateLoop;
    ScreenRenderLoop = pOldRenderLoop;

    ShowWindow(hWndResolutionListBox, SW_HIDE);
    ShowWindow(hWndRendererListBox, SW_HIDE);
    ShowWindow(hWndDisplayAdapterListBox, SW_HIDE);
}

void DisplaySetupScreen(void *pRenderLoop, void *pUpdateLoop)
{
    static bool bFirstTime = TRUE;

    // save old render and update loops
    pOldRenderLoop = pRenderLoop;
    pOldUpdateLoop = pUpdateLoop;

    // set new render and update loops so our main loop is happy
    ScreenRenderLoop = DisplayRenderScreen;
    ScreenUpdateLoop = DisplayUpdateLoop;

    SetProcessDpiAware();

    g_hWnd = GetWindowHandle();

    if (bFirstTime)
    {

        hWndResolutionListBox = CreateWindowEx(0, TEXT("LISTBOX"), TEXT("1"),
                                               WS_CHILD | WS_OVERLAPPED | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY,
                                               20, 78, 126, 240, g_hWnd, NULL, NULL, NULL);

        hWndRendererListBox = CreateWindowEx(0, TEXT("LISTBOX"), TEXT("2"),
                                             WS_CHILD | WS_OVERLAPPED | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY,
                                             180, 78, 403, 105, g_hWnd, NULL, NULL, NULL);

        hWndDisplayAdapterListBox = CreateWindowEx(0, TEXT("LISTBOX"), TEXT("3"),
                                                   WS_CHILD | WS_OVERLAPPED | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY,
                                                   180, 206, 403, 105, g_hWnd, NULL, NULL, NULL);

        // set font size to 14
        // SendMessage(hwndListBox, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));

        // set font
        HFONT hFont = CreateFontA(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
                                  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_MODERN, "Segoe UI");
        SendMessage(hWndResolutionListBox, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hWndRendererListBox, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hWndDisplayAdapterListBox, WM_SETFONT, (WPARAM)hFont, TRUE);

        // set callback
        oldWndProc = (WNDPROC)SetWindowLongPtr(g_hWnd, GWLP_WNDPROC, (LONG_PTR)WndProc);

        autoexec = (AutoexecCfg *)malloc(sizeof(AutoexecCfg));
        autoexec->nType = 0;
        autoexec->pNext = NULL;
        autoexec->szKey = NULL;
        autoexec->szValue = NULL;

        LoadAutoexecCfg(autoexec);

        // remove first node its nulled - #TODO: fix this
        void *temp = autoexec;
        autoexec = autoexec->pNext;
        free(temp);
    }
    else
    {
        // unhide the listbox
        ShowWindow(hWndResolutionListBox, SW_SHOW);
        ShowWindow(hWndRendererListBox, SW_SHOW);
        ShowWindow(hWndDisplayAdapterListBox, SW_SHOW);
    }

    SetWindowSize(600, 394);

    if (bFirstTime)
    {

        // x button
        if (xButton.texture[0].id == 0)
        {
            LoadTextureFromResource(&xButton.texture[0], "CLOSEU", false);
            LoadTextureFromResource(&xButton.texture[1], "CLOSEF", false);
            LoadTextureFromResource(&xButton.texture[2], "CLOSED", false);
        }
        xButton.position = (Vector2){579, 1};
        xButton.onPress = OnButtonPressCancel;
        xButton.onUnload = UnloadButton;
        xButton.isEnabled = TRUE;

        // generic
        if (okButton.texture[0].id == 0)
        {
            LoadTextureFromResource(&okButton.texture[0], "OKU", false);
            LoadTextureFromResource(&okButton.texture[1], "OKF", false);
            LoadTextureFromResource(&okButton.texture[2], "OKD", false);
        }
        okButton.position = (Vector2){194, 349};
        okButton.onPress = OnButtonPressOK;
        okButton.onUnload = UnloadButton;
        okButton.isEnabled = TRUE;

        if (cancelButton.texture[0].id == 0)
        {
            LoadTextureFromResource(&cancelButton.texture[0], "CANCELU", false);
            LoadTextureFromResource(&cancelButton.texture[1], "CANCELF", false);
            LoadTextureFromResource(&cancelButton.texture[2], "CANCELD", false);
        }
        cancelButton.position = (Vector2){306, 349};
        cancelButton.onPress = OnButtonPressCancel;
        cancelButton.onUnload = UnloadButton;
        cancelButton.isEnabled = TRUE;

        // seTUP BUTTONS
        buttons = (Button **)malloc(sizeof(Button *) * DISPLAY_BUTTON_COUNT);

        if (!buttons)
        {
            MessageBox(NULL, "Failed to allocate memory for buttons!", "Error", MB_OK | MB_ICONERROR);
            exit(1);
        }

        for (size_t i = 0; i < DISPLAY_BUTTON_COUNT; i++)
        {
            buttons[i] = (Button *)malloc(sizeof(Button));

            if (!buttons[i])
            {
                MessageBox(NULL, "Failed to allocate memory for buttons!", "Error", MB_OK | MB_ICONERROR);
                exit(1);
            }
        }

        buttons[0] = &okButton;
        buttons[1] = &cancelButton;
        buttons[2] = &xButton;

        // Create thread for GetModes function
        pThreadParam pThreadInfo = (pThreadParam)malloc(sizeof(ThreadParam));
        pThreadInfo->hWndRendererLB = hWndRendererListBox;
        pThreadInfo->hWndResolutionLB = hWndResolutionListBox;
        pThreadInfo->hWndDisplayLB = hWndDisplayAdapterListBox;

        renderEnumThread = CreateThread(NULL, 0, LoadRendererInfoThread, (LPVOID)pThreadInfo, 0, NULL);
        bFirstTime = FALSE;
    }
}

void DisplayRenderScreen(void)
{
    // Start the 2D Canvas
    BeginDrawing();
    ClearBackground(BLACK);

    DrawTexture(g_backgroundImage[g_nCurrentScreen], 0, 0, WHITE);

    for (size_t i = 0; i < DISPLAY_BUTTON_COUNT; i++)
    {
        if (buttons[i]->isEnabled == TRUE)
        {
            DrawTexture(buttons[i]->texture[buttons[i]->currentTexture], buttons[i]->position.x, buttons[i]->position.y, WHITE);
        }
    }

    if (nCurrentToolTip > 0)
    {
        char *temp = FormatStringWithNewLines(AVP2_TOOLTIPS[nCurrentToolTip], (Rect){26, 290, 400, 300});

        DrawTextExRay(g_font, temp, (Vector2){26, 290}, 18, 1, WHITE);

        free(temp);
    }

    EndDrawing();
}

void DisplayUnloadScreen(void)
{
    // null check
    if (buttons == NULL)
    {
        return;
    }

    for (size_t i = 0; i < DISPLAY_BUTTON_COUNT; i++)
    {
        buttons[i]->onUnload(buttons[i]);
        free(buttons[i]);
        buttons[i] = NULL;
    }
    free(buttons);
    buttons = NULL;

    // free memory in pRendererInfo array
    for (size_t renderIndex = 0; renderIndex < nRendererCount; renderIndex++)
    {
        for (size_t displayIndex = 0; displayIndex < pRendererInfo[renderIndex]->nNumDisplays; displayIndex++)
        {
            for (size_t resolutionIndex = 0; resolutionIndex < pRendererInfo[renderIndex]->pDisplays[displayIndex]->nNumResolutions; resolutionIndex++)
            {
                free(pRendererInfo[renderIndex]->pDisplays[displayIndex]->pResolutions[resolutionIndex]);
                pRendererInfo[renderIndex]->pDisplays[displayIndex]->pResolutions[resolutionIndex] = NULL;
            }

            free(pRendererInfo[renderIndex]->pDisplays[displayIndex]->pResolutions);
            pRendererInfo[renderIndex]->pDisplays[displayIndex]->pResolutions = NULL;
        }
        free(pRendererInfo[renderIndex]);
        pRendererInfo[renderIndex] = NULL;
    }
    free(pRendererInfo);

    FreeList(autoexec);
}

static void CheckAllButtons(void)
{

    for (size_t i = 0; i < DISPLAY_BUTTON_COUNT; i++)
    {
        if (CheckCollisionPointRec(GetMousePosition(), (Rect){buttons[i]->position.x, buttons[i]->position.y, buttons[i]->texture[0].width, buttons[i]->texture[0].height}) && buttons[i]->isEnabled == TRUE)
        {
            buttons[i]->currentTexture = HOVER;

            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
            {
                buttons[i]->currentTexture = DOWN;
            }

            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
            {
                buttons[i]->currentTexture = HOVER;
                buttons[i]->onPress(buttons[i]);
            }
        }
        else
        {
            buttons[i]->currentTexture = UP;
            buttons[i]->isPressed = FALSE;
        }
    }
}

void DisplayUpdateLoop(void)
{

    if (bRenderThreadDone)
    {
        // free thread
        CloseHandle(renderEnumThread);
        renderEnumThread = NULL;

        // force refresh of renderer listbox
        bRenderThreadDone = FALSE;
    }

    nCurrentToolTip = 0;
    CheckAllButtons();
}