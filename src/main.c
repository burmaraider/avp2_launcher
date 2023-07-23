#include "raylib.h"
#include "raymath.h"

// C headers
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

// Program headers
#include "include\types.h"
#include "include\registry.h"
#include "include\utils.h"

// Screen headers
#include "include\Screens\splash.h"
#include "include\Screens\advanced.h"

void InitGame(void);
bool bShouldClose = FALSE;

/// GLOBAL DELCARATIONS
uint32_t g_nCurrentScreen = 0;
Font g_font;
Texture g_backgroundImage[6];
char *g_pszCommandLine;
char *g_pszInstallDir;
bool g_bAVP2Installed;
LauncherSettings g_Settings;
void (*ScreenRenderLoop)(void);
void (*ScreenUpdateLoop)(void);

int main(int argc, char *argv[])
{
    // GET THE COMMAND LINE TO HANDLE OUR INSTALL ARGUMENT WITH ADMIN RIGHTS
    if (argc > 1)
    {
        if (strcmp(argv[1], "-install") == 0)
        {
            // start install process
            InstallAVP2Registry();
            // relaunch the launcher as non admin
            char szPath[MAX_PATH];
            GetModuleFileName(NULL, szPath, MAX_PATH);
            ShellExecute(NULL, "open", szPath, NULL, NULL, SW_SHOWNORMAL);
            return 0;
        }
    }

    // GET THE AVP2 REGISTRY ENTRIES, IF THEY EXIST
    GetRegistryEntries();

    // INIT RAYLIB
    // VSYNC AND WINDOW UNDECORATED
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_UNDECORATED);

    // SETS THE WINDOW TITLE
    //--------------------------------------------------------------------------------------
    char *szTitle;
    szTitle = (char *)malloc(256);
    if (szTitle == NULL)
    {
        MessageBox(NULL, "Failed to allocate memory for the window title.", "Error", MB_OK | MB_ICONERROR);
        exit(1);
    }
    LoadString(GetModuleHandle(NULL), IDS_APPNAME, szTitle, 256);
    InitWindow(AVP2_MAIN_SCREEN_WIDTH, AVP2_MAIN_SCREEN_HEIGHT, szTitle);
    free(szTitle);

    // SETUP OUR FUNCTIONS FOR UPDATES
    //--------------------------------------------------------------------------------------
    SplashSetupScreen();
    ScreenRenderLoop = SplashScreenRender;
    ScreenUpdateLoop = SplashUpdateLoop;

    // GET ALL MODES REPORTED BY GLFW
    //--------------------------------------------------------------------------------------
    // GetModes(mon);

    // Load all the textures for the GUI
    //--------------------------------------------------------------------------------------
    Loader_InitializeBackgroundTextures();

    PlayRandomIntroSound();

    while (!bShouldClose)
    {
        // Allow the user to drag the window context
        DragWindow();

        // Update loop, can change with function pointers
        ScreenUpdateLoop();

        // Render loop, can change with function pointers
        ScreenRenderLoop();
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(g_backgroundImage[0]);
    UnloadTexture(g_backgroundImage[1]);
    UnloadTexture(g_backgroundImage[2]);
    UnloadTexture(g_backgroundImage[3]);
    UnloadTexture(g_backgroundImage[4]);
    UnloadTexture(g_backgroundImage[5]);

    UnloadFont(g_font);

    free(g_pszCommandLine);
    free(g_pszInstallDir);
    free(g_Settings.szCommands);
    free(g_Settings.szLanguage);
    // FreeModes(mon);

    OptionsUnloadScreen();
    SplashUnloadScreen();
    CloseAudioDevice();

    ExitWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    return 0;
}

void ButtonPressCallback(Button *button)
{
    PlaySoundResource("OK");

    if (strcmp(button->szName, "play") == 0)
    {
        g_nCurrentScreen = 1;
    }
    else if (strcmp(button->szName, "server") == 0)
    {
    }
    else if (strcmp(button->szName, "display") == 0)
    {

        // g_nCurrentScreen = SCREEN_DISPLAY;
    }
    else if (strcmp(button->szName, "options") == 0)
    {
        g_nCurrentScreen = SCREEN_OPTIONS;
        OptionsSetupScreen(ScreenRenderLoop, ScreenUpdateLoop);
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
    else if (strcmp(button->szName, "install") == 0)
    {
        InstallAVP2Registry();
    }
}