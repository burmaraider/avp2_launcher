#include "raylib.h"
#include <string.h>

//Program Headers
#include "..\include\registry.h"
#include "..\include\utils.h"
#include "..\include\Screens\advanced.h"


static Button playButton;
static Button serverButton;
static Button displayButton;
static Button optionsButton;
static Button exitButton;
static Button minimizeButton;
static Button xButton;
static Button okButton;
static Button cancelButton;
static Button installButton;
static Button lithFAQButton;
static Button **buttons;


// predator animation
static uint32_t nPredatorTextureFrame = 0;
static Texture tPredatorAnimationTexture;
static float fPredatorCountdownTimer = 1.0f; // 10 seconds countdown

void SplashSetupScreen()
{

    // play button
    LoadTextureFromResource(&playButton.texture[0], "PLAYU");
    LoadTextureFromResource(&playButton.texture[1], "PLAYF");
    LoadTextureFromResource(&playButton.texture[2], "PLAYD");
    playButton.position = (Vector2){417, 19};
    playButton.onPress = ButtonPressCallback;
    playButton.onUnload = UnloadButton;
    playButton.isEnabled = TRUE;
    playButton.id = BUTTON_PLAY;

    // server button
    LoadTextureFromResource(&serverButton.texture[0], "SERVERU");
    LoadTextureFromResource(&serverButton.texture[1], "SERVERF");
    LoadTextureFromResource(&serverButton.texture[2], "SERVERD");
    serverButton.position = (Vector2){417, 55};
    serverButton.onPress = ButtonPressCallback;
    serverButton.onUnload = UnloadButton;
    serverButton.isEnabled = TRUE;
    serverButton.id = BUTTON_SERVER;

    // display button
    LoadTextureFromResource(&displayButton.texture[0], "DISPLAYU");
    LoadTextureFromResource(&displayButton.texture[1], "DISPLAYF");
    LoadTextureFromResource(&displayButton.texture[2], "DISPLAYD");
    displayButton.position = (Vector2){417, 91};
    displayButton.onPress = ButtonPressCallback;
    displayButton.onUnload = UnloadButton;
    displayButton.isEnabled = TRUE;
    displayButton.id = BUTTON_DISPLAY;

    // options button
    LoadTextureFromResource(&optionsButton.texture[0], "OPTIONSU");
    LoadTextureFromResource(&optionsButton.texture[1], "OPTIONSF");
    LoadTextureFromResource(&optionsButton.texture[2], "OPTIONSD");
    optionsButton.position = (Vector2){417, 127};
    optionsButton.onPress = ButtonPressCallback;
    optionsButton.onUnload = UnloadButton;
    optionsButton.isEnabled = TRUE;
    optionsButton.id = BUTTON_OPTIONS;

    // exit button
    LoadTextureFromResource(&exitButton.texture[0], "QUITU");
    LoadTextureFromResource(&exitButton.texture[1], "QUITF");
    LoadTextureFromResource(&exitButton.texture[2], "QUITD");
    exitButton.position = (Vector2){417, 199};
    exitButton.onPress = ButtonPressCallback;
    exitButton.onUnload = UnloadButton;
    exitButton.isEnabled = TRUE;
    exitButton.id = BUTTON_EXIT;

    // minimize button
    LoadTextureFromResource(&minimizeButton.texture[0], "MINIMIZEU");
    LoadTextureFromResource(&minimizeButton.texture[1], "MINIMIZEF");
    LoadTextureFromResource(&minimizeButton.texture[2], "MINIMIZED");
    minimizeButton.position = (Vector2){485, 1};
    minimizeButton.onPress = ButtonPressCallback;
    minimizeButton.onUnload = UnloadButton;
    minimizeButton.isEnabled = TRUE;
    minimizeButton.id = BUTTON_MINIMIZE;

    // x button
    LoadTextureFromResource(&xButton.texture[0], "CLOSEU");
    LoadTextureFromResource(&xButton.texture[1], "CLOSEF");
    LoadTextureFromResource(&xButton.texture[2], "CLOSED");
    xButton.position = (Vector2){505, 1};
    xButton.onPress = ButtonPressCallback;
    xButton.onUnload = UnloadButton;
    xButton.isEnabled = TRUE;
    xButton.id = BUTTON_X;

    // generic
    LoadTextureFromResource(&okButton.texture[0], "OKU");
    LoadTextureFromResource(&okButton.texture[1], "OKF");
    LoadTextureFromResource(&okButton.texture[2], "OKD");
    okButton.position = (Vector2){0, 0};
    okButton.onPress = ButtonPressCallback;
    okButton.onUnload = UnloadButton;
    okButton.isEnabled = FALSE;
    okButton.id = BUTTON_OK;

    LoadTextureFromResource(&cancelButton.texture[0], "CANCELU");
    LoadTextureFromResource(&cancelButton.texture[1], "CANCELF");
    LoadTextureFromResource(&cancelButton.texture[2], "CANCELD");
    cancelButton.position = (Vector2){0, 0};
    cancelButton.onPress = ButtonPressCallback;
    cancelButton.onUnload = UnloadButton;
    cancelButton.isEnabled = FALSE;
    cancelButton.id = BUTTON_CANCEL;

    LoadTextureFromResource(&installButton.texture[0], "INSTALLU");
    LoadTextureFromResource(&installButton.texture[1], "INSTALLF");
    LoadTextureFromResource(&installButton.texture[2], "INSTALLD");
    installButton.position = (Vector2){417, 19};
    installButton.onPress = ButtonPressCallback;
    installButton.onUnload = UnloadButton;
    installButton.id = BUTTON_INSTALL;

    LoadTextureFromResource(&lithFAQButton.texture[0], "LITHFAQU");
    LoadTextureFromResource(&lithFAQButton.texture[1], "LITHFAQF");
    LoadTextureFromResource(&lithFAQButton.texture[2], "LITHFAQD");
    lithFAQButton.position = (Vector2){417, 163};
    lithFAQButton.onPress = ButtonPressCallback;
    lithFAQButton.onUnload = UnloadButton;
    lithFAQButton.id = BUTTON_LITHFAQ;
    lithFAQButton.isEnabled = TRUE;


    buttons = (Button **)malloc(sizeof(Button *) * SPLASH_BUTTON_COUNT);

    if (!buttons)
    {
        MessageBox(NULL, "Failed to allocate memory for buttons!", "Error", MB_OK | MB_ICONERROR);
        exit(1);
    }

    for (size_t i = 0; i < SPLASH_BUTTON_COUNT; i++)
    {
        buttons[i] = (Button *)malloc(sizeof(Button));
        
        if (!buttons[i])
        {
            MessageBox(NULL, "Failed to allocate memory for buttons!", "Error", MB_OK | MB_ICONERROR);
            exit(1);
        }
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
    buttons[10] = &lithFAQButton;

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

}

void SplashScreenRender()
{
    //Start the 2D Canvas
    BeginDrawing();
    ClearBackground(BLACK);

    // draw background
    DrawTexture(g_backgroundImage[g_nCurrentScreen], 0, 0, WHITE);

    // draw predator animation
    DrawTexturePro(g_backgroundImage[5], (Rect){nPredatorTextureFrame * (g_backgroundImage[5].width / 24), 0, g_backgroundImage[5].width / 24, g_backgroundImage[5].height}, (Rect){387, 18, g_backgroundImage[5].width / 24, g_backgroundImage[5].height}, (Vector2){0, 0}, 0, WHITE);

    for (size_t i = 0; i < SPLASH_BUTTON_COUNT; i++)
    {
        if (buttons[i]->isEnabled)
        {
            DrawTexture(buttons[i]->texture[buttons[i]->currentTexture], buttons[i]->position.x, buttons[i]->position.y, WHITE);
        }
    }

    EndDrawing();
}


void SplashUnloadScreen()
{
    // null check
    if (buttons == NULL)
    {
        return;
    }
    // free buttons
    for (size_t i = 0; i < SPLASH_BUTTON_COUNT; i++)
    {
        buttons[i]->onUnload(buttons[i]);
        free(buttons[i]);
        buttons[i] = NULL;
    }
    free(buttons);
    buttons = NULL;

    
}

static void CheckAllButtons()
{
    for (size_t i = 0; i < SPLASH_BUTTON_COUNT; i++)
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

void SplashUpdateLoop()
{
    CheckAllButtons();

    float deltaTime = GetFrameTime();
    fPredatorCountdownTimer -= deltaTime;

    if (fPredatorCountdownTimer <= 0.0f)
    {
        fPredatorCountdownTimer = 1.0f;                         // Reset countdown
        nPredatorTextureFrame = (nPredatorTextureFrame + 1) % 25; // Cycle predator frame
    }
}