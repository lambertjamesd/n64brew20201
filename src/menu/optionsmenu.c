#include "optionsmenu.h"
#include "menucommon.h"
#include "controls/controller.h"
#include "graphics/spritefont.h"
#include "kickflipfont.h"
#include "savefile/savefile.h"

#define MENU_X      50
#define MENU_Y      104
#define ROW_HEIGHT  24

char* gOptionsText[] = {
    "Back",
    "Erase Save Data",
};

char* gEraseText[] = {
    "Cancel",
    "Erase",
};


#define ERASE_ITEM_COUNT (sizeof(gEraseText) / sizeof(*gEraseText))
#define OPTION_MENU_ITEM_COUNT (sizeof(gOptionsText) / sizeof(*gOptionsText))

void optionsMenuInit(struct OptionsMenu* optionsMenu) {
    optionsMenu->isErasing = 0;
    optionsMenu->itemSelection = 0;
}

int optionsMenuUpdate(struct OptionsMenu* optionsMenu) {
    if (controllerGetButtonDown(0, B_BUTTON)) {
        if (optionsMenu->isErasing) {
            optionsMenu->isErasing = 0;
            optionsMenu->itemSelection = 1;
        } else {
            return 1;
        }
    }

    enum ControllerDirection dir = controllerGetDirectionDown(0);

    int currentItemCount = optionsMenu->isErasing ? ERASE_ITEM_COUNT : OPTION_MENU_ITEM_COUNT;

    if (dir & ControllerDirectionDown) {
        optionsMenu->itemSelection = MIN(currentItemCount - 1, optionsMenu->itemSelection + 1);
    } else if (dir & ControllerDirectionUp) {
        optionsMenu->itemSelection = (unsigned short)MIN(currentItemCount - 1, (int)optionsMenu->itemSelection - 1);
    }

    if (controllerGetButtonDown(0, A_BUTTON)) {
        if (optionsMenu->isErasing) {
            if (optionsMenu->itemSelection == 1) {
                saveFileErase();
                optionsMenu->isErasing = 0;
                optionsMenu->itemSelection = 1;
            } else {
                optionsMenu->isErasing = 0;
                optionsMenu->itemSelection = 1;
            }
        } else {
            if (optionsMenu->itemSelection == 0) {
                return 1;
            } else {
                optionsMenu->isErasing = 1;
                optionsMenu->itemSelection = 0;
            }
        }
    }

    return 0;
}

void optionsMenuRender(struct OptionsMenu* optionsMenu, struct RenderState* renderState) {
    char** textSource = optionsMenu->isErasing ? gEraseText : gOptionsText;
    unsigned itemCount = optionsMenu->isErasing ? ERASE_ITEM_COUNT : OPTION_MENU_ITEM_COUNT;

    for (unsigned i = 0; i < itemCount; ++i) {
        struct Coloru8 color;

        if (i == optionsMenu->itemSelection) {
            menuSelectionColor(&color);
        } else {
            color = gDeselectedColor;
        }

        spriteSetColor(renderState, LAYER_KICKFLIP_FONT, color);
        fontRenderText(renderState, &gKickflipFont, textSource[i], MENU_X, MENU_Y + i * ROW_HEIGHT, 0);
    }

    spriteSetColor(renderState, LAYER_KICKFLIP_FONT, gColorWhite);
    if (optionsMenu->isErasing) {
        fontRenderText(renderState, &gKickflipFont, "Are You Sure?", MENU_X - 10, MENU_Y - 24, 0);
    }
}