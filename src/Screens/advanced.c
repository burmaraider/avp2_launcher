#include "raylib.h"
#include "raymath.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "..\include\types.h"
#include "..\include\registry.h"
#include "string.h"

#include <winuser.h>
#include <shellscalingapi.h>
#include <wingdi.h>
#include <richedit.h>
#include "..\include\loader.h"
#include "..\include\Screens\advanced.h"

//Win32 TextBox
HWND g_hWnd;
HWND hwndTextBox;
WNDPROC oldTextBoxProc;


#include <Windows.h>

typedef BOOL (WINAPI * SETPROCESSDPIAWARE_T)(void);
typedef HRESULT (WINAPI * SETPROCESSDPIAWARENESS_T)(PROCESS_DPI_AWARENESS);

bool win32_SetProcessDpiAware(void) {
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

void CheckboxPress(Checkbox *checkbox)
{
    checkbox->isChecked = !checkbox->isChecked;
    PlaySoundResource("OK");
}

void SaveSettingsToRegistry()
{

    DWORD nDisableHardwareCursor = (DWORD)g_Settings.nDisableHardwareCursor;
    DWORD nDisableJoysticks = (DWORD)g_Settings.nDisableJoysticks;
    DWORD nDisableMovies = (DWORD)g_Settings.nDisableMovies;
    DWORD nDisableMusic = (DWORD)g_Settings.nDisableMusic;
    DWORD nDisableSound = (DWORD)g_Settings.nDisableSound;
    DWORD nDisableTripleBuffering = (DWORD)g_Settings.nDisableTripleBuffering;
    DWORD nSaveCommands = (DWORD)g_Settings.nSaveCommands;
    
    SetRegistryValue(HKEY_CURRENT_USER, "Disable Hardware Cursor", REG_DWORD, &nDisableHardwareCursor, sizeof(DWORD));
    SetRegistryValue(HKEY_CURRENT_USER, "Disable Joysticks", REG_DWORD, &nDisableJoysticks, sizeof(DWORD));
    SetRegistryValue(HKEY_CURRENT_USER, "Disable Movies", REG_DWORD, &nDisableMovies, sizeof(DWORD));
    SetRegistryValue(HKEY_CURRENT_USER, "Disable Music", REG_DWORD, &nDisableMusic, sizeof(DWORD));
    SetRegistryValue(HKEY_CURRENT_USER, "Disable Sound", REG_DWORD, &nDisableSound, sizeof(DWORD));
    SetRegistryValue(HKEY_CURRENT_USER, "Disable Triple Buffering", REG_DWORD, &nDisableTripleBuffering, sizeof(DWORD));
    SetRegistryValue(HKEY_CURRENT_USER, "Save Commands", REG_DWORD, &nSaveCommands, sizeof(DWORD));
    SetRegistryValue(HKEY_CURRENT_USER, "Commands", REG_SZ, g_Settings.szCommands, strlen(g_Settings.szCommands));
}

void OKPress(Button* button)
{
    g_Settings.nSaveCommands = saveCommands.isChecked;
    g_Settings.nDisableSound = disableSound.isChecked;
    g_Settings.nDisableMusic = disableMusic.isChecked;
    g_Settings.nDisableMovies = disableMovies.isChecked;
    g_Settings.nDisableTripleBuffering = disableTripleBuffering.isChecked;
    g_Settings.nDisableJoysticks = disableJoystick.isChecked;
    g_Settings.nDisableHardwareCursor = disableHardwareCursor.isChecked;

    //hide hwndTextBox
    ShowWindow(hwndTextBox, SW_HIDE);
    GetWindowTextA(hwndTextBox, g_Settings.szCommands, 256);

    SaveSettingsToRegistry();

    //SaveSettings();
    currentScreen = 0;
    Screen = DefaultScreen;
    SetWindowSize(screenWidth, screenHeight);
    g_xButton.onPress = ButtonPress;
    g_xButton.position = (Vector2){ 505, 1 };
}

void Cancel(Button *button)
{
    currentScreen = 0;
    Screen = DefaultScreen;
    SetWindowSize(screenWidth, screenHeight);
    g_xButton.onPress = ButtonPress;
    g_xButton.position = (Vector2){ 505, 1 };

    //hide hwndTextBox
    ShowWindow(hwndTextBox, SW_HIDE);
}

void SetupCheckBoxesState()
{
    saveCommands.isChecked = g_Settings.nSaveCommands;
    disableSound.isChecked = g_Settings.nDisableSound;
    disableMusic.isChecked = g_Settings.nDisableMusic;
    disableMovies.isChecked = g_Settings.nDisableMovies;
    disableTripleBuffering.isChecked = g_Settings.nDisableTripleBuffering;
    disableJoystick.isChecked = g_Settings.nDisableJoysticks;
    disableHardwareCursor.isChecked = g_Settings.nDisableHardwareCursor;
}

void SetupScreenAdvanced()
{
    win32_SetProcessDpiAware();

    g_hWnd = GetWindowHandle();

    

    if(!hwndTextBox)
    {

        LoadLibrary(TEXT("Msftedit.dll"));
        //create textbox
        hwndTextBox = CreateWindowExA(0, TEXT("RICHEDIT50W"), TEXT("Type here"),
            WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL | ES_WANTRETURN,
            26, 220, 400, 18, g_hWnd, NULL, GetModuleHandle(NULL), NULL);
        //create a style for the textbox
        SetWindowLong(hwndTextBox, GWL_EXSTYLE, GetWindowLong(hwndTextBox, GWL_STYLE) | ES_AUTOHSCROLL);

        SendMessage(hwndTextBox, EM_SETBKGNDCOLOR, 0, RGB(0, 0, 0));

        //get dpi scale factor
        HDC hdc = GetDC(hwndTextBox);
        int dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
        int dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
        ReleaseDC(hwndTextBox, hdc);

        //set font
        HFONT hFont = CreateFontA(-MulDiv(8, dpiX, 72), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Tahoma");
        SendMessage(hwndTextBox, WM_SETFONT, (WPARAM)hFont, TRUE);


        //change text color
        CHARFORMAT2 charFormat;
        ZeroMemory(&charFormat, sizeof(charFormat));
        charFormat.cbSize = sizeof(charFormat);
        charFormat.dwMask = CFM_COLOR;
        charFormat.crTextColor = RGB(0, 255, 0);
        SendMessage(hwndTextBox, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&charFormat);
        
    }
    else
    {
        ShowWindow(hwndTextBox, SW_SHOW);
    }

    //set the textbox text
    SendMessage(hwndTextBox, WM_SETTEXT, 0, (LPARAM)g_Settings.szCommands);

    //scale text in text boss because DPI scaling
    SendMessage(hwndTextBox, EM_SETZOOM, 0, 0);



    SetWindowSize(456, 431);

    g_okButton.position = (Vector2){ 123, 386 };
    g_okButton.onPress = OKPress;

    g_cancelButton.position = (Vector2){ 235, 386 };
    g_xButton.position = (Vector2){ 435, 1 };


    //setup checkboxes
    saveCommands.position = (Vector2){ 26, 246 };
    saveCommands.isChecked = g_Settings.nSaveCommands;
    LoadTextureFromResource(&saveCommands.texture[0], "ALWAYSSPECIFYN");
    LoadTextureFromResource(&saveCommands.texture[1], "ALWAYSSPECIFYC");
    saveCommands.onPress = CheckboxPress;

    disableSound.position = (Vector2){ 26, 68 };
    disableSound.isChecked = g_Settings.nDisableSound;
    LoadTextureFromResource(&disableSound.texture[0], "DISABLESOUNDN");
    LoadTextureFromResource(&disableSound.texture[1], "DISABLESOUNDC");
    disableSound.onPress = CheckboxPress;

    disableMusic.position = (Vector2){ 26, 88 };
    disableMusic.isChecked = g_Settings.nDisableMusic;
    LoadTextureFromResource(&disableMusic.texture[0], "DISABLEMUSICN");
    LoadTextureFromResource(&disableMusic.texture[1], "DISABLEMUSICC");
    disableMusic.onPress = CheckboxPress;

    disableMovies.position = (Vector2){ 26, 108 };
    disableMovies.isChecked = g_Settings.nDisableMovies;
    LoadTextureFromResource(&disableMovies.texture[0], "DISABLEMOVIESN");
    LoadTextureFromResource(&disableMovies.texture[1], "DISABLEMOVIESC");
    disableMovies.onPress = CheckboxPress;

    disableTripleBuffering.position = (Vector2){ 228, 68 };
    disableTripleBuffering.isChecked = g_Settings.nDisableTripleBuffering;
    LoadTextureFromResource(&disableTripleBuffering.texture[0], "DISABLETRIPLEBUFFERINGN");
    LoadTextureFromResource(&disableTripleBuffering.texture[1], "DISABLETRIPLEBUFFERINGC");
    disableTripleBuffering.onPress = CheckboxPress;

    disableJoystick.position = (Vector2){ 228, 88 };
    disableJoystick.isChecked = g_Settings.nDisableJoysticks;
    LoadTextureFromResource(&disableJoystick.texture[0], "DISABLEJOYSTICKSN");
    LoadTextureFromResource(&disableJoystick.texture[1], "DISABLEJOYSTICKSC");
    disableJoystick.onPress = CheckboxPress;

    disableHardwareCursor.position = (Vector2){ 228, 108 };
    disableHardwareCursor.isChecked = g_Settings.nDisableHardwareCursor;
    LoadTextureFromResource(&disableHardwareCursor.texture[0], "DISABLEHARDWARECURSORN");
    LoadTextureFromResource(&disableHardwareCursor.texture[1], "DISABLEHARDWARECURSORC");
    disableHardwareCursor.onPress = CheckboxPress;


    checkboxesAdvanced = (Checkbox **)malloc(sizeof(Checkbox *) * CHECK_COUNT);
    //check if malloc failed
    if (!checkboxesAdvanced)
    {
        MessageBox(NULL, "Failed to allocate memory", "Error", MB_OK | MB_ICONERROR);
        exit(1);
    }

    for (size_t i = 0; i < CHECK_COUNT; i++)
    {
        checkboxesAdvanced[i] = (Checkbox *)malloc(sizeof(Checkbox));
        //check if malloc failed
        if (!checkboxesAdvanced[i])
        {
            MessageBox(NULL, "Failed to allocate memory", "Error", MB_OK | MB_ICONERROR);
            exit(1);
        }
    }

    checkboxesAdvanced[0] = &saveCommands;
    checkboxesAdvanced[1] = &disableSound;
    checkboxesAdvanced[2] = &disableMusic;
    checkboxesAdvanced[3] = &disableMovies;
    checkboxesAdvanced[4] = &disableJoystick;
    checkboxesAdvanced[5] = &disableHardwareCursor;
    checkboxesAdvanced[6] = &disableTripleBuffering;

    SetupCheckBoxesState();

}

void SetCallbacksAdvancedScreen(Button *button)
{
    button->onPress = Cancel;
}

void RenderAdvancedScreen()
{
    DrawTexture(g_backgroundImage[currentScreen], 0, 0, WHITE);

    DrawTexture(g_okButton.texture[g_okButton.currentTexture], g_okButton.position.x, g_okButton.position.y, WHITE);
    DrawTexture(g_cancelButton.texture[g_cancelButton.currentTexture], g_cancelButton.position.x, g_cancelButton.position.y, WHITE);

    DrawTexture(g_xButton.texture[g_xButton.currentTexture], g_xButton.position.x, g_xButton.position.y, WHITE);

    //Draw checkboxes
    DrawTexture(saveCommands.texture[saveCommands.isChecked], saveCommands.position.x, saveCommands.position.y, WHITE);
    DrawTexture(disableSound.texture[disableSound.isChecked], disableSound.position.x, disableSound.position.y, WHITE);
    DrawTexture(disableMusic.texture[disableMusic.isChecked], disableMusic.position.x, disableMusic.position.y, WHITE);
    DrawTexture(disableMovies.texture[disableMovies.isChecked], disableMovies.position.x, disableMovies.position.y, WHITE);
    DrawTexture(disableTripleBuffering.texture[disableTripleBuffering.isChecked], disableTripleBuffering.position.x, disableTripleBuffering.position.y, WHITE);
    DrawTexture(disableJoystick.texture[disableJoystick.isChecked], disableJoystick.position.x, disableJoystick.position.y, WHITE);
    DrawTexture(disableHardwareCursor.texture[disableHardwareCursor.isChecked], disableHardwareCursor.position.x, disableHardwareCursor.position.y, WHITE);
}


void CheckAllCheckBoxes()
{
    for (size_t i = 0; i < CHECK_COUNT; i++)
    {
        if (CheckCollisionPointRec(GetMousePosition(), (Rect){checkboxesAdvanced[i]->position.x, checkboxesAdvanced[i]->position.y, checkboxesAdvanced[i]->texture[0].width, checkboxesAdvanced[i]->texture[0].height}))
        {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
            {
                checkboxesAdvanced[i]->currentTexture = 1;
            }

            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
            {
                checkboxesAdvanced[i]->currentTexture = 1;
                checkboxesAdvanced[i]->onPress(checkboxesAdvanced[i]);
            }
        }
        else
        {
            g_buttons[i]->currentTexture = UP;
        }
    }
}