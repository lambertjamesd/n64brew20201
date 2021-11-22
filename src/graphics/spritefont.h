
#ifndef _SPRITEFONT_H
#define _SPRITEFONT_H

#define ANSI_CHAR_COUNT 128

#include "sprite.h"

struct CharacterDefinition
{
    char character;
    char spriteLayer;
    struct SpriteTile data;
};

struct Font
{
    struct SpriteTile characters[ANSI_CHAR_COUNT];
    char characterLayer[ANSI_CHAR_COUNT];
    short spaceWidth; 
};

void fontInit(struct Font* font, int spaceWidth, struct CharacterDefinition* chars, int charCount);
void fontRenderText(struct RenderState* renderState, struct Font* font, const char* str, int x, int y, int scaleShift);
int fontMeasure(struct Font* font, const char* str, int scaleShift);

#endif