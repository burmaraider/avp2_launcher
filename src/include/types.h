#pragma once

#include "raylib.h"
#include "stdint.h"
#include <minwindef.h>


#define UP 0
#define FOCUS 1
#define HOVER 1
#define DOWN 2

#define SCREEN_MAIN 0
#define SCREEN_OPTIONS 1
#define SCREEN_DISPLAY 2

#define ERROR_SUCCESS 0

typedef struct button_t
{
    bool isEnabled;
    bool isPressed;
    Vector2 position;
    Texture texture[3];
    uint32_t currentTexture;
    char szName[32];
    void (*onPress)(struct button_t *);
    void (*onUnload)(struct button_t *);
} Button;

typedef struct checkbox_t
{
    bool isEnabled;
    bool isChecked;
    Vector2 position;
    Texture texture[2];
    uint32_t currentTexture;
    char szName[32];
    char szText[256];
    void (*onPress)(struct checkbox_t *);
    void (*onUnload)(struct checkbox_t *);
} Checkbox;

static void UnloadButton(Button *button)
{
    for (size_t i = 0; i < 3; i++)
    {
        UnloadTexture(button->texture[i]);
    }
}

static void UnloadCheckBox(Checkbox *checkbox)
{
    for (size_t i = 0; i < 2; i++)
    {
        UnloadTexture(checkbox->texture[i]);
    }
}

typedef struct mode_t
{
    uint32_t width;
    uint32_t height;
    uint32_t refreshRate;
} Mode;

typedef struct monitor_t
{
    uint32_t modeCount;
    Mode **modes;
} Monitor;

typedef struct settings_t
{
    char *szCommands;
    bool nDisableHardwareCursor;
    bool nDisableJoysticks;
    bool nDisableMovies;
    bool nDisableMusic;
    bool nDisableSound;
    bool nDisableTripleBuffering;
    bool nDisplayWarning;
    char *szLanguage;
    uint8_t nNumLauncherRuns;
    bool nOptionsWarning;
    bool nSaveCommands;
} LauncherSettings;

typedef unsigned short WORD;
typedef unsigned __LONG32 DWORD;
typedef char CHAR;
typedef short SHORT;
typedef __LONG32 LONG;