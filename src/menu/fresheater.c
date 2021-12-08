
#include "fresheater.h"

struct CharacterDefinition gFreshEaterCharacters[] = {
    {'U', LAYER_FRESH_EATERS, {1, 0, 17, 30}, 2},
    {'L', LAYER_FRESH_EATERS, {19, 0, 15, 30}, -3},
    {'T', LAYER_FRESH_EATERS, {40, 0, 16, 30}, 1},
    {'A', LAYER_FRESH_EATERS, {62, 0, 18, 30}, 2},
    {'R', LAYER_FRESH_EATERS, {86, 0, 17, 30}, 1},
    {'E', LAYER_FRESH_EATERS, {109, 0, 15, 30}},
};

struct Font gFreshEater;

void initFreshEaterFont() {
    fontInit(&gFreshEater, 5, 30, gFreshEaterCharacters, sizeof(gFreshEaterCharacters) / sizeof(gFreshEaterCharacters[0]));
}