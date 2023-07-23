#ifndef _TYPES_H_
#define _TYPES_H_

#include <Windows.h>
#include "raylib.h"
#include "stdint.h"

#define UP 0
#define FOCUS 1
#define HOVER 1
#define DOWN 2

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
    uint8_t id;
    void (*onPress)(struct checkbox_t *);
    void (*onUnload)(struct checkbox_t *);
    void (*onHover)(struct checkbox_t *);
} Checkbox;



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
    uint32_t nNumLauncherRuns;
    bool nOptionsWarning;
    bool nSaveCommands;
} LauncherSettings;

#endif // _TYPES_H_