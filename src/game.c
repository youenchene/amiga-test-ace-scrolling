#include "game.h"
#include <ace/managers/key.h>
#include <ace/managers/game.h>
#include <ace/managers/system.h>
#include <ace/managers/viewport/tilebuffer.h>
#include <ace/managers/viewport/simplebuffer.h>
#include <ace/managers/blit.h> // Blitting fns
#include <ace/utils/palette.h>
#include <ace/types.h>
#include <ace/utils/extview.h>
#include <ace/managers/viewport/camera.h>
#include <ace/utils/file.h>


#define TILE_MAP_SIZE_X 300
#define TILE_MAP_SIZE_Y 46

static tView *s_pView; // View containing all the viewports
static tVPort *s_pVpScore; // Viewport for score
static tSimpleBufferManager *s_pScoreBuffer;
static tVPort *s_pVpMain; // Viewport for playfield
static tTileBufferManager *s_pMainBuffer;
static tBitMap *s_pTiles;

static UBYTE s_pAttributeMap[TILE_MAP_SIZE_X][TILE_MAP_SIZE_Y];

static UWORD s_uwMapTileWidth;
static UWORD s_uwMapTileHeight;


tTileBufferManager *g_pMainBuffer;

static void onTileDraw(
	UWORD uwTileX, UWORD uwTileY,
	tBitMap *pBitMap, UWORD uwBitMapX, UWORD uwBitMapY
) {
}

void tileReset(void) {
	for(UWORD ubX = 0; ubX < TILE_MAP_SIZE_X; ++ubX) {
		for(UWORD ubY = 0; ubY < TILE_MAP_SIZE_Y; ++ubY) {
			s_pAttributeMap[ubX][ubY] = 0;
		}
	}
}

void tileSetAttribute(UWORD uwTileX, UWORD uwTileY, UBYTE ubAttribute) {
	s_pAttributeMap[uwTileX][uwTileY] = ubAttribute;
}

static void loadMap(void) {
	tileReset();

	systemUse();
	tFile *pFileTilemap = fileOpen("data/W1L1.dat", "rb");
	fileRead(pFileTilemap, &s_uwMapTileWidth, sizeof(s_uwMapTileWidth));
	fileRead(pFileTilemap, &s_uwMapTileHeight, sizeof(s_uwMapTileHeight));

  logWrite("sizeof(s_uwMapTileWidth)");
  //logWrite(sizeof(s_uwMapTileWidth));

	for(UWORD y = 0; y < s_uwMapTileHeight; ++y) {
		for(UWORD x = 0; x < s_uwMapTileWidth; ++x) {
			UBYTE uwTileData;
			fileRead(pFileTilemap, &uwTileData, sizeof(uwTileData));
			s_pMainBuffer->pTileData[x][y] = uwTileData;
			tileSetAttribute(x, y, uwTileData);
		}
	}
	fileClose(pFileTilemap);

	systemUnuse();
	cameraReset(s_pMainBuffer->pCamera, 0, 0, s_uwMapTileWidth*16, s_uwMapTileHeight * 16, 1);
}


void gameGsCreate(void) {
  s_pView = viewCreate(0, TAG_END);

  // Viewport for score bar - on top of screen
  s_pVpScore = vPortCreate(0,
    TAG_VPORT_VIEW, s_pView,
    TAG_VPORT_BPP, 4,
    TAG_VPORT_HEIGHT, 32,
  TAG_END);
  s_pScoreBuffer = simpleBufferCreate(0,
    TAG_SIMPLEBUFFER_VPORT, s_pVpScore,
    TAG_SIMPLEBUFFER_BITMAP_FLAGS, BMF_CLEAR,
  TAG_END);

  // Now let's do the same for main playfield
  s_pVpMain = vPortCreate(0,
    TAG_VPORT_VIEW, s_pView,
    TAG_VPORT_BPP, 5,
  TAG_END);

  s_pTiles = bitmapCreateFromPath("data/W1-Sheet.bm", 0);

	g_pMainBuffer = tileBufferCreate(0,
		TAG_TILEBUFFER_VPORT, s_pVpMain,
		TAG_TILEBUFFER_BITMAP_FLAGS, BMF_CLEAR | BMF_INTERLEAVED,
		TAG_TILEBUFFER_BOUND_TILE_X, TILE_MAP_SIZE_X,
		TAG_TILEBUFFER_BOUND_TILE_Y, TILE_MAP_SIZE_Y,
		TAG_TILEBUFFER_IS_DBLBUF, 1,
		TAG_TILEBUFFER_TILE_SHIFT, 4,
		TAG_TILEBUFFER_REDRAW_QUEUE_LENGTH, 100,
		TAG_TILEBUFFER_CALLBACK_TILE_DRAW, onTileDraw,
		TAG_TILEBUFFER_TILESET, s_pTiles,
	TAG_END);

  paletteLoadFromPath("data/supergre.plt", s_pVpScore->pPalette, 32);

  systemUnuse();

  loadMap();

  // Load the view
  viewLoad(s_pView);
}

void gameGsLoop(void) {
  // This will loop every frame
  if(keyCheck(KEY_ESCAPE)) {
    gameExit();
    return;
  }


  vPortWaitForEnd(s_pVpMain);
}

void gameGsDestroy(void) {
  systemUse();
  bitmapDestroy(s_pTiles);
  viewDestroy(s_pView);
}