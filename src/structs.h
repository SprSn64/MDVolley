#ifndef STRUCTS_H
#define STRUCTS_H

#include <genesis.h>

typedef enum EntityFlags{
	ENTITY_DEFAULT = 0b00000011,
	ENTITY_ACTIVE = 0b00000001,
	ENTITY_VISIBLE = 0b00000010,
} EntityFlags;

typedef enum Behaviours{
	BHV_NONE,
	BHV_LINK,
} Behaviours;

typedef enum GameStates{
	GAME_PAUSE,
	GAME_ACTIVE,
} GameStates;

typedef struct Vector2{
	u16 x, y;
} Vector2;

typedef struct Entity{
	u16 index;
	Vector2 pos;
	u8 size; 
	u16 tile, bhv;
	u8 flags; 

	struct Entity *prev;
	struct Entity *next;
} Entity;

#endif
