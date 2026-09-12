#ifndef ENTITIES_H
#define ENTITIES_H

#include "structs.h"

extern u8 spriteCount;

Entity* addEntity(s16 posX, s16 posY, u16 bhv, u8 size, u16 tile);

void updateEntity(Entity* item);
void updateEntities();

void freeEntity(Entity* item);
void freeEntities();

#endif
