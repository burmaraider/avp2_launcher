#include "raylib.h"
#include "raymath.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "include\types.h"
#include "include\registry.h"
#include "string.h"

#include <minwindef.h>
#include <winbase.h>
#include <winsmcrd.h>
#include <apisetcconv.h>
#include <winreg.h>
#include <mmsystem.h>
#include <winuser.h>
#include <time.h>
#include <wingdi.h>

// #define GLFW_EXPOSE_NATIVE_WIN32
// #define GLFW_EXPOSE_NATIVE_WGL
// #define GLFW_NATIVE_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "include\Screens\advanced.h"



/// @brief The registry keys and values used by the game.
/// @details The game uses the registry to store settings and the install directory.
/// @note The game uses the registry to store settings and the install directory.
char *g_pszCommandLine;
char *g_pszInstallDir;
bool g_bAVP2Installed;
LauncherSettings g_Settings;

int screenWidth = 525;
int screenHeight = 245;

Monitor **mon;

void InitGame(void);
void GameLoop(void); // Update and Draw one frame
void (*Screen)(void);
void DefaultScreen(void);
void Inputs(void);

void GetModes();
void FreeModes();
bool showModes = FALSE;
bool bShouldClose = FALSE;

void PlayRandomIntroSound();
void LoadBackgroundImages();
void LoadAllResources();
void LoadTextureFromResource(Texture *texture, const char *name);

int currentScreen = 0;
Font g_font;
Texture backgroundImage[6];

Button playButton;
Button serverButton;
Button displayButton;
Button optionsButton;
Button exitButton;
Button minimizeButton;
Button xButton;
Button okButton;
Button cancelButton;
Button installButton;

Button **buttons;
Checkbox **checkboxes;

// predator animation
unsigned int predatorFrame = 0;
Texture predator;
float countdown = 1.0f; // 10 seconds countdown

void ButtonPress(Button *button)
{
    PlaySoundResource("OK");

    if (strcmp(button->szName, "play") == 0)
    {
        currentScreen = 1;
    }
    else if (strcmp(button->szName, "server") == 0)
    {
    }
    else if (strcmp(button->szName, "display") == 0)
    {

        currentScreen = SCREEN_DISPLAY;
    }
    else if (strcmp(button->szName, "options") == 0)
    {

        currentScreen = SCREEN_OPTIONS;
        Screen = RenderAdvancedScreen;
        SetupScreenAdvanced();
        SetCallbacksAdvancedScreen(&cancelButton);
        SetCallbacksAdvancedScreen(&xButton);
    }
    else if (strcmp(button->szName, "exit") == 0)
    {

        bShouldClose = TRUE;
    }
    else if (strcmp(button->szName, "minimize") == 0)
    {
        MinimizeWindow();
    }
    else if (strcmp(button->szName, "x") == 0)
    {

        bShouldClose = TRUE;
    }
    else if(strcmp(button->szName, "install") == 0)
    {
        InstallAVP2Registry();
    }

    
}

int main(int argc, char *argv[])
{

    if (argc > 1)
    {
        if (strcmp(argv[1], "-install") == 0)
        {
            // start server
            InstallAVP2Registry();
            //relaunch the launcher as non admin
            char szPath[MAX_PATH];
            GetModuleFileName(NULL, szPath, MAX_PATH);
            ShellExecute(NULL, "open", szPath, NULL, NULL, SW_SHOWNORMAL);
            return 0;
        }
    }

    GetRegistryEntries();



    // Initialization
    //--------------------------------------------------------------------------------------
    // set vsync
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_UNDECORATED);


    
    //load string resource with LoadStringA
    char szTitle[256];

    LoadString(GetModuleHandle(NULL), IDS_APPNAME, szTitle, 256);
    InitWindow(screenWidth, screenHeight, szTitle);

    //Set the window icon
    HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APPICON));
    HWND hwnd = GetWindowHandle();

    SendMessage(hwnd, WM_SETICON, ICON_SMALL, hIcon);
    SendMessage(hwnd, WM_SETICON, ICON_BIG, hIcon);

    Screen = DefaultScreen;
    g_font = LoadFontEx("C:\\Windows\\Fonts\\segoeui.ttf", 16, 0, 250);
    SetTextureFilter(g_font.texture, TEXTURE_FILTER_POINT); // Texture scale filter to use

    LoadBackgroundImages();
    GetModes();

    // play button
    LoadTextureFromResource(&playButton.texture[0], "PLAYU");
    LoadTextureFromResource(&playButton.texture[1], "PLAYF");
    LoadTextureFromResource(&playButton.texture[2], "PLAYD");
    playButton.position = (Vector2){417, 19};
    playButton.onPress = ButtonPress;
    playButton.onUnload = UnloadButton;
    playButton.isEnabled = TRUE;
    strcpy(playButton.szName, "play");

    // server button
    LoadTextureFromResource(&serverButton.texture[0], "SERVERU");
    LoadTextureFromResource(&serverButton.texture[1], "SERVERF");
    LoadTextureFromResource(&serverButton.texture[2], "SERVERD");
    serverButton.position = (Vector2){417, 55};
    serverButton.onPress = ButtonPress;
    serverButton.onUnload = UnloadButton;
    serverButton.isEnabled = TRUE;
    strcpy(serverButton.szName, "server");

    // display button
    LoadTextureFromResource(&displayButton.texture[0], "DISPLAYU");
    LoadTextureFromResource(&displayButton.texture[1], "DISPLAYF");
    LoadTextureFromResource(&displayButton.texture[2], "DISPLAYD");
    displayButton.position = (Vector2){417, 91};
    displayButton.onPress = ButtonPress;
    displayButton.onUnload = UnloadButton;
    displayButton.isEnabled = TRUE;
    strcpy(displayButton.szName, "display");

    // options button
    LoadTextureFromResource(&optionsButton.texture[0], "OPTIONSU");
    LoadTextureFromResource(&optionsButton.texture[1], "OPTIONSF");
    LoadTextureFromResource(&optionsButton.texture[2], "OPTIONSD");
    optionsButton.position = (Vector2){417, 127};
    optionsButton.onPress = ButtonPress;
    optionsButton.onUnload = UnloadButton;
    optionsButton.isEnabled = TRUE;
    strcpy(optionsButton.szName, "options");

    // exit button
    LoadTextureFromResource(&exitButton.texture[0], "QUITU");
    LoadTextureFromResource(&exitButton.texture[1], "QUITF");
    LoadTextureFromResource(&exitButton.texture[2], "QUITD");
    exitButton.position = (Vector2){417, 199};
    exitButton.onPress = ButtonPress;
    exitButton.onUnload = UnloadButton;
    exitButton.isEnabled = TRUE;
    strcpy(exitButton.szName, "exit");

    // minimize button
    LoadTextureFromResource(&minimizeButton.texture[0], "MINIMIZEU");
    LoadTextureFromResource(&minimizeButton.texture[1], "MINIMIZEF");
    LoadTextureFromResource(&minimizeButton.texture[2], "MINIMIZED");
    minimizeButton.position = (Vector2){485, 1};
    minimizeButton.onPress = ButtonPress;
    minimizeButton.onUnload = UnloadButton;
    minimizeButton.isEnabled = TRUE;
    strcpy(minimizeButton.szName, "minimize");

    // x button
    LoadTextureFromResource(&xButton.texture[0], "CLOSEU");
    LoadTextureFromResource(&xButton.texture[1], "CLOSEF");
    LoadTextureFromResource(&xButton.texture[2], "CLOSED");
    xButton.position = (Vector2){505, 1};
    xButton.onPress = ButtonPress;
    xButton.onUnload = UnloadButton;
    xButton.isEnabled = TRUE;
    strcpy(xButton.szName, "x");

    // generic
    LoadTextureFromResource(&okButton.texture[0], "OKU");
    LoadTextureFromResource(&okButton.texture[1], "OKF");
    LoadTextureFromResource(&okButton.texture[2], "OKD");
    okButton.position = (Vector2){0, 0};
    okButton.onPress = ButtonPress;
    okButton.onUnload = UnloadButton;
    okButton.isEnabled = TRUE;
    strcpy(okButton.szName, "ok");

    LoadTextureFromResource(&cancelButton.texture[0], "CANCELU");
    LoadTextureFromResource(&cancelButton.texture[1], "CANCELF");
    LoadTextureFromResource(&cancelButton.texture[2], "CANCELD");
    cancelButton.position = (Vector2){0, 0};
    cancelButton.onPress = ButtonPress;
    cancelButton.onUnload = UnloadButton;
    cancelButton.isEnabled = TRUE;
    strcpy(cancelButton.szName, "cancel");

    LoadTextureFromResource(&installButton.texture[0], "INSTALLU");
    LoadTextureFromResource(&installButton.texture[1], "INSTALLF");
    LoadTextureFromResource(&installButton.texture[2], "INSTALLD");
    installButton.position = (Vector2){417, 19};
    installButton.onPress = ButtonPress;
    installButton.onUnload = UnloadButton;
    strcpy(installButton.szName, "install");


    //InstallAVP2Registry

    buttons = (Button **)malloc(sizeof(Button *) * BUTTON_COUNT);
    for (size_t i = 0; i < BUTTON_COUNT; i++)
    {
        buttons[i] = (Button *)malloc(sizeof(Button));
    }

    buttons[0] = &playButton;
    buttons[1] = &serverButton;
    buttons[2] = &displayButton;
    buttons[3] = &optionsButton;
    buttons[4] = &exitButton;
    buttons[5] = &minimizeButton;
    buttons[6] = &xButton;
    buttons[7] = &okButton;
    buttons[8] = &cancelButton;
    buttons[9] = &installButton;

    if(g_bAVP2Installed)
    {
        installButton.isEnabled = FALSE;
    }
    else
    {
        installButton.isEnabled = TRUE;
        playButton.isEnabled = FALSE;
        serverButton.isEnabled = FALSE;
        displayButton.isEnabled = FALSE;
        optionsButton.isEnabled = FALSE;
    }

    PlayRandomIntroSound();

    while (!bShouldClose)
    {
        GameLoop();
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(backgroundImage[0]);
    UnloadTexture(backgroundImage[1]);
    UnloadTexture(backgroundImage[2]);
    UnloadTexture(backgroundImage[3]);
    UnloadTexture(backgroundImage[4]);
    UnloadTexture(backgroundImage[5]);

    playButton.onUnload(&playButton);
    serverButton.onUnload(&serverButton);
    displayButton.onUnload(&displayButton);
    optionsButton.onUnload(&optionsButton);
    exitButton.onUnload(&exitButton);
    minimizeButton.onUnload(&minimizeButton);
    xButton.onUnload(&xButton);
    okButton.onUnload(&okButton);
    cancelButton.onUnload(&cancelButton);
    installButton.onUnload(&installButton);

    UnloadFont(g_font);

    FreeRegistryEntries();
    FreeModes();

    for (size_t i = 0; i < BUTTON_COUNT; i++)
    {
        buttons[i] = NULL;
        free(buttons[i]);
        // free(buttons);
    }

    ExitWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    return 0;
}

void GameLoop(void)
{
    Inputs();

    BeginDrawing();
    ClearBackground(BLACK);

    Screen();

    EndDrawing();

    float deltaTime = GetFrameTime();
    countdown -= deltaTime;
    if (countdown <= 0.0f)
    {
        countdown = 1.0f;                         // Reset countdown
        predatorFrame = (predatorFrame + 1) % 25; // Cycle predator frame
    }
}

void DefaultScreen()
{
    // draw background
    DrawTexture(backgroundImage[currentScreen], 0, 0, WHITE);

    DrawTexturePro(backgroundImage[5], (Rect){predatorFrame * (backgroundImage[5].width / 24), 0, backgroundImage[5].width / 24, backgroundImage[5].height}, (Rect){387, 18, backgroundImage[5].width / 24, backgroundImage[5].height}, (Vector2){0, 0}, 0, WHITE);

    if(g_bAVP2Installed)
    {
        DrawTexture(playButton.texture[playButton.currentTexture], playButton.position.x, playButton.position.y, WHITE);
        DrawTexture(serverButton.texture[serverButton.currentTexture], serverButton.position.x, serverButton.position.y, WHITE);
        DrawTexture(displayButton.texture[displayButton.currentTexture], displayButton.position.x, displayButton.position.y, WHITE);
        DrawTexture(optionsButton.texture[optionsButton.currentTexture], optionsButton.position.x, optionsButton.position.y, WHITE);
    }
    else
    {
        DrawTexture(installButton.texture[installButton.currentTexture], installButton.position.x, installButton.position.y, WHITE);
    }

    DrawTexture(exitButton.texture[exitButton.currentTexture], exitButton.position.x, exitButton.position.y, WHITE);
    DrawTexture(minimizeButton.texture[minimizeButton.currentTexture], minimizeButton.position.x, minimizeButton.position.y, WHITE);
    DrawTexture(xButton.texture[xButton.currentTexture], xButton.position.x, xButton.position.y, WHITE);

    // if (showModes)
    // {
    //     // Draw modes
    //     for (int monitorIndex = 0; monitorIndex < 1; monitorIndex++)
    //     {
    //         for (int modeIndex = 0; modeIndex < mon[monitorIndex]->modeCount; modeIndex++)
    //         {
    //             char buffer[256];
    //             sprintf(buffer, "%d x %d", mon[monitorIndex]->modes[modeIndex]->width, mon[monitorIndex]->modes[modeIndex]->height);
    //             DrawTextEx(fontTahoma, buffer, (Vector2){0, modeIndex * 12 + 1}, 12, 0, WHITE);
    //         }
    //     }
    // }
}

void DragWindow()
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

void CheckAllButtons()
{
    for (size_t i = 0; i < 10; i++)
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

void Inputs()
{
    DragWindow();
    CheckAllButtons();

    if(currentScreen == SCREEN_OPTIONS)
    {

        CheckAllCheckBoxes();
    }
}

void GetModes()
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
    mon = calloc(monitorCount, sizeof(Monitor *));

    // get modes for each adapter
    for (monitorIndex = 0; monitorIndex < monitorCount; monitorIndex++)
    {
        mon[monitorIndex] = calloc(1, sizeof(Monitor));

        const GLFWvidmode *modes = glfwGetVideoModes(monitors[monitorIndex], &modeCount);
        mon[monitorIndex]->modeCount = modeCount;

        mon[monitorIndex]->modes = calloc(modeCount, sizeof(Mode *));

        for (modeIndex = 0; modeIndex < modeCount; modeIndex++)
        {
            mon[monitorIndex]->modes[modeIndex] = calloc(1, sizeof(Mode));

            mon[monitorIndex]->modes[modeIndex]->width = modes[modeIndex].width;
            mon[monitorIndex]->modes[modeIndex]->height = modes[modeIndex].height;
            mon[monitorIndex]->modes[modeIndex]->refreshRate = modes[modeIndex].refreshRate;

            TraceLog(LOG_INFO, "Adapter %i, Mode %i: %i x %i", monitorIndex, modeIndex, modes[modeIndex].width, modes[modeIndex].height);
        }

        // Remove duplicate and non-standard modes
        for (modeIndex = 0; modeIndex < mon[monitorIndex]->modeCount; modeIndex++)
        {
            float aspectRatio = (float)mon[monitorIndex]->modes[modeIndex]->width / (float)mon[monitorIndex]->modes[modeIndex]->height;
            const float expectedAspectRatio[] = {4.0f / 3.0f, 16.0f / 9.0f, 21.5f / 9.0f, 16.0f / 10.0f};
            bool isExpectedAspectRatio = FALSE;

            for (int i = 0; i < sizeof(expectedAspectRatio) / sizeof(expectedAspectRatio[0]); i++)
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
                free(mon[monitorIndex]->modes[modeIndex]);
                mon[monitorIndex]->modes[modeIndex] = NULL;

                // Shift the remaining modes to fill the gap
                memmove(&mon[monitorIndex]->modes[modeIndex], &mon[monitorIndex]->modes[modeIndex + 1], (mon[monitorIndex]->modeCount - modeIndex - 1) * sizeof(Mode *));
                mon[monitorIndex]->modeCount--;

                TraceLog(LOG_INFO, "Adapter %i, Mode %i: Removed non-standard mode", monitorIndex, modeIndex);
                modeIndex--;
            }
            else
            {
                for (int otherIndex = modeIndex + 1; otherIndex < mon[monitorIndex]->modeCount; otherIndex++)
                {
                    if (mon[monitorIndex]->modes[modeIndex]->width == mon[monitorIndex]->modes[otherIndex]->width &&
                        mon[monitorIndex]->modes[modeIndex]->height == mon[monitorIndex]->modes[otherIndex]->height)
                    {
                        // Free the memory used by the removed mode
                        free(mon[monitorIndex]->modes[otherIndex]);
                        mon[monitorIndex]->modes[otherIndex] = NULL;

                        // Shift the remaining modes to fill the gap
                        memmove(&mon[monitorIndex]->modes[otherIndex], &mon[monitorIndex]->modes[otherIndex + 1], (mon[monitorIndex]->modeCount - otherIndex - 1) * sizeof(Mode *));
                        mon[monitorIndex]->modeCount--;

                        TraceLog(LOG_INFO, "Adapter %i, Mode %i: Removed duplicate mode", monitorIndex, otherIndex);
                        otherIndex--;
                    }
                }
            }
        }
    }

    TraceLog(LOG_INFO, "Number of monitors: %i", monitorCount);
}

void FreeModes()
{
    if (mon == NULL)
        return;

    for (int monitorIndex = 0; monitorIndex < 1; monitorIndex++)
    {
        if (mon[monitorIndex] == NULL)
            continue;

        for (int modeIndex = 0; modeIndex < mon[monitorIndex]->modeCount; modeIndex++)
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

void PlayRandomIntroSound()
{
    InitAudioDevice();

    srand(time(NULL));
    int random = rand() % AVP2_LAUNCHER_INTRO_COUNT;

    PlaySoundResource(AVP2_LAUNCHER_INTRO[random]);
}

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

void LoadAllResources()
{
}

void LoadBackgroundImages()
{
    HMODULE hInst = GetModuleHandle(NULL);

    for (size_t i = 0; i < AVP2_LAUNCHER_IMAGES_COUNT; i++)
    {
        HRSRC hRes = FindResourceA(hInst, AVP2_LAUNCHER_IMAGES[i], RT_BITMAP);
        HGLOBAL hGlobal = LoadResource(hInst, hRes);
        void *pResource = LockResource(hGlobal);
        int nFileSizeLessHeader = SizeofResource(NULL, hRes);

        BITMAPFILEHEADER bitmapFileHeader = {0};
        bitmapFileHeader.bfType = 0x4D42; // BM
        bitmapFileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + nFileSizeLessHeader;
        bitmapFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

        void *pData = calloc(1, nFileSizeLessHeader + sizeof(BITMAPFILEHEADER));

        if (!pData) continue;

        memcpy(pData, &bitmapFileHeader, sizeof(BITMAPFILEHEADER));
        memcpy(pData + sizeof(BITMAPFILEHEADER), pResource, nFileSizeLessHeader);

        // convert bitmap to image
        Image iImage = LoadImageFromMemory(".bmp", pData, nFileSizeLessHeader + sizeof(BITMAPFILEHEADER));

        backgroundImage[i] = LoadTextureFromImage(iImage);
        UnloadImage(iImage);
        UnlockResource(hGlobal);
        FreeResource(hGlobal);
        free(pData);
    }
}

void LoadTextureFromResource(Texture *texture, const char *name)
{
    HMODULE hInst = GetModuleHandle(NULL);

    HRSRC hRes = FindResource(hInst, name, RT_BITMAP);
    HGLOBAL hGlobal = LoadResource(hInst, hRes);
    void *pResource = LockResource(hGlobal);
    int nFileSizeLessHeader = SizeofResource(NULL, hRes);

    BITMAPFILEHEADER bitmapFileHeader = {0};
    bitmapFileHeader.bfType = 0x4D42; // BM
    bitmapFileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + nFileSizeLessHeader;
    bitmapFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    void *pData = calloc(1, nFileSizeLessHeader + sizeof(BITMAPFILEHEADER));

    if (!pData) return;

    memcpy(pData, &bitmapFileHeader, sizeof(BITMAPFILEHEADER));
    memcpy(pData + sizeof(BITMAPFILEHEADER), pResource, nFileSizeLessHeader);

    // convert bitmap to image
    Image iImage = LoadImageFromMemory(".bmp", pData, nFileSizeLessHeader + sizeof(BITMAPFILEHEADER));

    *texture = LoadTextureFromImage(iImage);
    UnloadImage(iImage);
    UnlockResource(hGlobal);
    FreeResource(hGlobal);
    free(pData);
}