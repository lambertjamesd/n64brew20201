#include "optionsmenu.h"
#include "menucommon.h"
#include "controls/controller.h"
#include "graphics/spritefont.h"
#include "kickflipfont.h"
#include "savefile/savefile.h"
#include "audio/soundplayer.h"
#include "audio/clips.h"

#define MENU_X      50
#define MENU_Y      64
#define ROW_HEIGHT  24

#define BACK_OPTION 0
#define ERASE_OPTION 1
#define MUSIC_OPTION 2
#define SOUND_OPTION 3

char* gOptionsText[] = {
    "Back",
    "Erase Save Data",
    "Music",
    "Sound",
};

#define CANCEL_OPTION 0
#define ERASE_CONFIRM_OPTION 1

char* gEraseText[] = {
    "Cancel",
    "Erase",
};


#define ERASE_ITEM_COUNT (sizeof(gEraseText) / sizeof(*gEraseText))
#define OPTION_MENU_ITEM_COUNT (sizeof(gOptionsText) / sizeof(*gOptionsText))

void optionsMenuInit(struct OptionsMenu* optionsMenu) {
    optionsMenu->isErasing = 0;
    optionsMenu->itemSelection = BACK_OPTION;
}

int optionsMenuUpdate(struct OptionsMenu* optionsMenu) {
    if (controllerGetButtonDown(0, B_BUTTON)) {
        soundPlayerPlay(SOUNDS_UI_SELECT3, 1.0f, 1.0f, SoundPlayerPriorityPlayer, 0, 0);
        if (optionsMenu->isErasing) {
            optionsMenu->isErasing = 0;
            optionsMenu->itemSelection = ERASE_OPTION;
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
            if (optionsMenu->itemSelection == ERASE_CONFIRM_OPTION) {
                soundPlayerPlay(SOUNDS_CONTROLSCRAMBLE, 1.0f, 1.0f, SoundPlayerPriorityPlayer, 0, 0);
                saveFileErase();
            } else {
                soundPlayerPlay(SOUNDS_UI_SELECT3, 1.0f, 1.0f, SoundPlayerPriorityPlayer, 0, 0);
            }

            optionsMenu->isErasing = 0;
            optionsMenu->itemSelection = ERASE_OPTION;
        } else {
            if (optionsMenu->itemSelection == BACK_OPTION) {
                soundPlayerPlay(SOUNDS_UI_SELECT3, 1.0f, 1.0f, SoundPlayerPriorityPlayer, 0, 0);
                return 1;
            } else if (optionsMenu->itemSelection == ERASE_OPTION) {
                soundPlayerPlay(SOUNDS_UI_SELECT2, 1.0f, 1.0f, SoundPlayerPriorityPlayer, 0, 0);
                optionsMenu->isErasing = 1;
                optionsMenu->itemSelection = CANCEL_OPTION;
            }
        }
    }

    if (!optionsMenu->isErasing && optionsMenu->itemSelection == MUSIC_OPTION) {
        if (dir & ControllerDirectionLeft) {
            soundPlayerSetMusicVolume(soundPlayerGetMusicVolume() - 0.1f);
        } else if (dir & ControllerDirectionRight) {
            soundPlayerSetMusicVolume(soundPlayerGetMusicVolume() + 0.1f);
        }
    }

    if (!optionsMenu->isErasing && optionsMenu->itemSelection == SOUND_OPTION) {
        if (dir & ControllerDirectionLeft) {
            soundPlayerSetSoundVolume(soundPlayerGetSoundVolume() - 0.1f);
            soundPlayerPlay(SOUNDS_UI_SELECT2, 1.0f, 1.0f, SoundPlayerPriorityPlayer, 0, 0);
        } else if (dir & ControllerDirectionRight) {
            soundPlayerSetSoundVolume(soundPlayerGetSoundVolume() + 0.1f);
            soundPlayerPlay(SOUNDS_UI_SELECT2, 1.0f, 1.0f, SoundPlayerPriorityPlayer, 0, 0);
        }
    }

    return 0;
}

struct Coloru8 gSoundBarColor = {200, 160, 0, 255};

void optionsMenuRender(struct OptionsMenu* optionsMenu, struct RenderState* renderState) {
    char** textSource = optionsMenu->isErasing ? gEraseText : gOptionsText;
    unsigned itemCount = optionsMenu->isErasing ? ERASE_ITEM_COUNT : OPTION_MENU_ITEM_COUNT;

    unsigned y = MENU_Y;

    spriteSetColor(renderState, LAYER_KICKFLIP_FONT, gColorWhite);
    if (optionsMenu->isErasing) {
        fontRenderText(renderState, &gKickflipFont, "Are You Sure?", MENU_X - 10, y, 0);
        y += ROW_HEIGHT;
    } else {
        spriteSetColor(renderState, LAYER_SOLID_COLOR, gColorBlack);
        spriteSolid(renderState, LAYER_SOLID_COLOR, MENU_X + 96, MENU_Y + ROW_HEIGHT * 2, 120, 16);
        spriteSolid(renderState, LAYER_SOLID_COLOR, MENU_X + 96, MENU_Y + ROW_HEIGHT * 3, 120, 16);

        spriteSetColor(renderState, LAYER_SOLID_COLOR, gSoundBarColor);

        spriteSolid(renderState, LAYER_SOLID_COLOR, MENU_X + 100, MENU_Y + ROW_HEIGHT * 2 + 4, (int)(112 * soundPlayerGetMusicVolume()), 8);
        spriteSolid(renderState, LAYER_SOLID_COLOR, MENU_X + 100, MENU_Y + ROW_HEIGHT * 3 + 4, (int)(112 * soundPlayerGetSoundVolume()), 8);
    }

    for (unsigned i = 0; i < itemCount; ++i) {
        struct Coloru8 color;

        if (i == optionsMenu->itemSelection) {
            menuSelectionColor(&color);
        } else {
            color = gDeselectedColor;
        }

        spriteSetColor(renderState, LAYER_KICKFLIP_FONT, color);
        fontRenderText(renderState, &gKickflipFont, textSource[i], MENU_X, y, 0);

        y += ROW_HEIGHT;
    }
}