
#include "spritefont.h"
#include "sprite.h"

void fontInit(struct Font* font, int layer, int spaceWidth, struct CharacterDefinition* chars, int charCount)
{
    font->spriteLayer = layer;
    font->spaceWidth = spaceWidth;

    for (int i = 0; i < ANSI_CHAR_COUNT; ++i)
    {
        font->characters[i].w = 0;
    }

    for (int i = 0; i < charCount; ++i)
    {
        font->characters[(unsigned)chars[i].character] = chars[i].data;
    }
}

void fontRenderText(struct RenderState* renderState, struct Font* font, const char* str, int x, int y, int scaleShift)
{
    int startX = x;

    while (*str)
    {
        struct SpriteTile curr = font->characters[(unsigned)*str];
        if (curr.w)
        {
            spriteDraw(renderState, font->spriteLayer, x, y, curr.w, curr.h, curr.x, curr.y, scaleShift, scaleShift);
            if (scaleShift >= 0) {
                x += curr.w << scaleShift;
            } else {
                x += curr.w >> -scaleShift;
            }
        }
        else if (*str == ' ')
        {
            if (scaleShift >= 0) {
                x += font->spaceWidth << scaleShift;
            } else {
                x += font->spaceWidth >> -scaleShift;
            }
        }
        else if (*str == '\n')
        {
            x = startX;
            y += 20;
        }

        ++str;
    }
}

int fontMeasure(struct Font* font, const char* str, int scaleShift) {
    int result = 0;

    while (*str)
    {
        struct SpriteTile curr = font->characters[(unsigned)*str];

        if (*str == ' ') {
            if (scaleShift >= 0) {
                result += font->spaceWidth << scaleShift;
            } else {
                result += font->spaceWidth >> -scaleShift;
            }
        } else {
            if (scaleShift >= 0) {
                result += curr.w << scaleShift;
            } else {
                result += curr.w >> -scaleShift;
            }
        }

        ++str;
    }

    return result;
}