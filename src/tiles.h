#ifndef TILES_H
#define TILES_H

typedef struct Tilemap{
	Vector2 scale;
	u16* tiles;
} Tilemap;

u16 loadedTiles;
u16 loadTile(const u32* tileLoc, u16 tileCount);

void drawTilemap(VDPPlane plane, Tilemap* tilemap, u16 offset, Vector2 source, Vector2 pos, Vector2 scale);
void fillRect(VDPPlane plane, u16 tile, Vector2 pos, Vector2 scale);

void drawText(VDPPlane plane, Vector2 pos, char* string, u8 palette, u16 fontLoc, u8 charOffset);

#endif
