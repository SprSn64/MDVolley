#include <genesis.h>
#include <string.h>

#include "structs.h"
#include "tiledata/init.h"

#include "entities.h"
#include "tiles.h"

u32 timer = 0;
u16 gameState = GAME_ACTIVE;

PlayerChar players[4];

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

	if(currScanline >= 94)
		VDP_setHorizontalScroll(BG_A, (currScanline >> 1) - 80);

	if(between(currScanline, 72, 136))
		VDP_setHorizontalScroll(BG_B, (currScanline >> 1) - 48);
}

void vInterrupt(){
	VDP_setHorizontalScroll(BG_A, 0); VDP_setHorizontalScroll(BG_B, 0);
	currScanline = 0;
	//subScroll = 0;
}

//TILE_ATTR_FULL(palette, priority, vflip, hflip, tile)
void gameLoop();
u16 textTileLoc; u16 playerTileLoc; u16 ballTileLoc; u16 ballShadowLoc;
u16 freakTileLoc;

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

	freakTileLoc = loadTile(freakTiles, 9);

	//VDP_loadTileData(girlJumpTiles, playerTileLoc, 15, 0);

	//Load Palettes
	PAL_setColors(0, testPalettes, 64);

	drawText(BG_A, (Vector2){0, 4}, "YOU FUCKING IDIOT", PAL3, textTileLoc, 33);

	//Draw Palettes

	for(int i=0; i<16 * 4; i++){
		VDP_setTileMapXY(BG_A, TILE_ATTR_FULL(i>>4, 0, 0, 0, i&15), i&15, i>>4);
	}

	//40x28
	VDP_loadTileData(netTiles, 512, 1, 0);
	fillRect(BG_A, 3, (Vector2){0, 13}, (Vector2){40, 14});
	fillRect(BG_A, 1, (Vector2){2, 14}, (Vector2){36, 12});

	fillRect(BG_B, TILE_ATTR_FULL(PAL0, 1, 0, 0, 512), (Vector2){19, 10}, (Vector2){1, 16});
	fillRect(BG_B, TILE_ATTR_FULL(PAL0, 1, 0, 1, 512), (Vector2){20, 10}, (Vector2){1, 16});

	SYS_disableInts();
	{
		VDP_setHIntCounter(HINT_COUNT - 1);
		VDP_setHInterrupt(1);
		SYS_setVIntCallback(vInterrupt);
		SYS_setHIntCallback(hInterrupt);
	}
	SYS_enableInts();

	players[0] = (PlayerChar){NULL, NULL, CHARCONTROL_JOY1, CHAR_GIRL, (Vector2){80, 140}, (Vector2){0, 0}, (Vector2){0, 0}, (Vector2){-12, -32}, 0};
	players[1] = (PlayerChar){NULL, NULL, CHARCONTROL_JOY2, CHAR_FREAK, (Vector2){80, 180}, (Vector2){0, 0}, (Vector2){0, 0}, (Vector2){-12, -24}, 0};
	players[2] = (PlayerChar){NULL, NULL, CHARCONTROL_CPU, CHAR_GIRL | CHAR_TEAM, (Vector2){240, 140}, (Vector2){0, 0}, (Vector2){0, 0}, (Vector2){-12, -32}, 0};
	players[3] = (PlayerChar){NULL, NULL, CHARCONTROL_NONE, CHAR_NONE, (Vector2){80, 180}, (Vector2){0, 0}, (Vector2){0, 0}, (Vector2){-12, -32}, 0};

	ballEntity = addEntity(200, 64, BHV_NONE, SPRITE_SIZE(3, 3), TILE_ATTR_FULL(PAL0, 0, 0, 0, ballTileLoc));

	players[0].plrEntity = addEntity(120, 120, BHV_NONE, SPRITE_SIZE(3, 3), TILE_ATTR_FULL(PAL0, 0, 0, 0, playerTileLoc));
	addEntity(0, 24, BHV_LINK, SPRITE_SIZE(3, 1), TILE_ATTR_FULL(PAL0, 0, 0, 0, playerTileLoc + 9));

	players[1].plrEntity = addEntity(200, 80, BHV_NONE, SPRITE_SIZE(3, 3), TILE_ATTR_FULL(PAL1, 0, 0, 0, freakTileLoc));
	//addEntity(0, 24, BHV_LINK, SPRITE_SIZE(3, 1), TILE_ATTR_FULL(PAL1, 0, 0, 1, playerTileLoc + 9));

	players[2].plrEntity = addEntity(200, 160, BHV_NONE, SPRITE_SIZE(3, 3), TILE_ATTR_FULL(PAL1, 0, 0, 0, playerTileLoc));
	addEntity(0, 24, BHV_LINK, SPRITE_SIZE(3, 1), TILE_ATTR_FULL(PAL1, 0, 0, 0, playerTileLoc + 9));

	shadowEntity = addEntity(160, 120, BHV_NONE, SPRITE_SIZE(3, 1), TILE_ATTR_FULL(PAL0, 0, 0, 0, ballShadowLoc));
	players[0].shadowEntity = addEntity(160, 120, BHV_NONE, SPRITE_SIZE(3, 1), TILE_ATTR_FULL(PAL0, 0, 0, 0, ballShadowLoc));
	players[1].shadowEntity = addEntity(160, 140, BHV_NONE, SPRITE_SIZE(3, 1), TILE_ATTR_FULL(PAL0, 0, 0, 0, ballShadowLoc));
	players[2].shadowEntity = addEntity(160, 140, BHV_NONE, SPRITE_SIZE(3, 1), TILE_ATTR_FULL(PAL0, 0, 0, 0, ballShadowLoc));
	
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

void updatePlayer(PlayerChar* plr);
void gameLoop(){
	ballVel[1] -= 4;
	ballSubPos[0] += ballVel[0]; ballSubPos[1] += ballVel[1]; ballSubPos[2] += ballVel[2]; 
	if(!between(ballSubPos[0], -255, 255)){
		ballPos.x += negate(ballSubPos[0] < 0);
		ballSubPos[0] -= (abs(ballSubPos[0]) >> 8) * 256 * negate(ballSubPos[0] < 0);
	}
	if(!between(ballSubPos[1], -255, 255)){
		ballY += negate(ballSubPos[1] < 0);
		ballSubPos[1] -= (abs(ballSubPos[1]) >> 8) * 256 * negate(ballSubPos[1] < 0);
	}
	if(!between(ballSubPos[2], -255, 255)){
		ballPos.y += negate(ballSubPos[2] < 0);
		ballSubPos[2] -= (abs(ballSubPos[2]) >> 8) * 256 * negate(ballSubPos[2] < 0);
	}

	if(!between(ballPos.x, 12, 308)){
		ballPos.x = ballPos.x < 160 ? 13 : 307;
		ballSubPos[0] = 0;
		ballVel[0] = -ballVel[0];
	}
	if(!between(ballPos.y, 104, 228)){
		ballPos.y = ballPos.y < 180 ? 104 : 227;
		ballSubPos[2] = 0;
		ballVel[2] = -ballVel[2];
	}

	if(ballY < 0){
		ballVel[1] = 240;
		ballY = 0;
	}

	ballEntity->pos = (Vector2){ballPos.x - 12, ballPos.y - ballY - 22};
	shadowEntity->pos = (Vector2){ballPos.x - 12, ballPos.y - 4};

	for(int i=0; i < 4; i++){
		if(players[i].character > 0) updatePlayer(&players[i]);
	}
}

/*void updatePlayer(Entity* plr, u16 joy){
	playerVel[0] += ((inputDown[joy] & BUTTON_RIGHT) != 0) - ((inputDown[joy] & BUTTON_LEFT) != 0);
	playerVel[1] += ((inputDown[joy] & BUTTON_DOWN) != 0) - ((inputDown[joy] & BUTTON_UP) != 0);

	playerSubPos[0] += playerVel[0]; playerSubPos[1] += playerVel[1];
	if(!between(playerSubPos[0], -255, 255)){
		playerPos.x += negate(playerSubPos[0] < 0);
		playerSubPos[0] -= 256 * negate(playerSubPos[0] < 0);
	}
	if(!between(playerSubPos[1], -255, 255)){
		playerPos.y += negate(playerSubPos[1] < 0);
		playerSubPos[1] -= 256 * negate(playerSubPos[1] < 0);
	}
	playerVel[0] += playerVel[0] != 0 ? negate(playerVel[0] > 0) : 0;
	playerVel[1] += playerVel[1] != 0 ? negate(playerVel[1] > 0) : 0;
	

	playerPos.x += ((inputDown[joy] & BUTTON_RIGHT) != 0) - ((inputDown[joy] & BUTTON_LEFT) != 0);
	playerPos.y += ((inputDown[joy] & BUTTON_DOWN) != 0) - ((inputDown[joy] & BUTTON_UP) != 0);

	plr->pos = (Vector2){playerPos.x - 12, playerPos.y - 32 - 12 * (hitTimer > 0)};
	playerShadow[joy]->pos = (Vector2){playerPos.x - 12, playerPos.y - 4};

	if(hitTimer > 0){
		hitTimer -= 1;
		if(hitTimer == 0){
			VDP_loadTileData(girlTiles, playerTileLoc, 12, 0);
			plr->next->size = SPRITE_SIZE(3, 1);
		}else{
			VDP_loadTileData(girlJumpTiles, playerTileLoc, 15, 0);
			plr->next->size = SPRITE_SIZE(3, 2);
		}
	}
}*/

void updatePlayer(PlayerChar* plr){
	if(!plr->plrEntity) return;

	if(between(plr->controller, CHARCONTROL_JOY1, CHARCONTROL_JOY4)){
		u8 currJoy = plr->controller - 1;

		plr->pos.x += ((inputDown[currJoy] & BUTTON_RIGHT) != 0) - ((inputDown[currJoy] & BUTTON_LEFT) != 0);
		plr->pos.y += ((inputDown[currJoy] & BUTTON_DOWN) != 0) - ((inputDown[currJoy] & BUTTON_UP) != 0);
	}

	if(plr->controller == CHARCONTROL_CPU){
		plr->pos = (Vector2){
			(plr->character & CHAR_TEAM) ? max(ballPos.x, 172) : min(ballPos.x, 148), 
			ballPos.y
		};
	}

	if(plr->character == CHAR_GIRL){
		plr->plrEntity->tile = TILE_ATTR_FULL(PAL0, 0, 0, 1, playerTileLoc);
		plr->plrEntity->next->tile = TILE_ATTR_FULL(PAL0, 0, 0, 1, playerTileLoc + 9);
	}

	if(plr->pos.x > 148 && !(plr->character & CHAR_TEAM)){
		plr->pos.x = 148;
	}
	if(plr->pos.x < 172 && (plr->character & CHAR_TEAM)){
		plr->pos.x = 172;
	}

	if(!between(plr->pos.y, 104, 240)){
		plr->pos.y = plr->pos.y < 180 ? 104 : 240;
	}

	plr->plrEntity->pos = (Vector2){
		plr->pos.x + plr->sprOffset.x, 
		//plr->pos.y + plr->sprOffset.y - 12 * (plr->hitTimer > 0)
		plr->pos.y + plr->sprOffset.y + ((plr->hitTimer * (plr->hitTimer - 32)) >> 4)
	};
	if(plr->shadowEntity)
		plr->shadowEntity->pos = (Vector2){plr->pos.x - 12, plr->pos.y - 4};

	if(between(ballPos.x - plr->pos.x, -12, 12) && between(ballPos.y - plr->pos.y, -12, 12) && ballY < 16){
		plr->hitTimer = 32;
		ballVel[1] = 360; 
		ballVel[0] = negate((plr->character & CHAR_TEAM) > 0) * 180 + (ballPos.x - plr->pos.x) * 4;
		ballVel[2] += (ballPos.y - plr->pos.y) * 16;
		ballY = 16;
	}

	if(plr->hitTimer > 0){
		plr->hitTimer -= 1;
	}
}