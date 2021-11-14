
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

enum LevelBaseState gBaseUpgardeIndex[] = {
    LevelBaseStateUpgradingSpawnRate,
    LevelBaseStateUpgradingCapacity,
    LevelBaseStateUpgradingDefence,
};

struct SpriteTile gBaseUpgradeTiles[MinionCommandCount] = {
    {0, 16, 16, 16},// LevelBaseStateUpgradingSpawnRate,
    {0, 0, 16, 16},// LevelBaseStateUpgradingCapacity,
    {16, 0, 16, 16},// LevelBaseStateUpgradingDefence,
};

unsigned baseCommandStateToIndex(enum LevelBaseState state) {
    switch (state) {
        case LevelBaseStateUpgradingSpawnRate:
            return 0;
        case LevelBaseStateUpgradingCapacity:
            return 1;
        case LevelBaseStateUpgradingDefence:
            return 2;
        default: 
            break;
    }
    return 3;
}

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
    menu->flags &= ~BaseCommandMenuFlagsShowingUpgrades;
    menu->forBase = forBase;
    menu->selectedCommand = forBase->defaultComand;
    menu->selectedUpgrade = 0;
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
            levelBaseSetDefaultCommand(menu->forBase, MinionCommandFollow, team);
            baseCommandMenuHide(menu);
            return;
        }

        if (playerInput & PlayerInputActionsCommandAttack) {
            levelBaseSetDefaultCommand(menu->forBase, MinionCommandAttack, team);
            baseCommandMenuHide(menu);
            return;
        }

        if (playerInput & PlayerInputActionsCommandDefend) {
            levelBaseSetDefaultCommand(menu->forBase, MinionCommandDefend, team);
            baseCommandMenuHide(menu);
            return;
        }

        enum ControllerDirection direction = controllerGetDirectionDown(team);

        if (menu->flags & BaseCommandMenuFlagsShowingUpgrades) {
            if ((direction & ControllerDirectionUp) != 0 && menu->selectedUpgrade > 0) {
                --menu->selectedUpgrade;
            }

            if ((direction & ControllerDirectionDown) != 0 && menu->selectedUpgrade < 2) {
                ++menu->selectedUpgrade;
            }

            if (controllerGetButtonUp(team, A_BUTTON) != 0) {
                levelBaseStartUpgrade(menu->forBase, gBaseUpgardeIndex[(unsigned)menu->selectedUpgrade]);
                baseCommandMenuHide(menu);
                return;
            }
        } else {
            if ((direction & ControllerDirectionUp) != 0 && menu->selectedCommand > MinionCommandFollow) {
                --menu->selectedCommand;
            }

            if ((direction & ControllerDirectionDown) != 0 && menu->selectedCommand < MinionCommandDefend) {
                ++menu->selectedCommand;
            }

            if (controllerGetButtonUp(team, A_BUTTON) != 0) {
                levelBaseSetDefaultCommand(menu->forBase, menu->selectedCommand, team);
                baseCommandMenuHide(menu);
                return;
            }
        }

        if (direction & (ControllerDirectionLeft | ControllerDirectionRight)) {
            menu->flags ^= BaseCommandMenuFlagsShowingUpgrades;
        }
    }
}

void baseCommandMenuRenderCommandSelect(struct BaseCommandMenu* menu, struct RenderState* renderState, unsigned horizontalCenter, unsigned verticalCenter) {
    spriteSetColor(renderState, LAYER_SOLID_COLOR, gHalfTransparentBlack);
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
}

void baseCommandMenuRenderUpgradeSelect(struct BaseCommandMenu* menu, struct RenderState* renderState, unsigned horizontalCenter, unsigned verticalCenter) {
    spriteSetColor(renderState, LAYER_SOLID_COLOR, gHalfTransparentBlack);
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
        LAYER_UPGRADE_ICONS, 
        horizontalCenter - 32, 
        verticalCenter - 24, 
        16, 
        16, 
        gBaseUpgradeTiles[0]
    );
    fontRenderText(renderState, &gGBFont, "Wait", horizontalCenter - 8, verticalCenter - 20, 0);

    spriteDrawTile(
        renderState, 
        LAYER_UPGRADE_ICONS, 
        horizontalCenter - 32, 
        verticalCenter - 8, 
        16, 
        16, 
        gBaseUpgradeTiles[1]
    );
    fontRenderText(renderState, &gGBFont, "Minion", horizontalCenter - 8, verticalCenter - 4, 0);

    spriteDrawTile(
        renderState, 
        LAYER_UPGRADE_ICONS, 
        horizontalCenter - 32, 
        verticalCenter + 8, 
        16, 
        16, 
        gBaseUpgradeTiles[2]
    );
    fontRenderText(renderState, &gGBFont, "Defense", horizontalCenter - 8, verticalCenter + 12, 0);

    struct SpriteTile spriteTile;
    spriteTile.x = 48;
    spriteTile.y = 16;
    spriteTile.w = 16;
    spriteTile.h = 16;
    spriteDrawTile(
        renderState, 
        LAYER_COMMAND_BUTTONS, 
        horizontalCenter - 48, 
        verticalCenter - 24 + menu->selectedUpgrade * 16, 
        16, 
        16, 
        spriteTile
    );
}

void baseCommandMenuRender(struct BaseCommandMenu* menu, struct RenderState* renderState, unsigned short* screenPos) {
    if (menu->flags & BaseCommandMenuFlagsShowingMenu && menu->forBase) {
        unsigned horizontalCenter = (screenPos[0] + screenPos[2]) >> 1;
        unsigned verticalCenter = (screenPos[1] + screenPos[3]) >> 1;
        if (menu->flags & BaseCommandMenuFlagsShowingUpgrades) {
            baseCommandMenuRenderUpgradeSelect(menu, renderState, horizontalCenter, verticalCenter);
        } else {
            baseCommandMenuRenderCommandSelect(menu, renderState, horizontalCenter, verticalCenter);
        }

        // LAYER_UPGRADE_ICONS
    } else if ((menu->flags & BaseCommandMenuFlagsShowingOpenCommand) && !(menu->flags & BaseCommandMenuFlagsForceHideOpenCommand) && menu->forBase) {
        unsigned screenCenter = (screenPos[2] + screenPos[0]) >> 1;

        struct SpriteTile spriteTile;
        spriteTile.x = 0;
        spriteTile.y = 0;
        spriteTile.w = 16;
        spriteTile.h = 16;
        spriteDrawTile(
            renderState, 
            LAYER_COMMAND_BUTTONS, 
            screenCenter - 24, 
            screenPos[3] - OPEN_MENU_BOTTOM_OFFSET -  16, 
            16, 
            16, 
            spriteTile
        );

        spriteDrawTile(
            renderState, 
            LAYER_COMMAND_BUTTONS, 
            screenCenter - 8, 
            screenPos[3] - OPEN_MENU_BOTTOM_OFFSET -  16, 
            16, 
            16, 
            gBaseCommandTiles[menu->forBase->defaultComand]
        );
    }
}