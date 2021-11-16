#ifndef _MENU_BASE_COMMAND_MENU_H
#define _MENU_BASE_COMMAND_MENU_H

#include "scene/levelbase.h"
#include "graphics/render_state.h"

enum BaseCommandMenuFlags {
    BaseCommandMenuFlagsShowingOpenCommand = (1 << 0),
    BaseCommandMenuFlagsShowingMenu = (1 << 1),
    BaseCommandMenuFlagsShowingUpgrades = (1 << 2),
    BaseCommandMenuFlagsForceHideOpenCommand = (1 << 3),
    BaseCommandMenuFlagsAnimating = (1 << 4),
};

struct BaseCommandMenu {
    struct LevelBase* forBase;
    unsigned short flags;
    char selectedCommand;
    char selectedUpgrade;
    float openAnimation;
};

void baseCommandMenuInit(struct BaseCommandMenu* menu);
void baseCommandMenuShowOpenCommand(struct BaseCommandMenu* menu, struct LevelBase* forBase);
void baseCommandMenuHideOpenCommand(struct BaseCommandMenu* menu);

void baseCommandMenuShow(struct BaseCommandMenu* menu, struct LevelBase* forBase);
void baseCommandMenuHide(struct BaseCommandMenu* menu);
int baseCommandMenuIsShowing(struct BaseCommandMenu* menu);
void baseCommandMenuUpdate(struct BaseCommandMenu* menu, unsigned team);

void baseCommandMenuRender(struct BaseCommandMenu* menu, struct RenderState* renderState, unsigned short* screenPos);

#endif