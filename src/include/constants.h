#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

#include <stdbool.h>
#include <stdint.h>
#include "types.h"

static const uint32_t AVP2_MAIN_SCREEN_WIDTH = 525;
static const uint32_t AVP2_MAIN_SCREEN_HEIGHT = 245;

#define IDI_APPICON                     101
#define IDS_APPNAME                     1

static const char* const TEST = "This restores all configuration settings to the original defaults. This includes resetting the key configuration, detail level and sound options. This is useful if you've changed settings, and you just want things back the way they were. ";

static const char* const AVP2_REGISTRY_x64 = "SOFTWARE\\Monolith Productions\\Aliens vs. Predator 2\\1.0";
static const char* const AVP2_USER_REGISTRY_x64 = "Software\\Classes\\VirtualStore\\MACHINE\\SOFTWARE\\WOW6432Node\\Monolith Productions\\Aliens vs. Predator 2\\1.0";
static const char* const AVP2_REGISTRY_COMMANDLINE = "Update Command Line";
static const char* const AVP2_REGISTRY_COMMANDS = "Commands";
static const char* const AVP2_REGISTRY_INSTALLDIR = "InstallDir";

static const char* const AVP2_DEFAULT_COMMANDLINE = "-rez AVP2P1.REZ -rez AVP2SP.REZ -rez avp2p5.rez";
static const char* const AVP2_DEFAULT_INSTALLDIR = "C:\\Program Files (x86)\\Aliens vs. Predator 2";

static const uint32_t AVP2_LAUNCHER_IMAGES_COUNT =6;
static const char* const AVP2_LAUNCHER_IMAGES[6] = {
    "MAIN",
    "OPTIONS",
    "DISPLAY",
    "INFO",
    "GRAPHICS",
    "PREDANIM"
};

static const uint32_t AVP2_LAUNCHER_INTRO_COUNT =3;
static const char* const AVP2_LAUNCHER_INTRO[3] = {"INTRO1", "INTRO2", "INTRO3"};

static const uint32_t AVP2_LAUNCHER_TYPE_SOUND_COUNT =3;
static const char* const AVP2_LAUNCHER_TYPE_SOUND[3] = {"TYPE1", "TYPE2", "TYPE3"};
static const char* const AVP2_LAUNCHER_TYPE_SOUND_BACKSPACE = "BACKSPACE";

extern char* g_pszCommandLine;
extern char* g_pszInstallDir;
extern bool g_bAVP2Installed;
extern LauncherSettings g_Settings;

extern Button g_playButton;
extern Button g_serverButton;
extern Button g_displayButton;
extern Button g_optionsButton;
extern Button g_exitButton;
extern Button g_minimizeButton;
extern Button g_xButton;
extern Button g_installButton;

// generic
extern Button g_okButton;
extern Button g_cancelButton;

static const uint32_t BUTTON_COUNT = 10;
extern Button** g_buttons;
static const uint32_t CHECK_COUNT = 7;

static const char* const AVP2_TOOLTIPS[8] = {
"What the hell am I looking at here?!"
,"This is for advanced users only. The command line is used for setting console variables at startup."
,"This will disable all sound effects. Use this for troublehooting only."
,"This will disable the use of DirectMusic. Some sound cards may have incompatibilities with DirectMusic.  Use this for troubleshooting only."
,"This will disable the logo movies at the start of the game."
,"This will disable a feature on some video cards that can improve performance, but uses more of the card's video memory as a result.  Most AGP video cards benefit from this feature being enabled.  However, this may cause lock-ups on a few PCI video cards."
,"This will disable all joysticks and gamepads.  Use this if your particular joystick is causing problems when the game starts."
,"This will disable the use of a hardware cursor. Some older cards do not support the hardware cursor."
};

extern uint32_t g_nCurrentScreen;
extern Font g_font;
extern Texture g_backgroundImage[6];

extern void (*Screen)(void);
extern void (*ScreenUpdateLoop)(void);
extern void DefaultScreen(void);
extern void MainScreenUpdateLoop(void);
extern void ButtonPressCallback(Button *button);

#endif // _CONSTANTS_H_