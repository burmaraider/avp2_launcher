#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "types.h"

extern int screenWidth;
extern int screenHeight;

#define IDI_APPICON                     101
#define IDS_APPNAME                     1

static const char* const AVP2_REGISTRY_x64 = "SOFTWARE\\Monolith Productions\\Aliens vs. Predator 2\\1.0";
static const char* const AVP2_USER_REGISTRY_x64 = "Software\\Classes\\VirtualStore\\MACHINE\\SOFTWARE\\WOW6432Node\\Monolith Productions\\Aliens vs. Predator 2\\1.0";
static const char* const AVP2_REGISTRY_COMMANDLINE = "Update Command Line";
static const char* const AVP2_REGISTRY_COMMANDS = "Commands";
static const char* const AVP2_REGISTRY_INSTALLDIR = "InstallDir";

static const char* const AVP2_DEFAULT_COMMANDLINE = "-rez AVP2P1.REZ -rez AVP2SP.REZ -rez avp2p5.rez";
static const char* const AVP2_DEFAULT_INSTALLDIR = "C:\\Program Files (x86)\\Aliens vs. Predator 2";

static const unsigned int AVP2_LAUNCHER_IMAGES_COUNT =6;
static const char* const AVP2_LAUNCHER_IMAGES[6] = {
    "MAIN",
    "OPTIONS",
    "DISPLAY",
    "INFO",
    "GRAPHICS",
    "PREDANIM"
};

static const unsigned int AVP2_LAUNCHER_INTRO_COUNT =3;
static const char* const AVP2_LAUNCHER_INTRO[3] = {"INTRO1", "INTRO2", "INTRO3"};

static const unsigned int AVP2_LAUNCHER_TYPE_SOUND_COUNT =3;
static const char* const AVP2_LAUNCHER_TYPE_SOUND[3] = {"TYPE1", "TYPE2", "TYPE3"};
static const char* const AVP2_LAUNCHER_TYPE_SOUND_BACKSPACE = "BACKSPACE";

extern char* g_pszCommandLine;
extern char* g_pszInstallDir;
extern bool g_bAVP2Installed;
extern LauncherSettings g_Settings;

extern Button playButton;
extern Button serverButton;
extern Button displayButton;
extern Button optionsButton;
extern Button exitButton;
extern Button minimizeButton;
extern Button xButton;

// generic
extern Button okButton;
extern Button cancelButton;

static const int BUTTON_COUNT = 10;
extern Button** buttons;
static const int CHECK_COUNT = 7;
extern Checkbox** checkboxes;

extern int currentScreen;
extern Font g_font;
extern Texture backgroundImage[6];

extern void (*Screen)(void);
extern void DefaultScreen(void);
extern void ButtonPress(Button *button);
extern void PlaySoundResource(const char* szName);
extern void LoadTextureFromResource(Texture *texture, const char *name);