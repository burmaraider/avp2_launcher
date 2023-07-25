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
static HWND hwndComboBox;
static HWND hwndListBox;
static WNDPROC oldWndProc;
static WNDPROC oldListProc;
;

// BUTTONS
static Button xButton;
static Button okButton;
static Button cancelButton;
static Button **buttons;

static Monitor **pMonitorArray;
uint8_t listboxIndex = 0;

// LOCAL VARIABLES
static uint32_t nCurrentToolTip = 0;
static bool bIsClosing = false;

// Function pointers for the original render and update loops
static void *pOldRenderLoop;
static void *pOldUpdateLoop;

static void GetModes();

// callback

static LRESULT CALLBACK listBoxCallBackProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    // get the index from listbox
    case LBN_SELCHANGE:
    {
        listboxIndex = (uint8_t)SendMessage(hwndListBox, LB_GETCURSEL, 0, 0);
        break;
    }
    }
    return CallWindowProc(oldWndProc, hWnd, message, wParam, lParam);
}
// Windows Process
static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {

    case WM_COMMAND:
    {
        if ((HWND)lParam == hwndListBox)
        {
            if (HIWORD(wParam) == LBN_SELCHANGE)
            {
                listboxIndex = (uint8_t)SendMessage(hwndListBox, LB_GETCURSEL, 0, 0);
            }
        }
    }
    case WM_CTLCOLORLISTBOX:
    {
        COMBOBOXINFO info1;
        info1.cbSize = sizeof(info1);
        SendMessage(hwndComboBox, CB_GETCOMBOBOXINFO, 0, (LPARAM)&info1);

        if ((HWND)lParam == hwndListBox)
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
        HWND hWnd = (HWND)lParam;
        HDC dc = (HDC)wParam;
        if (hWnd == hwndListBox)
        {
            SetBkMode(dc, OPAQUE);
            SetTextColor(dc, RGB(0, 255, 0));
            SetBkColor(dc, RGB(0, 0, 0));                   // 0x383838
            HBRUSH comboBrush = CreateSolidBrush(0x383838); // global var
            return (LRESULT)comboBrush;
        }
    }
    // change scrollbar color to black with green arrows
    case WM_CTLCOLORSCROLLBAR:
    {
        HWND hWnd = (HWND)lParam;
        HDC dc = (HDC)wParam;
        if (hWnd == hwndListBox)
        {
            SetBkMode(dc, OPAQUE);
            SetTextColor(dc, RGB(0, 255, 0));
            SetBkColor(dc, RGB(0, 0, 0));                   // 0x383838
            HBRUSH comboBrush = CreateSolidBrush(0x383838); // global var
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

    ShowWindow(hwndListBox, SW_HIDE);
}

static void OnButtonPressCancel(Button *button)
{
    SetWindowSize(AVP2_MAIN_SCREEN_WIDTH, AVP2_MAIN_SCREEN_HEIGHT);

    g_nCurrentScreen = SCREEN_SPLASH;

    PlaySoundResource("BACK");

    ScreenUpdateLoop = pOldUpdateLoop;
    ScreenRenderLoop = pOldRenderLoop;

    

    ShowWindow(hwndListBox, SW_HIDE);
}

void DisplaySetupScreen(void *pRenderLoop, void *pUpdateLoop)
{
    // save old render and update loops
    pOldRenderLoop = pRenderLoop;
    pOldUpdateLoop = pUpdateLoop;

    // set new render and update loops so our main loop is happy
    ScreenRenderLoop = DisplayRenderScreen;
    ScreenUpdateLoop = DisplayUpdateLoop;

    SetProcessDpiAware();

    // get the monitors
    int monitorCount = 0;
    // monitors = (Monitor **)malloc(sizeof(Monitor *) * 1);

    g_hWnd = GetWindowHandle();

    if (!hwndListBox)
    {

        hwndListBox = CreateWindowEx(0, TEXT("LISTBOX"), TEXT(""),
                                     WS_CHILD | WS_OVERLAPPED | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY,
                                     20, 78, 126, 240, g_hWnd, NULL, NULL, NULL);

        // set font size to 14
        // SendMessage(hwndListBox, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));

        // set font
        HFONT hFont = CreateFontA(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
                                  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_MODERN, "Segoe UI");
        SendMessage(hwndListBox, WM_SETFONT, (WPARAM)hFont, TRUE);

        // set callback
        oldWndProc = (WNDPROC)SetWindowLongPtr(g_hWnd, GWLP_WNDPROC, (LONG_PTR)WndProc);

        GetModes();

        for (int i = 0; i < pMonitorArray[0]->modeCount; i++)
        {
            // build string width x height
            char *temp = (char *)malloc(256);
            sprintf(temp, "%d x %d", pMonitorArray[0]->modes[i]->width, pMonitorArray[0]->modes[i]->height);
            SendMessage(hwndListBox, (UINT)LB_ADDSTRING, (WPARAM)0, (LPARAM)temp);
        }

        //load library "d3d.ren"
        HMODULE hdll = LoadLibrary("d3d.ren");
        if (hdll == NULL)
        {
            MessageBox(NULL, "Failed to load d3d.ren!", "Error", MB_OK | MB_ICONERROR);
            exit(1);
        }

        //load string from resource in d3d.ren
        char *szString = (char *)malloc(256);
        if (szString == NULL)
        {
            MessageBox(NULL, "Failed to allocate memory for the window title.", "Error", MB_OK | MB_ICONERROR);
            exit(1);
        }

        LoadString(hdll, 5, szString, 256);

        MessageBox(NULL, szString, "Error", MB_OK | MB_ICONERROR);

        //free library
        FreeLibrary(hdll);
        free(szString);

    }
    else
    {
        // unhide the listbox
        ShowWindow(hwndListBox, SW_SHOW);
    }

    SetWindowSize(600, 394);

    // x button
    if (xButton.texture[0].id == 0)
    {
        LoadTextureFromResource(&xButton.texture[0], "CLOSEU");
        LoadTextureFromResource(&xButton.texture[1], "CLOSEF");
        LoadTextureFromResource(&xButton.texture[2], "CLOSED");
    }
    xButton.position = (Vector2){579, 1};
    xButton.onPress = OnButtonPressCancel;
    xButton.onUnload = UnloadButton;
    xButton.isEnabled = TRUE;

    // generic
    if (okButton.texture[0].id == 0)
    {
        LoadTextureFromResource(&okButton.texture[0], "OKU");
        LoadTextureFromResource(&okButton.texture[1], "OKF");
        LoadTextureFromResource(&okButton.texture[2], "OKD");
    }
    okButton.position = (Vector2){194, 349};
    okButton.onPress = OnButtonPressOK;
    okButton.onUnload = UnloadButton;
    okButton.isEnabled = TRUE;

    if (cancelButton.texture[0].id == 0)
    {
        LoadTextureFromResource(&cancelButton.texture[0], "CANCELU");
        LoadTextureFromResource(&cancelButton.texture[1], "CANCELF");
        LoadTextureFromResource(&cancelButton.texture[2], "CANCELD");
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
}

void DisplayRenderScreen()
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

static void CheckAllButtons()
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

void DisplayUpdateLoop()
{
    nCurrentToolTip = 0;
    CheckAllButtons();
}

static void GetModes()
{

    // enumerate display adapters
    int adapterCount = 0;
    int monitorCount = 0;
    int modeCount = 0;
    int adapterIndex = 0;
    int monitorIndex = 0;
    int modeIndex = 0;

    // get number of adapters using glfwGetMonitors
    GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);

    // initialize mon array and calloc
    pMonitorArray = calloc(monitorCount, sizeof(Monitor *));

    if (pMonitorArray == NULL)
    {
        TraceLog(LOG_ERROR, "Failed to allocate memory for monitors!");
        exit(1);
    }

    // get modes for each adapter
    for (monitorIndex = 0; monitorIndex < monitorCount; monitorIndex++)
    {
        pMonitorArray[monitorIndex] = calloc(1, sizeof(Monitor));

        if (pMonitorArray[monitorIndex] == NULL)
        {
            TraceLog(LOG_ERROR, "Failed to allocate memory for monitor!");
            exit(1);
        }

        const GLFWvidmode *modes = glfwGetVideoModes(monitors[monitorIndex], &modeCount);
        pMonitorArray[monitorIndex]->modeCount = modeCount;

        pMonitorArray[monitorIndex]->modes = calloc(modeCount, sizeof(Mode *));

        if (pMonitorArray[monitorIndex]->modes == NULL)
        {
            TraceLog(LOG_ERROR, "Failed to allocate memory for modes!");
            exit(1);
        }

        for (modeIndex = 0; modeIndex < modeCount; modeIndex++)
        {
            pMonitorArray[monitorIndex]->modes[modeIndex] = calloc(1, sizeof(Mode));

            if (pMonitorArray[monitorIndex]->modes[modeIndex] == NULL)
            {
                TraceLog(LOG_ERROR, "Failed to allocate memory for mode!");
                exit(1);
            }

            pMonitorArray[monitorIndex]->modes[modeIndex]->width = modes[modeIndex].width;
            pMonitorArray[monitorIndex]->modes[modeIndex]->height = modes[modeIndex].height;
            pMonitorArray[monitorIndex]->modes[modeIndex]->refreshRate = modes[modeIndex].refreshRate;

            TraceLog(LOG_INFO, "Adapter %i, Mode %i: %i x %i", monitorIndex, modeIndex, modes[modeIndex].width, modes[modeIndex].height);
        }

        // Remove duplicate and non-standard modes
        for (modeIndex = 0; modeIndex < pMonitorArray[monitorIndex]->modeCount; modeIndex++)
        {
            float aspectRatio = (float)pMonitorArray[monitorIndex]->modes[modeIndex]->width / (float)pMonitorArray[monitorIndex]->modes[modeIndex]->height;
            const float expectedAspectRatio[] = {4.0f / 3.0f, 16.0f / 9.0f, 21.5f / 9.0f, 16.0f / 10.0f};
            bool isExpectedAspectRatio = FALSE;

            for (size_t i = 0; i < sizeof(expectedAspectRatio) / sizeof(expectedAspectRatio[0]); i++)
            {
                if (fabs(aspectRatio - expectedAspectRatio[i]) < 0.001f)
                {
                    isExpectedAspectRatio = TRUE;
                    break;
                }
            }

            if (!isExpectedAspectRatio)
            {
                // Free the memory used by the removed mode
                free(pMonitorArray[monitorIndex]->modes[modeIndex]);
                pMonitorArray[monitorIndex]->modes[modeIndex] = NULL;

                // Shift the remaining modes to fill the gap
                memmove(&pMonitorArray[monitorIndex]->modes[modeIndex], &pMonitorArray[monitorIndex]->modes[modeIndex + 1], (pMonitorArray[monitorIndex]->modeCount - modeIndex - 1) * sizeof(Mode *));
                pMonitorArray[monitorIndex]->modeCount--;

                TraceLog(LOG_INFO, "Adapter %i, Mode %i: Removed non-standard mode", monitorIndex, modeIndex);
                modeIndex--;
            }
            else
            {
                for (size_t otherIndex = modeIndex + 1; otherIndex < pMonitorArray[monitorIndex]->modeCount; otherIndex++)
                {
                    if (pMonitorArray[monitorIndex]->modes[modeIndex]->width == pMonitorArray[monitorIndex]->modes[otherIndex]->width &&
                        pMonitorArray[monitorIndex]->modes[modeIndex]->height == pMonitorArray[monitorIndex]->modes[otherIndex]->height)
                    {
                        // Free the memory used by the removed mode
                        free(pMonitorArray[monitorIndex]->modes[otherIndex]);
                        pMonitorArray[monitorIndex]->modes[otherIndex] = NULL;

                        // Shift the remaining modes to fill the gap
                        memmove(&pMonitorArray[monitorIndex]->modes[otherIndex], &pMonitorArray[monitorIndex]->modes[otherIndex + 1], (pMonitorArray[monitorIndex]->modeCount - otherIndex - 1) * sizeof(Mode *));
                        pMonitorArray[monitorIndex]->modeCount--;

                        TraceLog(LOG_INFO, "Adapter %i, Mode %i: Removed duplicate mode", monitorIndex, otherIndex);
                        otherIndex--;
                    }
                }
            }
        }
    }

    TraceLog(LOG_INFO, "Number of monitors: %i", monitorCount);
}