#include "game.h"
#include <ace/managers/key.h>
#include <ace/managers/joy.h>
#include <ace/managers/game.h>
#include <ace/managers/system.h>
#include <ace/managers/rand.h>
#include <ace/managers/viewport/tilebuffer.h>
#include <ace/managers/viewport/simplebuffer.h>
#include <ace/managers/blit.h> // Blitting fns
#include <ace/utils/palette.h>
#include <ace/types.h>
#include <ace/utils/extview.h>
#include <ace/managers/viewport/camera.h>
#include <ace/utils/disk_file.h>
#include <ace/utils/font.h>


#define TILE_MAP_SIZE_X 300
#define TILE_MAP_SIZE_Y 46
#define TILESET_TILE_COUNT 255
#define GAME_BPP 5
#define SCORE_HEIGHT 16


static tView *s_pView; // View containing all the viewports
static tVPort *s_pVpScore; // Viewport for score
static tSimpleBufferManager *s_pScoreBuffer;
static tVPort *s_pVpMain; // Viewport for playfield
static tTileBufferManager *s_pMainBuffer;
static tBitMap *s_pTiles;
static UWORD s_pPalette[1 << GAME_BPP];

static tFont *s_pFont;
static tTextBitMap *s_pTextBitMap;

tRandManager g_sRand;

UWORD g_uCameraX=0;
UWORD g_uCameraY=0;
UWORD g_uCameraSpeed=2;


//static UBYTE s_pAttributeMap[TILE_MAP_SIZE_X][TILE_MAP_SIZE_Y];

static UWORD s_uwMapTileWidth;
static UWORD s_uwMapTileHeight;


static void onTileDraw(
	UWORD uwTileX, UWORD uwTileY,
	tBitMap *pBitMap, UWORD uwBitMapX, UWORD uwBitMapY
) {
}

void tileReset(void) {
	for(UWORD ubX = 0; ubX < TILE_MAP_SIZE_X; ++ubX) {
		for(UWORD ubY = 0; ubY < TILE_MAP_SIZE_Y; ++ubY) {
			//s_pAttributeMap[ubX][ubY] = 0;
		}
	}
}

void tileSetAttribute(UWORD uwTileX, UWORD uwTileY, UBYTE ubAttribute) {
	//s_pAttributeMap[uwTileX][uwTileY] = ubAttribute;
}

static void loadMapFake(void) {
	logWrite("Map Loading!\n");
	tileReset();

	randInit(&g_sRand, 2184, 1911);

	for(UWORD y = 0; y < TILE_MAP_SIZE_Y; ++y) {
		for(UWORD x = 0; x < TILE_MAP_SIZE_X; ++x) {
			//logWrite("X %u Y %u Value %u\n",x,y,ubTileData);
			s_pMainBuffer->pTileData[x][y] = randUwMax(&g_sRand,170);
			//logWrite("Main Buffer Done\n");
			//tileSetAttribute(x, y, ubTileData);
			//logWrite("Map Done\n");
		}
	}

	logWrite("Map Loaded!\n");
}


static void loadMap(void) {
	logWrite("Map Loading!\n");
	tileReset();

	tFile *pFileTilemap = diskFileOpen("data/W1L1.dat", "rb");
	fileRead(pFileTilemap, &s_uwMapTileWidth, sizeof(s_uwMapTileWidth));
	fileRead(pFileTilemap, &s_uwMapTileHeight, sizeof(s_uwMapTileHeight));

  	logWrite("s_uwMapTileWidth %u",s_uwMapTileWidth);

  	logWrite("s_uwMapTileHeight %u",s_uwMapTileHeight);
  
	//fileRead(pFileTilemap, s_pMainBuffer->pTileData, sizeof(UBYTE) * s_uwMapTileWidth * s_uwMapTileHeight);

	UBYTE ubTileData;
	for(UWORD y = 0; y < s_uwMapTileHeight; ++y) {
		for(UWORD x = 0; x < s_uwMapTileWidth; ++x) {
			fileRead(pFileTilemap, &ubTileData, sizeof(ubTileData));
			//logWrite("X %u Y %u Value %u\n",x,y,ubTileData);
			s_pMainBuffer->pTileData[x][y] = ubTileData;
			//logWrite("Main Buffer Done\n");
			//tileSetAttribute(x, y, ubTileData);
			//logWrite("Map Done\n");
		}
	}
	
	fileClose(pFileTilemap);

	logWrite("Map Loaded!\n");
	
}


void gameGsCreate(void) {
  s_pView = viewCreate(0, TAG_END);

  // Viewport for score bar - on top of screen
	s_pVpScore = vPortCreate(0,
		TAG_VPORT_VIEW, s_pView,
		TAG_VPORT_BPP, GAME_BPP,
		TAG_VPORT_HEIGHT, SCORE_HEIGHT,
	TAG_END);

	s_pScoreBuffer = simpleBufferCreate(0,
		TAG_SIMPLEBUFFER_VPORT, s_pVpScore,
		TAG_SIMPLEBUFFER_BITMAP_FLAGS, BMF_CLEAR,
		TAG_SIMPLEBUFFER_BOUND_WIDTH, 320,
		TAG_SIMPLEBUFFER_BOUND_HEIGHT, SCORE_HEIGHT,
	TAG_END);

	// Now let's do the same for main playfield
	s_pVpMain = vPortCreate(0,
		TAG_VPORT_VIEW, s_pView,
		TAG_VPORT_BPP, GAME_BPP,
	TAG_END);

	s_pTiles = bitmapCreateFromPath("data/W1-Sheet.bm", 0);

	logWrite("Tiles Loaded!\n");

	s_pMainBuffer = tileBufferCreate(0,
		TAG_TILEBUFFER_VPORT, s_pVpMain,
		TAG_TILEBUFFER_BITMAP_FLAGS, BMF_CLEAR | BMF_INTERLEAVED,
		TAG_TILEBUFFER_BOUND_TILE_X, TILE_MAP_SIZE_X,
		TAG_TILEBUFFER_BOUND_TILE_Y, TILE_MAP_SIZE_Y,
		TAG_TILEBUFFER_IS_DBLBUF, 1,
		TAG_TILEBUFFER_TILE_SHIFT, 4, //Size of tile, given in bitshift. Set to 4 for 16px, 5 for 32px, etc. Mandatory.
		TAG_TILEBUFFER_REDRAW_QUEUE_LENGTH, 200,
		TAG_TILEBUFFER_CALLBACK_TILE_DRAW, onTileDraw,
		TAG_TILEBUFFER_TILESET, s_pTiles,
	TAG_END);

  	paletteLoadFromPath("data/supergre.plt", s_pPalette, 1 << GAME_BPP);
	memcpy(s_pVpScore->pPalette, s_pPalette, sizeof(s_pVpScore->pPalette));
	memcpy(s_pVpMain->pPalette, s_pPalette, sizeof(s_pVpMain->pPalette));


	s_pFont = fontCreateFromPath("data/fonts/silkscreen.fnt");
	s_pTextBitMap = fontCreateTextBitMap(320, s_pFont->uwHeight);

	loadMap();
	//loadMapFake();

	systemUnuse();


	logWrite("Camera Reset\n");
	cameraReset(s_pMainBuffer->pCamera, 0, 0, s_uwMapTileWidth*16, s_uwMapTileHeight * 16, 1);

	tileBufferRedrawAll(s_pMainBuffer);

	cameraSetCoord(s_pMainBuffer->pCamera, g_uCameraX, g_uCameraY);

	char szMsg[50];
	sprintf(szMsg, "Speed %u", g_uCameraSpeed);
	fontDrawStr(s_pFont, s_pScoreBuffer->pBack, 0, 2, szMsg, 1, FONT_LEFT | FONT_TOP | FONT_COOKIE, s_pTextBitMap);

	// Load the view
	viewLoad(s_pView);
}

void gameGsLoop(void) {
	// This will loop every frame
	if(keyCheck(KEY_ESCAPE)) {
	logWrite("Existing!\n");
	gameExit();
	return;
	}

	char szMsg[50];
	sprintf(szMsg, "Speed %u", g_uCameraSpeed);
	fontDrawStr(s_pFont, s_pScoreBuffer->pBack, 0, 2, szMsg, 0, FONT_LEFT | FONT_TOP | FONT_COOKIE, s_pTextBitMap);

	WORD wDx = 0, wDy = 0;
	if(keyCheck(KEY_SPACE)) {
		logWrite("Hit!\n");
	}
	if(joyCheck(JOY1_FIRE)) {
		logWrite("Hit Joy!\n");
	}
	if(keyCheck(KEY_Q)) {
		g_uCameraSpeed+=1;
	}
	if(keyCheck(KEY_W)) {
		if(g_uCameraSpeed>1) {
			g_uCameraSpeed-=1;
		}
	}
	if(keyCheck(KEY_SPACE)) {
		logWrite("Hit!\n");
	}
	if(joyCheck(JOY1_UP)) {
		wDy=-1*g_uCameraSpeed;
	}
	if(joyCheck(JOY1_DOWN)) {
		wDy=g_uCameraSpeed;
	}
	if(joyCheck(JOY1_LEFT)) {
		wDx=-1*g_uCameraSpeed;
	}
	if(joyCheck(JOY1_RIGHT)) {
		wDx=g_uCameraSpeed;
	}
	cameraMoveBy(s_pMainBuffer->pCamera, wDx, wDy);


	sprintf(szMsg, "Speed %u", g_uCameraSpeed);
	fontDrawStr(s_pFont, s_pScoreBuffer->pBack, 0, 2, szMsg, 1, FONT_LEFT | FONT_TOP | FONT_COOKIE, s_pTextBitMap);

	viewProcessManagers(s_pView);
	copProcessBlocks();
	vPortWaitForEnd(s_pVpMain);
}

void gameGsDestroy(void) {
  systemUse();
  fontDestroyTextBitMap(s_pTextBitMap);
  fontDestroy(s_pFont);
  bitmapDestroy(s_pTiles);
  viewDestroy(s_pView);
}