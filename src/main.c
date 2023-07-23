#include <Windows.h>
#include "raylib.h"
#include "raymath.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "include\types.h"
#include "include\registry.h"
#include "string.h"

#include <time.h>


#include "include\utils.h"
#include "include\Screens\advanced.h"

Monitor **mon;

void InitGame(void);
void GameLoop(void); // Update and Draw one frame
void DefaultScreen(void);
void MainScreenUpdateLoop(void);
void Inputs(void);
bool bShouldClose = FALSE;

/// GLOBAL DELCARATIONS
uint32_t g_nCurrentScreen = 0;
Font g_font;
Texture g_backgroundImage[6];
Button g_playButton;
Button g_serverButton;
Button g_displayButton;
Button g_optionsButton;
Button g_exitButton;
Button g_minimizeButton;
Button g_xButton;
Button g_okButton;
Button g_cancelButton;
Button g_installButton;
Button **g_buttons;
char *g_pszCommandLine;
char *g_pszInstallDir;
bool g_bAVP2Installed;
LauncherSettings g_Settings;
void (*Screen)(void);
void (*ScreenUpdateLoop)(void);

// predator animation
uint32_t nPredatorTextureFrame = 0;
Texture tPredatorAnimationTexture;
float fPredatorCountdownTimer = 1.0f; // 10 seconds countdown

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

        g_nCurrentScreen = SCREEN_DISPLAY;
    }
    else if (strcmp(button->szName, "options") == 0)
    {

        g_nCurrentScreen = SCREEN_OPTIONS;
        Screen = RenderAdvancedScreen;
        SetupScreenAdvanced();
        SetCallbacksAdvancedScreen(&g_cancelButton);
        SetCallbacksAdvancedScreen(&g_xButton);
        ScreenUpdateLoop = AdvancedUpdateLoop;
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
    // GET THE COMMAND LINE TO HANDLE OUR INSTALL ARGUMENT WITH ADMIN RIGHTS
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

    // GET THE AVP2 REGISTRY ENTRIES, IF THEY EXIST
    GetRegistryEntries();



    // INIT RAYLIB
    // VSYNC AND WINDOW UNDECORATED
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_UNDECORATED);

    //SETS THE WINDOW TITLE
    //--------------------------------------------------------------------------------------
    char szTitle[256];
    LoadString(GetModuleHandle(NULL), IDS_APPNAME, szTitle, 256);
    InitWindow(AVP2_MAIN_SCREEN_WIDTH, AVP2_MAIN_SCREEN_HEIGHT, szTitle);

    //SETUP OUR FUNCTIONS FOR UPDATES
    //--------------------------------------------------------------------------------------
    Screen = DefaultScreen;
    ScreenUpdateLoop = MainScreenUpdateLoop;

    //GET ALL MODES REPORTED BY GLFW
    //--------------------------------------------------------------------------------------
    GetModes(mon);

    //Load all the textures for the GUI
    //--------------------------------------------------------------------------------------
    Loader_InitializeAllTextures();

    PlayRandomIntroSound();

    while (!bShouldClose)
    {
        GameLoop();
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(g_backgroundImage[0]);
    UnloadTexture(g_backgroundImage[1]);
    UnloadTexture(g_backgroundImage[2]);
    UnloadTexture(g_backgroundImage[3]);
    UnloadTexture(g_backgroundImage[4]);
    UnloadTexture(g_backgroundImage[5]);

    g_playButton.onUnload(&g_playButton);
    g_serverButton.onUnload(&g_serverButton);
    g_displayButton.onUnload(&g_displayButton);
    g_optionsButton.onUnload(&g_optionsButton);
    g_exitButton.onUnload(&g_exitButton);
    g_minimizeButton.onUnload(&g_minimizeButton);
    g_xButton.onUnload(&g_xButton);
    g_okButton.onUnload(&g_okButton);
    g_cancelButton.onUnload(&g_cancelButton);
    g_installButton.onUnload(&g_installButton);

    UnloadFont(g_font);

    free(g_pszCommandLine);
    free(g_pszInstallDir);
    free(g_Settings.szCommands);
    free(g_Settings.szLanguage);
    FreeModes(mon);

    for (size_t i = 0; i < BUTTON_COUNT; i++)
    {
        g_buttons[i] = NULL;
        free(g_buttons[i]);
        // free(buttons);
    }

    ExitWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    return 0;
}

void GameLoop(void)
{
    Inputs();
    ScreenUpdateLoop();

    BeginDrawing();
    ClearBackground(BLACK);

    Screen();

    EndDrawing();
}

void MainScreenUpdateLoop()
{
    float deltaTime = GetFrameTime();
    fPredatorCountdownTimer -= deltaTime;
    if (fPredatorCountdownTimer <= 0.0f)
    {
        fPredatorCountdownTimer = 1.0f;                         // Reset countdown
        nPredatorTextureFrame = (nPredatorTextureFrame + 1) % 25; // Cycle predator frame
    }
}

void DefaultScreen()
{
    // draw background
    DrawTexture(g_backgroundImage[g_nCurrentScreen], 0, 0, WHITE);

    DrawTexturePro(g_backgroundImage[5], (Rect){nPredatorTextureFrame * (g_backgroundImage[5].width / 24), 0, g_backgroundImage[5].width / 24, g_backgroundImage[5].height}, (Rect){387, 18, g_backgroundImage[5].width / 24, g_backgroundImage[5].height}, (Vector2){0, 0}, 0, WHITE);

    if(g_bAVP2Installed)
    {
        DrawTexture(g_playButton.texture[g_playButton.currentTexture], g_playButton.position.x, g_playButton.position.y, WHITE);
        DrawTexture(g_serverButton.texture[g_serverButton.currentTexture], g_serverButton.position.x, g_serverButton.position.y, WHITE);
        DrawTexture(g_displayButton.texture[g_displayButton.currentTexture], g_displayButton.position.x, g_displayButton.position.y, WHITE);
        DrawTexture(g_optionsButton.texture[g_optionsButton.currentTexture], g_optionsButton.position.x, g_optionsButton.position.y, WHITE);
    }
    else
    {
        DrawTexture(g_installButton.texture[g_installButton.currentTexture], g_installButton.position.x, g_installButton.position.y, WHITE);
    }

    DrawTexture(g_exitButton.texture[g_exitButton.currentTexture], g_exitButton.position.x, g_exitButton.position.y, WHITE);
    DrawTexture(g_minimizeButton.texture[g_minimizeButton.currentTexture], g_minimizeButton.position.x, g_minimizeButton.position.y, WHITE);
    DrawTexture(g_xButton.texture[g_xButton.currentTexture], g_xButton.position.x, g_xButton.position.y, WHITE);
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
    for (size_t i = 0; i < BUTTON_COUNT; i++)
    {
        if (CheckCollisionPointRec(GetMousePosition(), (Rect){g_buttons[i]->position.x, g_buttons[i]->position.y, g_buttons[i]->texture[0].width, g_buttons[i]->texture[0].height})
        && g_buttons[i]->isEnabled == TRUE)
        {
            g_buttons[i]->currentTexture = HOVER;

            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
            {
                g_buttons[i]->currentTexture = DOWN;
                
            }

            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
            {
                g_buttons[i]->currentTexture = HOVER;
                g_buttons[i]->onPress(g_buttons[i]);
            }
        }
        else
        {
            g_buttons[i]->currentTexture = UP;
            g_buttons[i]->isPressed = FALSE;
        }
    }
}

void Inputs()
{
    DragWindow();
    CheckAllButtons();
}