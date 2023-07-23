#ifndef OPTIONS_H
#define OPTIONS_H

#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "..\types.h"
#include "..\constants.h"

void OptionsSetupScreen(void *pRenderLoop, void *pUpdateLoop);
void OptionsRenderScreen();
void OptionsUpdateLoop();
void OptionsUnloadScreen();

#endif