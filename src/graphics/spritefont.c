
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
        font->characters[chars[i].character] = chars[i].data;
    }
}

void setFontColor(struct Font* font, char r, char g, char b)
{

}

void fontRenderText(struct RenderState* renderState, struct Font* font, const char* str, int x, int y, int scaleShift)
{
    int startX = x;

    while (*str)
    {
        struct SpriteTile curr = font->characters[*str];
        if (curr.w)
        {
            spriteDraw(renderState, font->spriteLayer, x, y, curr.w, curr.h, curr.x, curr.y, scaleShift, scaleShift);
            x += curr.w << scaleShift;
        }
        else if (*str == ' ')
        {
            x += font->spaceWidth << scaleShift;
        }
        else if (*str == '\n')
        {
            x = startX;
            y += 20;
        }

        ++str;
    }
}