#ifndef SPLASH_H
#define SPLASH_H

#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "..\types.h"
#include "..\constants.h"

void SplashSetupScreen();
void SplashScreenRender();
void SplashUpdateLoop();
void SplashUnloadScreen();

#endif //SPLASH_H