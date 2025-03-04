// Use this only if you want to enable logging to file instead of UAE console (heavy performance hit, not recommended)
//#define GENERIC_MAIN_LOG_PATH "game.log"

#include <ace/generic/main.h>
#include <ace/managers/key.h>
#include <ace/managers/state.h>
#include <ace/managers/joy.h>
// Without it compiler will yell about undeclared gameGsCreate etc
#include "game.h"

tStateManager *g_pGameStateManager = 0;
tState *g_pGameState = 0;

void genericCreate(void) {
  // Here goes your startup code
  logWrite("Hello, Amiga OS!\n");
  keyCreate(); // We'll use keyboard
	joyOpen();
  // Initialize gamestate
  g_pGameStateManager = stateManagerCreate();
  g_pGameState = stateCreate(gameGsCreate, gameGsLoop, gameGsDestroy, 0, 0);

  statePush(g_pGameStateManager, g_pGameState);
}

void genericProcess(void) {
  // Here goes code done each game frame
  keyProcess();
  joyProcess();
  stateProcess(g_pGameStateManager); // Process current gamestate's loop
}

void genericDestroy(void) {
  // Here goes your cleanup code
  stateManagerDestroy(g_pGameStateManager);
  stateDestroy(g_pGameState);
  keyDestroy();
  joyClose(); 
  logWrite("Goodbye, Amiga OS!\n");
}