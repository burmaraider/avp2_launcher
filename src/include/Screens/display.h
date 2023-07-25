#ifndef DISPLAY_H
#define DISPLAY_H

#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "..\types.h"
#include "..\constants.h"

void DisplaySetupScreen(void *pRenderLoop, void *pUpdateLoop);
void DisplayRenderScreen();
void DisplayUpdateLoop();
void DisplayUnloadScreen();

#endif // DISPLAY_H