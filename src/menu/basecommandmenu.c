
#include "basecommandmenu.h"
#include "graphics/sprite.h"
#include "scene/minion.h"
#include "controls/controller.h"
#include "scene/playerinput.h"
#include "gbfont.h"

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
    menu->selectedCommand = forBase->defaultComand;
}

void baseCommandMenuHide(struct BaseCommandMenu* menu) {
    menu->flags &= ~BaseCommandMenuFlagsShowingMenu;
}

int baseCommandMenuIsShowing(struct BaseCommandMenu* menu) {
    return (menu->flags & BaseCommandMenuFlagsShowingMenu) != 0;
}

void baseCommandMenuUpdate(struct BaseCommandMenu* menu, unsigned team) {
    if (menu->flags & BaseCommandMenuFlagsShowingMenu) {
        if (!menu->forBase) {
            baseCommandMenuHide(menu);
            return;
        }

        unsigned short playerInput = playerInputMapActionFlags(controllerGetButtonDown(team, ~0));

        if (controllerGetButtonUp(team, B_BUTTON) != 0 || (playerInput & PlayerInputActionsCommandOpenBaseMenu) != 0) {
            baseCommandMenuHide(menu);
            return;
        }

        if (playerInput & PlayerInputActionsCommandRecall) {
            levelBaseSetDefaultCommand(menu->forBase, MinionCommandFollow);
            baseCommandMenuHide(menu);
            return;
        }

        if (playerInput & PlayerInputActionsCommandAttack) {
            levelBaseSetDefaultCommand(menu->forBase, MinionCommandAttack);
            baseCommandMenuHide(menu);
            return;
        }

        if (playerInput & PlayerInputActionsCommandDefend) {
            levelBaseSetDefaultCommand(menu->forBase, MinionCommandDefend);
            baseCommandMenuHide(menu);
            return;
        }

        enum ControllerDirection direction = controllerGetDirectionDown(team);

        if ((direction & ControllerDirectionUp) != 0 && menu->selectedCommand > MinionCommandFollow) {
            --menu->selectedCommand;
        }

        if ((direction & ControllerDirectionDown) != 0 && menu->selectedCommand < MinionCommandDefend) {
            ++menu->selectedCommand;
        }

        if (controllerGetButtonUp(team, A_BUTTON) != 0) {
            levelBaseSetDefaultCommand(menu->forBase, menu->selectedCommand);
            baseCommandMenuHide(menu);
            return;
        }
    }
}

void baseCommandMenuRender(struct BaseCommandMenu* menu, struct RenderState* renderState, unsigned screenL, unsigned screenT, unsigned screenR, unsigned screenB) {
    if (menu->flags & BaseCommandMenuFlagsShowingMenu && menu->forBase) {
        unsigned horizontalCenter = (screenL + screenR) >> 1;
        unsigned verticalCenter = (screenT + screenB) >> 1;

        spriteSolid(
            renderState,
            LAYER_SOLID_COLOR,
            horizontalCenter - 56,
            verticalCenter - 32,
            112,
            64
        );

        spriteDrawTile(
            renderState, 
            LAYER_COMMAND_BUTTONS, 
            horizontalCenter - 32, 
            verticalCenter - 24, 
            16, 
            16, 
            gBaseCommandTiles[MinionCommandFollow]
        );
        fontRenderText(renderState, &gGBFont, "Follow", horizontalCenter - 8, verticalCenter - 20, 0);

        spriteDrawTile(
            renderState, 
            LAYER_COMMAND_BUTTONS, 
            horizontalCenter - 32, 
            verticalCenter - 8, 
            16, 
            16, 
            gBaseCommandTiles[MinionCommandAttack]
        );
        fontRenderText(renderState, &gGBFont, "Attack", horizontalCenter - 8, verticalCenter - 4, 0);

        spriteDrawTile(
            renderState, 
            LAYER_COMMAND_BUTTONS, 
            horizontalCenter - 32, 
            verticalCenter + 8, 
            16, 
            16, 
            gBaseCommandTiles[MinionCommandDefend]
        );
        fontRenderText(renderState, &gGBFont, "Defend", horizontalCenter - 8, verticalCenter + 12, 0);

        struct SpriteTile spriteTile;
        spriteTile.x = 48;
        spriteTile.y = 16;
        spriteTile.w = 16;
        spriteTile.h = 16;
        spriteDrawTile(
            renderState, 
            LAYER_COMMAND_BUTTONS, 
            horizontalCenter - 48, 
            verticalCenter - 24 + menu->selectedCommand * 16, 
            16, 
            16, 
            spriteTile
        );
    } else if (menu->flags & BaseCommandMenuFlagsShowingOpenCommand && menu->forBase) {
        unsigned screenCenter = (screenL + screenR) >> 1;

        struct SpriteTile spriteTile;
        spriteTile.x = 0;
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