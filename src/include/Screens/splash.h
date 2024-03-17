#ifndef SPLASH_H
#define SPLASH_H

#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "..\types.h"
#include "..\constants.h"

void SplashSetupScreen(void);
void SplashScreenRender(void);
void SplashUpdateLoop(void);
void SplashUnloadScreen(void);

#endif // SPLASH_H