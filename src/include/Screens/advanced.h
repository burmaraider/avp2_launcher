#pragma once

#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "..\types.h"
#include "..\constants.h"
#include <time.h>


static Checkbox saveCommands;
static Checkbox disableMovies;
static Checkbox disableSound;
static Checkbox disableMusic;
static Checkbox disableMovies;
static Checkbox disableJoystick;
static Checkbox disableFog;
static Checkbox disableHardwareCursor;
static Checkbox disableTripleBuffering;
static Checkbox restoreDefaultSettings;
static Checkbox** checkboxesAdvanced;

void SetupScreenAdvanced();
void SetCallbacksAdvancedScreen(Button *button);
void RenderAdvancedScreen();
void CheckAllCheckBoxes();