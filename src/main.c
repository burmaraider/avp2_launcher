#include <Windows.h>
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
#include <GLFW/glfw3.h>

#include "include\loader.h"
#include "include\Screens\advanced.h"



/// @brief The registry keys and values used by the game.
/// @details The game uses the registry to store settings and the install directory.
/// @note The game uses the registry to store settings and the install directory.
char *g_pszCommandLine;
char *g_pszInstallDir;
bool g_bAVP2Installed;
LauncherSettings g_Settings;

uint32_t screenWidth = 525;
uint32_t screenHeight = 245;

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



uint32_t currentScreen = 0;
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
Checkbox **g_checkboxes;

// predator animation
uint32_t predatorFrame = 0;
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
        SetCallbacksAdvancedScreen(&g_cancelButton);
        SetCallbacksAdvancedScreen(&g_xButton);
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

    Screen = DefaultScreen;

    GetModes();

    //Load all the textures for the GUI
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
    FreeModes();

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
    DrawTexture(g_backgroundImage[currentScreen], 0, 0, WHITE);

    DrawTexturePro(g_backgroundImage[5], (Rect){predatorFrame * (g_backgroundImage[5].width / 24), 0, g_backgroundImage[5].width / 24, g_backgroundImage[5].height}, (Rect){387, 18, g_backgroundImage[5].width / 24, g_backgroundImage[5].height}, (Vector2){0, 0}, 0, WHITE);

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

    if(mon == NULL)
    {
        TraceLog(LOG_ERROR, "Failed to allocate memory for monitors!");
        exit(1);
    }

    // get modes for each adapter
    for (monitorIndex = 0; monitorIndex < monitorCount; monitorIndex++)
    {
        mon[monitorIndex] = calloc(1, sizeof(Monitor));

        if(mon[monitorIndex] == NULL)
        {
            TraceLog(LOG_ERROR, "Failed to allocate memory for monitor!");
            exit(1);
        }

        const GLFWvidmode *modes = glfwGetVideoModes(monitors[monitorIndex], &modeCount);
        mon[monitorIndex]->modeCount = modeCount;

        mon[monitorIndex]->modes = calloc(modeCount, sizeof(Mode *));

        if(mon[monitorIndex]->modes == NULL)
        {
            TraceLog(LOG_ERROR, "Failed to allocate memory for modes!");
            exit(1);
        }

        for (modeIndex = 0; modeIndex < modeCount; modeIndex++)
        {
            mon[monitorIndex]->modes[modeIndex] = calloc(1, sizeof(Mode));

            if(mon[monitorIndex]->modes[modeIndex] == NULL)
            {
                TraceLog(LOG_ERROR, "Failed to allocate memory for mode!");
                exit(1);
            }

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
                for (size_t otherIndex = modeIndex + 1; otherIndex < mon[monitorIndex]->modeCount; otherIndex++)
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