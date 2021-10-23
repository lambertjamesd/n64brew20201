
#include "basecommandmenu.h"
#include "graphics/sprite.h"
#include "scene/minion.h"

#define OPEN_MENU_BOTTOM_OFFSET 32

struct SpriteTile gBaseCommandTiles[MinionCommandCount] = {
    {48, 0, 16, 16},// MinionCommandFollow,
    {32, 0, 16, 16},// MinionCommandAttack,
    {32, 16, 16, 16},// MinionCommandDefend,
};

void baseCommandMenuInit(struct BaseCommandMenu* menu) {
    menu->flags = 0;
    menu->forBase = 0;
}

void baseCommandMenuShowOpenCommand(struct BaseCommandMenu* menu, struct LevelBase* forBase) {
    menu->flags |= BaseCommandMenuFlagsShowingOpenCommand;
    menu->forBase = forBase;
}

void baseCommandMenuHideOpenCommand(struct BaseCommandMenu* menu) {
    menu->flags &= ~BaseCommandMenuFlagsShowingOpenCommand;
}

void baseCommandMenuShow(struct BaseCommandMenu* menu, struct LevelBase* forBase) {
    menu->flags |= BaseCommandMenuFlagsShowingMenu;
    menu->forBase = forBase;
}

void baseCommandMenuHide(struct BaseCommandMenu* menu) {
    menu->flags &= ~BaseCommandMenuFlagsShowingMenu;
}

int baseCommandMenuIsShowing(struct BaseCommandMenu* menu) {
    return (menu->flags & BaseCommandMenuFlagsShowingMenu) != 0;
}

void baseCommandMenuUpdate(struct BaseCommandMenu* menu, unsigned team) {

}

void baseCommandMenuRender(struct BaseCommandMenu* menu, struct RenderState* renderState, unsigned screenL, unsigned screenT, unsigned screenR, unsigned screenB) {
    if (menu->flags & BaseCommandMenuFlagsShowingMenu && menu->forBase) {

    } else if (menu->flags & BaseCommandMenuFlagsShowingOpenCommand && menu->forBase) {
        unsigned screenCenter = (screenL + screenR) >> 1;

        struct SpriteTile spriteTile;
        spriteTile.x = 16;
        spriteTile.y = 0;
        spriteTile.w = 16;
        spriteTile.h = 16;
        spriteDrawTile(
            renderState, 
            LAYER_COMMAND_BUTTONS, 
            screenCenter - 24, 
            screenB - OPEN_MENU_BOTTOM_OFFSET -  16, 
            16, 
            16, 
            spriteTile
        );

        spriteTile.x = 16;
        spriteTile.y = 0;
        spriteDrawTile(
            renderState, 
            LAYER_COMMAND_BUTTONS, 
            screenCenter - 8, 
            screenB - OPEN_MENU_BOTTOM_OFFSET -  16, 
            16, 
            16, 
            gBaseCommandTiles[menu->forBase->defaultComand]
        );
    }
}