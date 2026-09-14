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
	s16 x, y;
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

typedef enum CharController{
	CHARCONTROL_NONE = 0x00,

	CHARCONTROL_JOY1 = 0x01,
	CHARCONTROL_JOY2 = 0x02,
	CHARCONTROL_JOY3 = 0x03,
	CHARCONTROL_JOY4 = 0x04,

	CHARCONTROL_CPU = 0xFF
} CharController;

typedef enum Character{
	CHAR_NONE = 0x00,
	CHAR_GIRL = 0x01,
	CHAR_FREAK = 0x02,

	CHAR_TEAM = 0x80,
	CHAR_CHAR = 0x7F //127 possible characters
} Character;

typedef struct PlayerChar{
	Entity *plrEntity;
	Entity *shadowEntity;
	u8 controller;
	u8 character;

	Vector2 pos, subPos, vel, sprOffset;
	u8 hitTimer;
} PlayerChar;

#endif
