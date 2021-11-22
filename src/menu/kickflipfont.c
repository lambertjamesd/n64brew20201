#include "kickflipfont.h"

struct CharacterDefinition gKickflipFontDef[] = {
    {'A', LAYER_KICKFLIP_FONT, {0, 0, 15, 16}},
    {'B', LAYER_KICKFLIP_FONT, {15, 0, 15, 16}},
    {'C', LAYER_KICKFLIP_FONT, {30, 0, 15, 16}},
    {'D', LAYER_KICKFLIP_FONT, {45, 0, 15, 16}},

    {'E', LAYER_KICKFLIP_FONT, {0, 16, 15, 16}},
    {'F', LAYER_KICKFLIP_FONT, {15, 16, 15, 16}},
    {'G', LAYER_KICKFLIP_FONT, {30, 16, 15, 16}},
    {'H', LAYER_KICKFLIP_FONT, {45, 16, 15, 16}},

    {'J', LAYER_KICKFLIP_FONT, {0, 32, 15, 16}},
    {'K', LAYER_KICKFLIP_FONT, {15, 32, 15, 16}},
    {'L', LAYER_KICKFLIP_FONT, {30, 32, 15, 16}},
    {'N', LAYER_KICKFLIP_FONT, {45, 32, 15, 16}},

    {'O', LAYER_KICKFLIP_FONT, {0, 48, 15, 16}},
    {'P', LAYER_KICKFLIP_FONT, {15, 48, 15, 16}},
    {'R', LAYER_KICKFLIP_FONT, {30, 48, 15, 16}},
    {'S', LAYER_KICKFLIP_FONT, {45, 48, 15, 16}},

    {'T', LAYER_KICKFLIP_FONT, {0, 64, 15, 16}},
    {'U', LAYER_KICKFLIP_FONT, {15, 64, 15, 16}},
    {'V', LAYER_KICKFLIP_FONT, {30, 64, 15, 16}},
    {'Y', LAYER_KICKFLIP_FONT, {45, 64, 15, 16}},

    {'W', LAYER_KICKFLIP_FONT, {0, 80, 24, 16}},
    {'X', LAYER_KICKFLIP_FONT, {24, 80, 16, 16}},
    {'Q', LAYER_KICKFLIP_FONT, {40, 80, 16, 16}},

    {'Z', LAYER_KICKFLIP_FONT, {0, 96, 15, 16}},
    {'?', LAYER_KICKFLIP_FONT, {15, 96, 15, 16}},
    {'I', LAYER_KICKFLIP_FONT, {30, 96, 6, 16}},
    {'M', LAYER_KICKFLIP_FONT, {36, 96, 26, 16}},
    
    {'\'', LAYER_KICKFLIP_FONT, {0, 112, 6, 16}},

    {'a', LAYER_KICKFLIP_FONT, {0, 0, 15, 16}},
    {'b', LAYER_KICKFLIP_FONT, {15, 0, 15, 16}},
    {'c', LAYER_KICKFLIP_FONT, {30, 0, 15, 16}},
    {'d', LAYER_KICKFLIP_FONT, {45, 0, 15, 16}},

    {'e', LAYER_KICKFLIP_FONT, {0, 16, 15, 16}},
    {'f', LAYER_KICKFLIP_FONT, {15, 16, 15, 16}},
    {'g', LAYER_KICKFLIP_FONT, {30, 16, 15, 16}},
    {'h', LAYER_KICKFLIP_FONT, {45, 16, 15, 16}},

    {'j', LAYER_KICKFLIP_FONT, {0, 32, 15, 16}},
    {'k', LAYER_KICKFLIP_FONT, {15, 32, 15, 16}},
    {'l', LAYER_KICKFLIP_FONT, {30, 32, 15, 16}},
    {'n', LAYER_KICKFLIP_FONT, {45, 32, 15, 16}},

    {'o', LAYER_KICKFLIP_FONT, {0, 48, 15, 16}},
    {'p', LAYER_KICKFLIP_FONT, {15, 48, 15, 16}},
    {'r', LAYER_KICKFLIP_FONT, {30, 48, 15, 16}},
    {'s', LAYER_KICKFLIP_FONT, {45, 48, 15, 16}},

    {'t', LAYER_KICKFLIP_FONT, {0, 64, 15, 16}},
    {'u', LAYER_KICKFLIP_FONT, {15, 64, 15, 16}},
    {'v', LAYER_KICKFLIP_FONT, {30, 64, 15, 16}},
    {'y', LAYER_KICKFLIP_FONT, {45, 64, 15, 16}},

    {'w', LAYER_KICKFLIP_FONT, {0, 80, 24, 16}},
    {'x', LAYER_KICKFLIP_FONT, {24, 80, 16, 16}},
    {'q', LAYER_KICKFLIP_FONT, {40, 80, 16, 16}},

    {'z', LAYER_KICKFLIP_FONT, {0, 96, 15, 16}},
    {'i', LAYER_KICKFLIP_FONT, {30, 96, 6, 16}},
    {'m', LAYER_KICKFLIP_FONT, {36, 96, 26, 16}},

    {'0', LAYER_KICKFLIP_NUMBERS_FONT, {0, 0, 15, 16}},
    {'1', LAYER_KICKFLIP_NUMBERS_FONT, {15, 0, 6, 16}},
    {'2', LAYER_KICKFLIP_NUMBERS_FONT, {21, 0, 15, 16}},
    {'3', LAYER_KICKFLIP_NUMBERS_FONT, {36, 0, 15, 16}},

    {'4', LAYER_KICKFLIP_NUMBERS_FONT, {0, 16, 15, 16}},
    {'5', LAYER_KICKFLIP_NUMBERS_FONT, {15, 16, 15, 16}},
    {'6', LAYER_KICKFLIP_NUMBERS_FONT, {30, 16, 15, 16}},
    {'7', LAYER_KICKFLIP_NUMBERS_FONT, {45, 16, 15, 16}},

    {'8', LAYER_KICKFLIP_NUMBERS_FONT, {0, 32, 15, 16}},
    {'9', LAYER_KICKFLIP_NUMBERS_FONT, {15, 32, 15, 16}},
    {'!', LAYER_KICKFLIP_NUMBERS_FONT, {30, 32, 6, 16}},
    {'.', LAYER_KICKFLIP_NUMBERS_FONT, {36, 32, 6, 16}},
    {':', LAYER_KICKFLIP_NUMBERS_FONT, {42, 32, 6, 16}},
};

struct Font gKickflipFont;

void initKickflipFont() {
    fontInit(&gKickflipFont, 12, gKickflipFontDef, sizeof(gKickflipFontDef) / sizeof(gKickflipFontDef[0]));
}