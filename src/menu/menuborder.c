#include "menuborder.h"
#include "graphics/sprite.h"

struct Coloru8 gMenuBackColor = {0x56, 0x56, 0x56, 0xbf};

void menuBorderRender(struct RenderState* renderState, int x, int y, int w, int h) {

    if (h < MENU_BORDER_WIDTH * 2) {
        h = MENU_BORDER_WIDTH * 2;
    }

    if (w < MENU_BORDER_WIDTH * 2) {
        w = MENU_BORDER_WIDTH * 2;
    }


    // top left corner
    spriteDraw(
        renderState,
        LAYER_MENU_BORDER,
        x, y,
        MENU_BORDER_WIDTH, MENU_BORDER_WIDTH,
        0, 0,
        0, 0
    );

    // top
    spriteTextureRectangle(
        renderState, 
        LAYER_MENU_BORDER, 
        (x + MENU_BORDER_WIDTH) << 2, y << 2, 
        (w - MENU_BORDER_WIDTH * 2) << 2, MENU_BORDER_WIDTH << 2,
        MENU_BORDER_WIDTH << 5,
        0 << 5,
        0,
        0x400
    );

    // top details
    unsigned tileParity = 0;
    for (unsigned i = 40; i + MENU_BORDER_WIDTH < w; i += 40) {
        spriteDraw(
            renderState,
            LAYER_MENU_BORDER,
            x + i, y,
            MENU_BORDER_WIDTH, MENU_BORDER_WIDTH,
            MENU_BORDER_WIDTH * (2 + tileParity), 0,
            0, 0
        );  
        tileParity ^= 1;
    }

    // top right corner
    spriteDraw(
        renderState,
        LAYER_MENU_BORDER,
        x + w - MENU_BORDER_WIDTH, y,
        MENU_BORDER_WIDTH, MENU_BORDER_WIDTH,
        MENU_BORDER_WIDTH * 4, 0,
        0, 0
    );

    // left
    spriteTextureRectangle(
        renderState, 
        LAYER_MENU_BORDER, 
        x << 2,  (y + MENU_BORDER_WIDTH) << 2, 
        MENU_BORDER_WIDTH << 2, (h - MENU_BORDER_WIDTH * 2) << 2,
        MENU_BORDER_WIDTH << 5,
        MENU_BORDER_WIDTH << 5,
        0x400,
        0
    );

    // left details
    tileParity = 0;
    for (unsigned i = 30; i + MENU_BORDER_WIDTH < h; i += 30) {
        spriteDraw(
            renderState,
            LAYER_MENU_BORDER,
            x, y + i,
            MENU_BORDER_WIDTH, MENU_BORDER_WIDTH,
            MENU_BORDER_WIDTH * (2 + tileParity), MENU_BORDER_WIDTH,
            0, 0
        );  
        tileParity ^= 1;
    }

    // bottom left corner
    spriteDraw(
        renderState,
        LAYER_MENU_BORDER,
        x, y + h - MENU_BORDER_WIDTH,
        MENU_BORDER_WIDTH, MENU_BORDER_WIDTH,
        MENU_BORDER_WIDTH * 4, MENU_BORDER_WIDTH,
        0, 0
    );

    // TODO details in border

    spriteSetColor(renderState, LAYER_SOLID_COLOR, gMenuBackColor);
    spriteSolid(renderState, LAYER_SOLID_COLOR, x + MENU_BORDER_WIDTH, y + MENU_BORDER_WIDTH, w - MENU_BORDER_WIDTH, h - MENU_BORDER_WIDTH);
}