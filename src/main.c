#include <genesis.h>
#include <string.h>

#include "structs.h"
#include "tiledata/init.h"

#include "entities.h"
#include "tiles.h"

u32 timer = 0;
u16 gameState = GAME_ACTIVE;

u8 between(s16 val, s16 min, s16 max){
	return val >= min && val <= max;
}
s8 negate(u8 bool){
	return 1 - 2 * bool;
}

u16 inputDown[4] = {0, 0, 0, 0}; u16 inputPressed[4] = {0, 0, 0, 0};

void inputHandler(u16 joy, u16 change, u16 state){
	if(joy > JOY_4) return;
	//if(joy != JOY_1) return;
	inputPressed[joy] = state & change;
	inputDown[joy] = state;
}

void setScroll(VDPPlane plane, s16 x, s16 y){
	VDP_setHorizontalScroll(plane, x);
	VDP_setVerticalScroll(plane, y);
}

#define HINT_COUNT 2
u16 currScanline = 0;
s16 subScroll = 0;
void hInterrupt(){
	currScanline += HINT_COUNT;

	//subScroll += 4 * HINT_COUNT;

	if(currScanline >= 128)
		VDP_setHorizontalScroll(BG_A, currScanline + 344);

	if(between(currScanline, 104, 168))
		VDP_setHorizontalScroll(BG_B, currScanline + 368);
}

void vInterrupt(){
	VDP_setHorizontalScroll(BG_A, 0); VDP_setHorizontalScroll(BG_B, 0);
	currScanline = 0;
	//subScroll = 0;
}

//TILE_ATTR_FULL(palette, priority, vflip, hflip, tile)
void gameLoop();
u16 textTileLoc; u16 playerTileLoc; u16 ballTileLoc; u16 ballShadowLoc;

Entity* playerEntity[2] = {NULL, NULL}; Entity* playerShadow[2] = {NULL, NULL};
Entity* ballEntity = NULL; Entity* shadowEntity = NULL;

int main(bool hardReset){
	JOY_init(); JOY_setEventHandler(&inputHandler);
	SPR_init();

	VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_PLANE);
	VDP_setScreenHeight240();

	loadedTiles = 0;

	loadTile(solidTiles, 16);
	textTileLoc = loadTile(textTiles, 64);
	playerTileLoc = loadTile(girlTiles, 15);
	ballTileLoc = loadTile(ballTiles, 9);
	ballShadowLoc = loadTile(ballShadow, 3);

	//VDP_loadTileData(girlJumpTiles, playerTileLoc, 15, 0);

	//Load Palettes
	PAL_setColors(0, testPalettes, 64);

	drawText(BG_A, (Vector2){0, 4}, "YOU FUCKING IDIOT", PAL3, textTileLoc, 33);

	//Draw Palettes

	for(int i=0; i<16 * 4; i++){
		VDP_setTileMapXY(BG_A, TILE_ATTR_FULL(i>>4, 0, 0, 0, i&15), i&15, i>>4);
	}

	//40x28
	VDP_loadTileData(netTiles, 64, 1, 0);
	fillRect(BG_A, 3, (Vector2){0, 16}, (Vector2){40, 12});
	fillRect(BG_A, 1, (Vector2){2, 18}, (Vector2){36, 8});

	fillRect(BG_B, TILE_ATTR_FULL(PAL0, 1, 0, 0, 64), (Vector2){20, 14}, (Vector2){1, 12});
	fillRect(BG_B, TILE_ATTR_FULL(PAL0, 1, 0, 1, 64), (Vector2){21, 14}, (Vector2){1, 12});

	SYS_disableInts();
	{
		VDP_setHIntCounter(HINT_COUNT - 1);
		VDP_setHInterrupt(1);
		SYS_setVIntCallback(vInterrupt);
		SYS_setHIntCallback(hInterrupt);
	}
	SYS_enableInts();

	ballEntity = addEntity(160, 64, BHV_NONE, SPRITE_SIZE(3, 3), TILE_ATTR_FULL(PAL0, 0, 0, 0, ballTileLoc));

	playerEntity[0] = addEntity(160, 120, BHV_NONE, SPRITE_SIZE(3, 3), TILE_ATTR_FULL(PAL0, 0, 0, 0, playerTileLoc));
	addEntity(0, 24, BHV_LINK, SPRITE_SIZE(3, 1), TILE_ATTR_FULL(PAL0, 0, 0, 0, playerTileLoc + 9));

	playerEntity[1] =addEntity(160, 120, BHV_NONE, SPRITE_SIZE(3, 3), TILE_ATTR_FULL(PAL1, 0, 0, 1, playerTileLoc));
	addEntity(0, 24, BHV_LINK, SPRITE_SIZE(3, 1), TILE_ATTR_FULL(PAL1, 0, 0, 1, playerTileLoc + 9));

	shadowEntity = addEntity(160, 120, BHV_NONE, SPRITE_SIZE(3, 1), TILE_ATTR_FULL(PAL0, 0, 0, 0, ballShadowLoc));
	playerShadow[0] = addEntity(160, 120, BHV_NONE, SPRITE_SIZE(3, 1), TILE_ATTR_FULL(PAL0, 0, 0, 0, ballShadowLoc));
	playerShadow[1] = addEntity(160, 120, BHV_NONE, SPRITE_SIZE(3, 1), TILE_ATTR_FULL(PAL0, 0, 0, 0, ballShadowLoc));
	
	while(1){
		timer++;
		//VDP_setHorizontalScroll(BG_A, timer >> 1);
		//VDP_setVerticalScroll(BG_A, timer >> 2);

		if(inputPressed[0] & BUTTON_START)
			gameState = gameState == GAME_ACTIVE ? GAME_PAUSE : GAME_ACTIVE;

		if(gameState == GAME_ACTIVE)
			gameLoop();

		//if((lastButton & BUTTON_START) && (GAME_ACTIVE || GAME_PAUSE))
		//	gameState = gameState == GAME_PAUSE ? GAME_ACTIVE : GAME_PAUSE;

		updateEntities();
		VDP_updateSprites(spriteCount, DMA_QUEUE_COPY);

		for(int i=0; i<4; i++){
			inputPressed[i] = 0;
		}

		SYS_doVBlankProcess();
	}

	freeEntities();
	return 0;
}

Vector2 ballPos = {160, 120};
s16 ballY = 60;
s16 ballSubPos[3] = {0, 0, 0};
s16 ballVel[3] = {0, 0, 0};

Vector2 playerPos = {160, 120};
s16 playerSubPos[2] = {0, 0};
s16 playerVel[2] = {0, 0};

void updatePlayer(Entity* plr, u16 joy);
void gameLoop(){
	ballVel[1] -= 4;
	ballSubPos[0] += ballVel[0]; ballSubPos[1] += ballVel[1]; ballSubPos[2] += ballVel[2]; 
	if(!between(ballSubPos[0], -255, 255)){
		ballPos.x += negate(ballSubPos[0] < 0);
		ballSubPos[0] -= 256 * negate(ballSubPos[0] < 0);
	}
	if(!between(ballSubPos[1], -255, 255)){
		ballY += negate(ballSubPos[1] < 0);
		ballSubPos[1] -= 256 * negate(ballSubPos[1] < 0);
	}
	if(!between(ballSubPos[2], -255, 255)){
		ballPos.y += negate(ballSubPos[2] < 0);
		ballSubPos[2] -= 256 * negate(ballSubPos[2] < 0);
	}

	if(!between(ballPos.x, 12, 308)){
		ballPos.x = ballPos.x < 160 ? 13 : 307;
		ballSubPos[0] = 0;
		ballVel[0] = -ballVel[0];
	}
	if(!between(ballPos.y, 12, 228)){
		ballPos.y = ballPos.y < 120 ? 13 : 227;
		ballSubPos[2] = 0;
		ballVel[2] = -ballVel[2];
	}

	if(ballY < 0){
		ballVel[1] = 240;
		ballY = 0;
	}

	if(between(ballPos.x - playerPos.x, -12, 12) && between(ballPos.y - playerPos.y, -12, 12) && ballY < 16 && ballVel[1] < 0){
		ballVel[1] = 360; 
		ballVel[0] += (ballPos.x - playerPos.x) * 16; ballVel[2] += (ballPos.y - playerPos.y) * 16;
		ballY = 16;
	}

	ballEntity->pos = (Vector2){ballPos.x - 12, ballPos.y - ballY - 22};
	shadowEntity->pos = (Vector2){ballPos.x - 12, ballPos.y - 4};

	updatePlayer(playerEntity[0], JOY_1);
}

void updatePlayer(Entity* plr, u16 joy){
	playerPos.x += ((inputDown[joy] & BUTTON_RIGHT) != 0) - ((inputDown[joy] & BUTTON_LEFT) != 0);
	playerPos.y += ((inputDown[joy] & BUTTON_DOWN) != 0) - ((inputDown[joy] & BUTTON_UP) != 0);
	plr->pos = (Vector2){playerPos.x - 12, playerPos.y - 32};
	playerShadow[joy]->pos = (Vector2){playerPos.x - 12, playerPos.y - 4};
}