#include "raylib.h"
#include <string.h>

//Windows Headers
#include <richedit.h>

//Program Headers
#include "..\include\registry.h"
#include "..\include\utils.h"
#include "..\include\Screens\advanced.h"

//Win32 Stuff
static HWND g_hWnd;
static HWND hwndTextBox;

//CHECKBOXES
static Checkbox saveCommands;
static Checkbox disableMovies;
static Checkbox disableSound;
static Checkbox disableMusic;
static Checkbox disableMovies;
static Checkbox disableJoystick;
static Checkbox disableFog;
static Checkbox disableHardwareCursor;
static Checkbox disableTripleBuffering;
static Checkbox restoreDefaultSettings;
static Checkbox** checkboxesAdvanced;

//BUTTONS
static Button xButton;
static Button okButton;
static Button cancelButton;
static Button **buttons;

//LOCAL VARIABLES
static uint32_t nCurrentToolTip = 0;

// Function pointers for the original render and update loops
static void *pOldRenderLoop;
static void *pOldUpdateLoop;

static void CheckBoxHover(Checkbox *checkbox)
{
    nCurrentToolTip = checkbox->id;
}

static void CheckboxPress(Checkbox *checkbox)
{
    checkbox->isChecked = !checkbox->isChecked;
    PlaySoundResource("OK");
}

static void SaveSettingsToRegistry()
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

static void OnButtonPressOK(Button* button)
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
    g_nCurrentScreen = 0;
    SetWindowSize(AVP2_MAIN_SCREEN_WIDTH, AVP2_MAIN_SCREEN_HEIGHT);
    
    //free textures
    for (size_t i = 0; i < CHECK_COUNT; i++)
    {
        UnloadCheckBox(checkboxesAdvanced[i]);
    }

    PlaySoundResource("OK");

    ScreenUpdateLoop = pOldUpdateLoop;
    ScreenRenderLoop = pOldRenderLoop;
}

static void OnButtonPressCancel(Button *button)
{
    g_nCurrentScreen = 0;
    SetWindowSize(AVP2_MAIN_SCREEN_WIDTH, AVP2_MAIN_SCREEN_HEIGHT);
    xButton.onPress = ButtonPressCallback;
    xButton.position = (Vector2){ 505, 1 };

    //hide hwndTextBox
    ShowWindow(hwndTextBox, SW_HIDE);

    //free textures
    for (size_t i = 0; i < CHECK_COUNT; i++)
    {
        UnloadCheckBox(checkboxesAdvanced[i]);
    }
    
    PlaySoundResource("BACK");

    ScreenUpdateLoop = pOldUpdateLoop;
    ScreenRenderLoop = pOldRenderLoop;
}

static void SetupCheckBoxesState()
{
    saveCommands.isChecked = g_Settings.nSaveCommands;
    disableSound.isChecked = g_Settings.nDisableSound;
    disableMusic.isChecked = g_Settings.nDisableMusic;
    disableMovies.isChecked = g_Settings.nDisableMovies;
    disableTripleBuffering.isChecked = g_Settings.nDisableTripleBuffering;
    disableJoystick.isChecked = g_Settings.nDisableJoysticks;
    disableHardwareCursor.isChecked = g_Settings.nDisableHardwareCursor;
}

void OptionsSetupScreen(void *pRenderLoop, void *pUpdateLoop)
{
    //save old render and update loops
    pOldRenderLoop = pRenderLoop;
    pOldUpdateLoop = pUpdateLoop;

    //set new render and update loops so our main loop is happy
    ScreenRenderLoop = OptionsRenderScreen;
    ScreenUpdateLoop = OptionsUpdateLoop;

    SetProcessDpiAware();

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
        int dpiXScale = GetDeviceCaps(hdc, LOGPIXELSX) / USER_DEFAULT_SCREEN_DPI;
        int dpiYScale = GetDeviceCaps(hdc, LOGPIXELSY) / USER_DEFAULT_SCREEN_DPI;
        ReleaseDC(hwndTextBox, hdc);

        //set font
        HFONT hFont = CreateFontA(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_MODERN, "Tahoma");
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

    //setup checkboxes
    saveCommands.position = (Vector2){ 26, 246 };
    saveCommands.isChecked = g_Settings.nSaveCommands;
    LoadTextureFromResource(&saveCommands.texture[0], "ALWAYSSPECIFYN");
    LoadTextureFromResource(&saveCommands.texture[1], "ALWAYSSPECIFYC");
    saveCommands.onPress = CheckboxPress;
    saveCommands.onHover = CheckBoxHover;
    saveCommands.id = 1;

    disableSound.position = (Vector2){ 26, 68 };
    disableSound.isChecked = g_Settings.nDisableSound;
    LoadTextureFromResource(&disableSound.texture[0], "DISABLESOUNDN");
    LoadTextureFromResource(&disableSound.texture[1], "DISABLESOUNDC");
    disableSound.onPress = CheckboxPress;
    disableSound.onHover = CheckBoxHover;
    disableSound.id = 2;

    disableMusic.position = (Vector2){ 26, 88 };
    disableMusic.isChecked = g_Settings.nDisableMusic;
    LoadTextureFromResource(&disableMusic.texture[0], "DISABLEMUSICN");
    LoadTextureFromResource(&disableMusic.texture[1], "DISABLEMUSICC");
    disableMusic.onPress = CheckboxPress;
    disableMusic.onHover = CheckBoxHover;
    disableMusic.id = 3;

    disableMovies.position = (Vector2){ 26, 108 };
    disableMovies.isChecked = g_Settings.nDisableMovies;
    LoadTextureFromResource(&disableMovies.texture[0], "DISABLEMOVIESN");
    LoadTextureFromResource(&disableMovies.texture[1], "DISABLEMOVIESC");
    disableMovies.onPress = CheckboxPress;
    disableMovies.onHover = CheckBoxHover;
    disableMovies.id = 4;

    disableTripleBuffering.position = (Vector2){ 228, 68 };
    disableTripleBuffering.isChecked = g_Settings.nDisableTripleBuffering;
    LoadTextureFromResource(&disableTripleBuffering.texture[0], "DISABLETRIPLEBUFFERINGN");
    LoadTextureFromResource(&disableTripleBuffering.texture[1], "DISABLETRIPLEBUFFERINGC");
    disableTripleBuffering.onPress = CheckboxPress;
    disableTripleBuffering.onHover = CheckBoxHover;
    disableTripleBuffering.id = 5;

    disableJoystick.position = (Vector2){ 228, 88 };
    disableJoystick.isChecked = g_Settings.nDisableJoysticks;
    LoadTextureFromResource(&disableJoystick.texture[0], "DISABLEJOYSTICKSN");
    LoadTextureFromResource(&disableJoystick.texture[1], "DISABLEJOYSTICKSC");
    disableJoystick.onPress = CheckboxPress;
    disableJoystick.onHover = CheckBoxHover;
    disableJoystick.id = 6;

    disableHardwareCursor.position = (Vector2){ 228, 108 };
    disableHardwareCursor.isChecked = g_Settings.nDisableHardwareCursor;
    LoadTextureFromResource(&disableHardwareCursor.texture[0], "DISABLEHARDWARECURSORN");
    LoadTextureFromResource(&disableHardwareCursor.texture[1], "DISABLEHARDWARECURSORC");
    disableHardwareCursor.onPress = CheckboxPress;
    disableHardwareCursor.onHover = CheckBoxHover;
    disableHardwareCursor.id = 7;

    checkboxesAdvanced = (Checkbox **)malloc(sizeof(Checkbox *) * CHECK_COUNT);

    if (!checkboxesAdvanced)
    {
        MessageBox(NULL, "Failed to allocate memory", "Error", MB_OK | MB_ICONERROR);
        exit(1);
    }

    for (size_t i = 0; i < CHECK_COUNT; i++)
    {
        checkboxesAdvanced[i] = (Checkbox *)malloc(sizeof(Checkbox));

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

    // x button
    LoadTextureFromResource(&xButton.texture[0], "CLOSEU");
    LoadTextureFromResource(&xButton.texture[1], "CLOSEF");
    LoadTextureFromResource(&xButton.texture[2], "CLOSED");
    xButton.position = (Vector2){505, 1};
    xButton.onPress = ButtonPressCallback;
    xButton.onUnload = UnloadButton;
    xButton.isEnabled = TRUE;
    strcpy(xButton.szName, "x");

     // generic
    LoadTextureFromResource(&okButton.texture[0], "OKU");
    LoadTextureFromResource(&okButton.texture[1], "OKF");
    LoadTextureFromResource(&okButton.texture[2], "OKD");
    okButton.position = (Vector2){ 123, 386 };
    okButton.onPress = OnButtonPressOK;
    okButton.onUnload = UnloadButton;
    okButton.isEnabled = TRUE;
    strcpy(okButton.szName, "ok");

    LoadTextureFromResource(&cancelButton.texture[0], "CANCELU");
    LoadTextureFromResource(&cancelButton.texture[1], "CANCELF");
    LoadTextureFromResource(&cancelButton.texture[2], "CANCELD");
    cancelButton.position = (Vector2){ 235, 386 };
    cancelButton.onPress = OnButtonPressCancel;
    cancelButton.onUnload = UnloadButton;
    cancelButton.isEnabled = TRUE;
    strcpy(cancelButton.szName, "cancel");

    //seTUP BUTTONS
    buttons = (Button **)malloc(sizeof(Button *) * 3);

    if(!buttons)
    {
        MessageBox(NULL, "Failed to allocate memory for buttons!", "Error", MB_OK | MB_ICONERROR);
        exit(1);
    }

    for (size_t i = 0; i < 3; i++)
    {
        buttons[i] = (Button *)malloc(sizeof(Button));
        
        if(!buttons[i])
        {
            MessageBox(NULL, "Failed to allocate memory for buttons!", "Error", MB_OK | MB_ICONERROR);
            exit(1);
        }
    }

    buttons[0] = &okButton;
    buttons[1] = &cancelButton;
    buttons[2] = &xButton;

}


void OptionsRenderScreen()
{
    DrawTexture(g_backgroundImage[g_nCurrentScreen], 0, 0, WHITE);

    DrawTexture(okButton.texture[okButton.currentTexture], okButton.position.x, okButton.position.y, WHITE);
    DrawTexture(cancelButton.texture[cancelButton.currentTexture], cancelButton.position.x, cancelButton.position.y, WHITE);
    DrawTexture(xButton.texture[xButton.currentTexture], xButton.position.x, xButton.position.y, WHITE);

    //Draw checkboxes
    DrawTexture(saveCommands.texture[saveCommands.isChecked], saveCommands.position.x, saveCommands.position.y, WHITE);
    DrawTexture(disableSound.texture[disableSound.isChecked], disableSound.position.x, disableSound.position.y, WHITE);
    DrawTexture(disableMusic.texture[disableMusic.isChecked], disableMusic.position.x, disableMusic.position.y, WHITE);
    DrawTexture(disableMovies.texture[disableMovies.isChecked], disableMovies.position.x, disableMovies.position.y, WHITE);
    DrawTexture(disableTripleBuffering.texture[disableTripleBuffering.isChecked], disableTripleBuffering.position.x, disableTripleBuffering.position.y, WHITE);
    DrawTexture(disableJoystick.texture[disableJoystick.isChecked], disableJoystick.position.x, disableJoystick.position.y, WHITE);
    DrawTexture(disableHardwareCursor.texture[disableHardwareCursor.isChecked], disableHardwareCursor.position.x, disableHardwareCursor.position.y, WHITE);

    if(nCurrentToolTip > 0)
    {
        char* temp = FormatStringWithNewLines(AVP2_TOOLTIPS[nCurrentToolTip], (Rect){26, 290, 400, 300});

        DrawTextExRay(g_font, temp, (Vector2){26, 290}, 18, 1, WHITE);

        free(temp);
    }

}


static void CheckAllCheckBoxes()
{
    for (size_t i = 0; i < CHECK_COUNT; i++)
    {
        if (CheckCollisionPointRec(GetMousePosition(), (Rect){checkboxesAdvanced[i]->position.x, checkboxesAdvanced[i]->position.y, checkboxesAdvanced[i]->texture[0].width, checkboxesAdvanced[i]->texture[0].height}))
        {
            checkboxesAdvanced[i]->onHover(checkboxesAdvanced[i]);
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
            checkboxesAdvanced[i]->currentTexture = UP;
        }
    }
}

static void CheckAllButtons()
{
    for (size_t i = 0; i < ADVANCED_BUTTON_COUNT; i++)
    {
        if (CheckCollisionPointRec(GetMousePosition(), (Rect){buttons[i]->position.x, buttons[i]->position.y, buttons[i]->texture[0].width, buttons[i]->texture[0].height})
        && buttons[i]->isEnabled == TRUE)
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

void OptionsUpdateLoop()
{
    nCurrentToolTip = 0;
    CheckAllButtons();
    CheckAllCheckBoxes();
}