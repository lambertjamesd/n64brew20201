
#include "spritefont.h"
#include "sprite.h"

void fontInit(struct Font* font, int spaceWidth, struct CharacterDefinition* chars, int charCount)
{
    font->spaceWidth = spaceWidth;

    for (int i = 0; i < ANSI_CHAR_COUNT; ++i)
    {
        font->characters[i].data.w = 0;
    }

    for (int i = 0; i < charCount; ++i)
    {
        font->characters[(unsigned)chars[i].character] = chars[i];
    }
}

void fontRenderText(struct RenderState* renderState, struct Font* font, const char* str, int x, int y, int scaleShift)
{
    int startX = x;

    while (*str)
    {
        unsigned charValue = (unsigned)*str;
        struct CharacterDefinition* curr = &font->characters[charValue];
        if (curr->data.w)
        {
            spriteDraw(
                renderState, 
                curr->spriteLayer, 
                x, y, 
                curr->data.w, curr->data.h, 
                curr->data.x, curr->data.y, 
                scaleShift, scaleShift
            );
            if (scaleShift >= 0) {
                x += (curr->data.w + curr->kerning) << scaleShift;
            } else {
                x += (curr->data.w + curr->kerning) >> -scaleShift;
            }
        }
        else if (*str == ' ')
        {
            if (scaleShift >= 0) {
                x += (font->spaceWidth) << scaleShift;
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
        struct CharacterDefinition* curr = &font->characters[(unsigned)*str];

        if (*str == ' ') {
            if (scaleShift >= 0) {
                result += font->spaceWidth << scaleShift;
            } else {
                result += font->spaceWidth >> -scaleShift;
            }
        } else {
            if (scaleShift >= 0) {
                result += (curr->data.w + curr->kerning) << scaleShift;
            } else {
                result += (curr->data.w + curr->kerning) >> -scaleShift;
            }
        }

        ++str;
    }

    return result;
}