#include <genesis.h>

#include "structs.h"

u8 spriteCount = 0;
Entity* headEntity = NULL;

Entity* addEntity(s16 posX, s16 posY, u16 bhv, u8 size, u16 tile){
	if(spriteCount >= 80) return NULL;
	Entity* newEntity = MEM_alloc(sizeof(Entity));
	newEntity->index = spriteCount;
	newEntity->pos = (Vector2){posX, posY}; 
	newEntity->size = size; newEntity->tile = tile; 
	newEntity->flags = ENTITY_DEFAULT;
	newEntity->bhv = bhv;
	
	newEntity->prev = NULL; newEntity->next = NULL;

	spriteCount++;
	VDP_setSpriteFull(newEntity->index, posX, posY, size, tile, spriteCount);

	if(!headEntity){
		headEntity = newEntity;
		return newEntity;
	}

	Entity* currEntity = headEntity;
	while(currEntity->next){currEntity = currEntity->next;}

	currEntity->next = newEntity; newEntity->prev = currEntity;

	return newEntity;
}

void freeEntity(Entity* item){
	if(item->prev) item->prev->next = item->next;
	if(item->next) item->next->prev = item->prev;

	if(item == headEntity)
		headEntity = item->next;
	spriteCount--;
	VDP_releaseSprites(item->index, 1);
	MEM_free(item);
}

Entity* linkTarget = NULL;
void updateEntity(Entity* item){
	VDP_setSpriteAttribut(item->index, item->tile);
	VDP_setSpriteSize(item->index, item->size);

	if(item->bhv == BHV_LINK && linkTarget){
		VDP_setSpritePosition(item->index, linkTarget->pos.x + item->pos.x, linkTarget->pos.y + item->pos.y);
	}else{
		linkTarget = item;
		VDP_setSpritePosition(item->index, item->pos.x, item->pos.y);
	}
}

void updateEntities(){
	Entity* currEntity = headEntity;
	while(currEntity){
		if(currEntity->flags & ENTITY_ACTIVE)
			updateEntity(currEntity);
		currEntity = currEntity->next;
	}
	linkTarget = NULL;
}

void freeEntities(){
	Entity* currEntity = headEntity;
	while(currEntity){
		Entity* nextEntity = currEntity->next;
		freeEntity(currEntity);
		currEntity = nextEntity;
	}
}