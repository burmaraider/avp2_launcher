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
    uint8_t id;
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

typedef struct rendererInfo_t
{
    char szModuleName[256];
    char szModuleFileName[256];
}RendererInfo;

typedef enum configType_e
{
    CONFIG_TYPE_STRING,
    CONFIG_TYPE_WITHOUT_QUOTES,
}ConfigType;


typedef struct autoexecCfg_t
{
    ConfigType nType;
    char *szKey;
    char *szValue;
    struct autoexecCfg_t *pNext;
}AutoexecCfg;

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

//button ID's
typedef enum button_e
{
BUTTON_PLAY,
BUTTON_SERVER,
BUTTON_DISPLAY,
BUTTON_OPTIONS,
BUTTON_EXIT,
BUTTON_MINIMIZE,
BUTTON_X,
BUTTON_OK,
BUTTON_CANCEL,
BUTTON_INSTALL,
BUTTON_LITHFAQ
}ButtonTypes;


typedef enum checkbox_e
{
CHECKBOX_NONE,
CHECKBOX_ALWAYS_RUN,
CHECKBOX_NO_SOUND,
CHECKBOX_NO_MUSIC,
CHECKBOX_NO_LOGOS,
CHECKBOX_NO_TRIPLEBUFFERING,
CHECKBOX_NO_JOYSTICK,
CHECKBOX_NO_HARDWARECURSOR,
}CheckboxTypes;

#endif // _TYPES_H_