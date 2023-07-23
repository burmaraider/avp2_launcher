#ifndef SPLASH_H
#define SPLASH_H

#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "..\types.h"
#include "..\constants.h"

void SetupScreenSplash();
void SetCallbacksSplashScreen(Button *button);
void RenderSplashScreen();
void CheckAllButtons();
void SplashUpdateLoop();

#endif //SPLASH_H