#include <genesis.h>

#include "structs.h"
#include "tiles.h"

u16 loadedTiles = 0;
u16 loadTile(const u32* tileLoc, u16 tileCount){
	VDP_loadTileData(tileLoc, loadedTiles, tileCount, 0);
	u16 loadedLoc = loadedTiles;
	loadedTiles += tileCount;
	return loadedLoc;
}

void drawTilemap(VDPPlane plane, Tilemap* tilemap, u16 offset, Vector2 source, Vector2 pos, Vector2 scale){
	Vector2 tilePos = {0, 0};
	while(tilePos.y < scale.y){
		VDP_setTileMapXY(
			plane, 
			tilemap->tiles[source.x + tilePos.x + (source.y + tilePos.y) * tilemap->scale.x] + offset, 
			pos.x + tilePos.x, 
			pos.y + tilePos.y
		);
		tilePos.x++;
		if(tilePos.x >= scale.x) tilePos = (Vector2){0, tilePos.y + 1};
	}
}

void fillRect(VDPPlane plane, u16 tile, Vector2 pos, Vector2 scale){
	Vector2 tilePos = {0, 0};
	while(tilePos.y < scale.y){
		VDP_setTileMapXY(plane, tile, pos.x + tilePos.x, pos.y + tilePos.y);
		tilePos.x++;
		if(tilePos.x >= scale.x) tilePos = (Vector2){0, tilePos.y + 1};
	}
}

void drawText(VDPPlane plane, Vector2 pos, char* string, u8 palette, u16 fontLoc, u8 charOffset){
	for(int i=0; i<strlen(string); i++){
		if(string[i] == 0x20) continue;
		VDP_setTileMapXY(plane, TILE_ATTR_FULL(palette, 0, 0, 0, fontLoc + string[i] - charOffset), pos.x + i, pos.y);
	}
}
