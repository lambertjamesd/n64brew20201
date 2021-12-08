
#include "basecommandmenu.h"
#include "graphics/sprite.h"
#include "scene/minion.h"
#include "controls/controller.h"
#include "scene/playerinput.h"
#include "menuborder.h"
#include "kickflipfont.h"
#include "math/mathf.h"
#include "util/time.h"

#define OPEN_MENU_BOTTOM_OFFSET 32
#define MENU_WIDTH  110
#define MENU_HEIGHT 70
#define OPEN_ANIMATION_TIME 0.2f

struct Coloru8 gDisabledTextColor = {40, 30, 30, 255};

struct SpriteTile gBaseCommandTiles[] = {
    {16, 16, 16, 16},// MinionCommandFollow,
    {0, 0, 16, 16},// MinionCommandAttack,
    {0, 16, 16, 16},// MinionCommandDefend,
    {16, 0, 16, 16},
};

struct SpriteTile gBaseShortcutTiles[] = {
    {16, 32, 16, 16},
    {0, 32, 16, 16,},
    {16, 16, 16, 16},
    {16, 48, 16, 16},
};

char* gMenuPrompts[] = {
    "Follow",
    "Attack",
    "Defend",
    "Upgrade",
};

enum LevelBaseState gBaseUpgardeIndex[] = {
    LevelBaseStateUpgradingSpawnRate,
    LevelBaseStateUpgradingCapacity,
    LevelBaseStateUpgradingDefence,
};

struct SpriteTile gBaseUpgradeTiles[MinionCommandCount] = {
    {0, 32, 16, 16},// LevelBaseStateUpgradingSpawnRate,
    {0, 48, 16, 16},// LevelBaseStateUpgradingCapacity,
    {16, 32, 16, 16},// LevelBaseStateUpgradingDefence,
};

char* gUpgradePrompts[] = {
    "Speed",
    "Capacity",
    "Defense",
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
    menu->lastControllerDown = 0;
    menu->lastButtonDown = 0;
}

void baseCommandMenuShowOpenCommand(struct BaseCommandMenu* menu, struct LevelBase* forBase) {
    menu->flags |= BaseCommandMenuFlagsShowingOpenCommand;
    menu->forBase = forBase;
}

void baseCommandMenuHideOpenCommand(struct BaseCommandMenu* menu) {
    menu->flags &= ~BaseCommandMenuFlagsShowingOpenCommand;
}

void baseCommandMenuShow(struct BaseCommandMenu* menu, struct LevelBase* forBase) {
    menu->flags |= BaseCommandMenuFlagsShowingMenu | BaseCommandMenuFlagsAnimating;
    menu->flags &= ~BaseCommandMenuFlagsShowingUpgrades;
    menu->forBase = forBase;
    menu->selectedCommand = forBase->defaultComand;
    menu->selectedUpgrade = 0;
    menu->openAnimation = OPEN_ANIMATION_TIME;
    menu->lastControllerDown = 0;
    menu->lastButtonDown = 0;
}

void baseCommandMenuHide(struct BaseCommandMenu* menu) {
    if (menu->flags & BaseCommandMenuFlagsShowingMenu) {
        menu->flags &= ~BaseCommandMenuFlagsShowingMenu;
        menu->flags |= BaseCommandMenuFlagsAnimating;
    }
}

int baseCommandMenuIsShowing(struct BaseCommandMenu* menu) {
    return (menu->flags & BaseCommandMenuFlagsShowingMenu) != 0;
}

void baseCommandMenuUpdate(struct BaseCommandMenu* menu, unsigned team) {
    if (menu->flags & BaseCommandMenuFlagsAnimating) {
        if (menu->flags & BaseCommandMenuFlagsShowingMenu) {
            menu->openAnimation -= gTimeDelta;

            if (menu->openAnimation <= 0.0f) {
                menu->openAnimation = 0.0f;
                menu->flags &= ~BaseCommandMenuFlagsAnimating;
            }
        } else {
            menu->openAnimation += gTimeDelta;

            if (menu->openAnimation > OPEN_ANIMATION_TIME) {
                menu->openAnimation = 0.0f;
                menu->flags &= ~BaseCommandMenuFlagsAnimating;
            }
        }
    }

    if (menu->flags & BaseCommandMenuFlagsShowingMenu) {
        if (!menu->forBase) {
            baseCommandMenuHide(menu);
            return;
        }

        unsigned short buttons = controllerGetButton(team, ~0);
        unsigned short lastButtons = menu->lastButtonDown;
        menu->lastButtonDown = buttons;

        unsigned short actionDown = playerInputMapActionFlags(buttons & ~lastButtons);
        unsigned short buttonUp = ~buttons & lastButtons;

        if ((buttonUp & B_BUTTON) != 0) {
            if (menu->flags & BaseCommandMenuFlagsShowingUpgrades) {
                menu->flags ^= BaseCommandMenuFlagsShowingUpgrades;
            } else {
                baseCommandMenuHide(menu);
                return;
            }
        }

        if (actionDown & PlayerInputActionsCommandRecall) {
            levelBaseSetDefaultCommand(menu->forBase, MinionCommandFollow, team);
            baseCommandMenuHide(menu);
            return;
        }

        if (actionDown & PlayerInputActionsCommandAttack) {
            levelBaseSetDefaultCommand(menu->forBase, MinionCommandAttack, team);
            baseCommandMenuHide(menu);
            return;
        }

        if (actionDown & PlayerInputActionsCommandDefend) {
            levelBaseSetDefaultCommand(menu->forBase, MinionCommandDefend, team);
            baseCommandMenuHide(menu);
            return;
        }

        enum ControllerDirection direction = controllerGetDirection(team);
        enum ControllerDirection directionDown = direction & ~menu->lastControllerDown;
        menu->lastControllerDown = direction;

        if (menu->flags & BaseCommandMenuFlagsShowingUpgrades) {
            if ((directionDown & ControllerDirectionUp) != 0 && menu->selectedUpgrade > 0) {
                --menu->selectedUpgrade;
            }

            if ((directionDown & ControllerDirectionDown) != 0 && menu->selectedUpgrade < 2) {
                ++menu->selectedUpgrade;
            }

            unsigned canUpgrade = !levelBaseIsBeingUpgraded(menu->forBase) && levelBaseTimeForUpgrade(menu->forBase, gBaseUpgardeIndex[(unsigned)menu->selectedUpgrade]) >= 0.0f;

            if (canUpgrade && (buttonUp & A_BUTTON) != 0) {
                levelBaseStartUpgrade(menu->forBase, gBaseUpgardeIndex[(unsigned)menu->selectedUpgrade]);
                baseCommandMenuHide(menu);
                return;
            }
        } else {
            if ((directionDown & ControllerDirectionUp) != 0 && menu->selectedCommand > MinionCommandFollow) {
                --menu->selectedCommand;
            }

            if ((directionDown & ControllerDirectionDown) != 0 && menu->selectedCommand <= MinionCommandDefend) {
                ++menu->selectedCommand;
            }

            if ((buttonUp & A_BUTTON) != 0) {
                if (menu->selectedCommand >= MinionCommandCount) {
                    menu->flags ^= BaseCommandMenuFlagsShowingUpgrades;
                } else {
                    levelBaseSetDefaultCommand(menu->forBase, menu->selectedCommand, team);
                    baseCommandMenuHide(menu);
                }
                return;
            }
        }

        if (directionDown & (ControllerDirectionLeft | ControllerDirectionRight)) {
            menu->flags ^= BaseCommandMenuFlagsShowingUpgrades;
        }
    }
}

void baseCommandMenuRenderCommandSelect(struct BaseCommandMenu* menu, struct RenderState* renderState, unsigned horizontalCenter, unsigned verticalCenter) {
    unsigned x = horizontalCenter - MENU_WIDTH / 2 + MENU_BORDER_WIDTH + 4;
    unsigned y = verticalCenter - MENU_HEIGHT / 2 + MENU_BORDER_WIDTH;
    for (unsigned i = 0; i < sizeof(gMenuPrompts)/sizeof(gMenuPrompts[0]); ++i) {
        unsigned textX = x;
        if (menu->selectedCommand == i) {
            spriteDrawTile(renderState, LAYER_COMMAND_BUTTONS, x, y, 16, 16, gBaseCommandTiles[i]);
            textX += 16;
        }
        spriteDrawTile(renderState, LAYER_BUTTONS, horizontalCenter + MENU_WIDTH / 2 - 16 - MENU_BORDER_WIDTH, y, 16, 16, gBaseShortcutTiles[i]);
        fontRenderText(renderState, &gKickflipFont, gMenuPrompts[i], textX + 4, y + 4, -1);
        y += 16;
    }
}

void baseCommandMenuRenderUpgradeSelect(struct BaseCommandMenu* menu, struct RenderState* renderState, unsigned horizontalCenter, unsigned verticalCenter) {
    unsigned x = horizontalCenter - MENU_WIDTH / 2 + MENU_BORDER_WIDTH + 4;
    unsigned y = verticalCenter - MENU_HEIGHT / 2 + MENU_BORDER_WIDTH;
    for (unsigned i = 0; i < sizeof(gUpgradePrompts)/sizeof(gUpgradePrompts[0]); ++i) {
        unsigned canUpgrade = !levelBaseIsBeingUpgraded(menu->forBase) && levelBaseTimeForUpgrade(menu->forBase, gBaseUpgardeIndex[i]) >= 0.0f;

        spriteSetColor(renderState, LAYER_KICKFLIP_FONT, canUpgrade ? gColorWhite : gDisabledTextColor);
        spriteSetColor(renderState, LAYER_COMMAND_BUTTONS, canUpgrade ? gColorWhite : gColorBlack);
        
        unsigned textX = x;
        if (menu->selectedUpgrade == i) {
            spriteDrawTile(renderState, LAYER_COMMAND_BUTTONS, x, y, 16, 16, gBaseUpgradeTiles[i]);
            textX += 16;
        }
        fontRenderText(renderState, &gKickflipFont, gUpgradePrompts[i], textX + 4, y + 4, -1);
        y += 16;
    }
}

void baseCommandMenuRender(struct BaseCommandMenu* menu, struct RenderState* renderState, unsigned short* screenPos) {
    unsigned horizontalCenter = (screenPos[0] + screenPos[2]) >> 1;
    unsigned promptScreenY = screenPos[3] - OPEN_MENU_BOTTOM_OFFSET -  16;
    unsigned verticalCenter = (screenPos[1] + screenPos[3]) >> 1;

    if (menu->flags & BaseCommandMenuFlagsAnimating) {
        float lerp = menu->openAnimation / OPEN_ANIMATION_TIME;
        menuBorderRender(
            renderState, 
            (int)mathfLerp(horizontalCenter - MENU_WIDTH / 2, horizontalCenter - 30, lerp), 
            (int)mathfLerp(verticalCenter - MENU_HEIGHT / 2, promptScreenY - MENU_BORDER_WIDTH - 4, lerp), 
            (int)mathfLerp(MENU_WIDTH, 60, lerp), 
            (int)mathfLerp(MENU_HEIGHT, 16 + MENU_BORDER_WIDTH + 8, lerp)
        );

        spriteDrawTile(
            renderState, 
            LAYER_COMMAND_BUTTONS, 
            (int)mathfLerp(horizontalCenter - MENU_WIDTH / 2 + MENU_BORDER_WIDTH + 4, horizontalCenter + 4, lerp), 
            (int)mathfLerp(verticalCenter - MENU_HEIGHT / 2 + MENU_BORDER_WIDTH + menu->selectedCommand * 16, promptScreenY, lerp),
            16, 
            16, 
            gBaseCommandTiles[menu->forBase->defaultComand]
        );
    } else if (menu->flags & BaseCommandMenuFlagsShowingMenu && menu->forBase) {
        menuBorderRender(renderState, horizontalCenter - MENU_WIDTH / 2, verticalCenter - MENU_HEIGHT / 2, MENU_WIDTH, MENU_HEIGHT);

        if (menu->flags & BaseCommandMenuFlagsShowingUpgrades) {
            baseCommandMenuRenderUpgradeSelect(menu, renderState, horizontalCenter, verticalCenter);
        } else {
            baseCommandMenuRenderCommandSelect(menu, renderState, horizontalCenter, verticalCenter);
        }
    } else if ((menu->flags & BaseCommandMenuFlagsShowingOpenCommand) && !(menu->flags & BaseCommandMenuFlagsForceHideOpenCommand) && menu->forBase) {
        menuBorderRender(renderState, horizontalCenter - 30, promptScreenY - MENU_BORDER_WIDTH - 4, 60, 16 + MENU_BORDER_WIDTH + 8);

        struct SpriteTile spriteTile;
        spriteTile.x = 0;
        spriteTile.y = 16;
        spriteTile.w = 16;
        spriteTile.h = 16;
        spriteDrawTile(
            renderState, 
            LAYER_BUTTONS, 
            horizontalCenter - 20, 
            promptScreenY, 
            16, 
            16, 
            spriteTile
        );

        spriteDrawTile(
            renderState, 
            LAYER_COMMAND_BUTTONS, 
            horizontalCenter + 4, 
            promptScreenY, 
            16, 
            16, 
            gBaseCommandTiles[menu->forBase->defaultComand]
        );
    }
}