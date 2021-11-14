#include "mainmenu.h"
#include "graphics/spritefont.h"
#include "graphics/image.h"
#include "graphics/gfx.h"
#include "util/time.h"
#include "math/mathf.h"
#include "game_defs.h"
#include "scene/team_data.h"
#include "controls/controller.h"
#include "scene/scene_management.h"
#include "kickflipfont.h"
#include "util/memory.h"
#include "scene/faction.h"
#include "util/rom.h"
#include "textbox.h"

#include "../data/mainmenu/menu.h"
#include "../data/fonts/fonts.h"
#include "../data/models/characters.h"

#define MARS_ROTATE_RATE    (2.0f * M_PI / 30.0f)
#define MARS_TILT           (-10.0f * M_PI / 180.0f)

#define CYCLE_TIME              2.0f

#define SELECT_ANGLE            (M_PI * 1.2f)
#define SELECT_SPIN_TIME        0.5f
#define UNSELECTED_SPIN_FREQ    (0.2f * (M_PI * 2.0f))

struct Coloru8 gDeselectedColor = {128, 128, 128, 255};

struct ButtonLayoutInfo {
    unsigned short x;
    unsigned short y;
    unsigned short w;
    unsigned short h;
    unsigned short* imgData;
};

struct ButtonLayoutInfo gPlayerCountSelectButtons[] = {
    {68, 34, 64, 77, players_1_img},
    {177, 34, 64, 77, players_2_img},
    {68, 131, 64, 77, players_3_img},
    {177, 131, 64, 77, players_4_img},
};

enum MainMenuState gMainMenuTargetState;

void mainMenuFactionInit(struct MainMenuFactionSelector* faction, unsigned index) {
    faction->selectedFaction = index % FACTION_COUNT;

    skAnimatorInit(&faction->animator, ANY_FACTION_BONE_COUNT, 0, 0);
    skArmatureInit(
        &faction->armature, 
        gFactions[faction->selectedFaction]->playerMesh, 
        ANY_FACTION_BONE_COUNT, 
        CALC_ROM_POINTER(character_animations, gFactions[faction->selectedFaction]->playerDefaultPose), 
        gFactions[0]->playerBoneParent
    );
    faction->flags = 0;
    transformInitIdentity(&faction->transform);
    vector3Scale(&gOneVec, &faction->transform.scale, 0.5f);
    faction->transform.position.y = -50.0f;

    skAnimatorRunClip(
        &faction->animator, 
        gFactions[faction->selectedFaction]->playerAnimations[PlayerAnimationSelectIdle], 
        SKAnimatorFlagsLoop
    );

    faction->rotateLerp = 0.0f;
}

void mainMenuFactionUpdate(struct MainMenuFactionSelector* faction, unsigned index) {
    skAnimatorUpdate(&faction->animator, faction->armature.boneTransforms, 1.0f);

    if ((faction->flags & MainMenuFactionFlagsSelected) == 0) {
        enum ControllerDirection controllerDirection = controllerGetDirectionDown(index);

        unsigned prevFaction = faction->selectedFaction;

        if (controllerDirection & ControllerDirectionLeft) {
            if (faction->selectedFaction == 0) {
                faction->selectedFaction = FACTION_COUNT - 1;
            } else {
                --faction->selectedFaction;
            }
        }

        if (controllerDirection & ControllerDirectionRight) {
            ++faction->selectedFaction;

            if (faction->selectedFaction >= FACTION_COUNT) {
                faction->selectedFaction = 0;
            }
        }

        faction->rotateLerp = mathfMoveTowards(faction->rotateLerp, 0.0f, gTimeDelta / SELECT_SPIN_TIME);
        
        if (controllerGetButtonDown(index, A_BUTTON)) {
            soundPlayerPlay(SOUNDS_UI_SELECT, 0);
            faction->flags |= MainMenuFactionFlagsSelected;
            skAnimatorRunClip(
                &faction->animator, 
                gFactions[faction->selectedFaction]->playerAnimations[PlayerAnimationSelected], 
                0
            );
        } else if (prevFaction != faction->selectedFaction) {
            faction->armature.displayList = gFactions[faction->selectedFaction]->playerMesh;
            skAnimatorRunClip(
                &faction->animator, 
                gFactions[faction->selectedFaction]->playerAnimations[PlayerAnimationSelectIdle], 
                SKAnimatorFlagsLoop
            );
        }
    } else {
        if (controllerGetButtonDown(index, B_BUTTON)) {
            soundPlayerPlay(SOUNDS_UI_SELECT, 0);
            faction->flags &= ~MainMenuFactionFlagsSelected;

            skAnimatorRunClip(
                &faction->animator, 
                gFactions[faction->selectedFaction]->playerAnimations[PlayerAnimationSelectIdle], 
                SKAnimatorFlagsLoop
            );
        }

        faction->rotateLerp = mathfMoveTowards(faction->rotateLerp, 1.0f, gTimeDelta / SELECT_SPIN_TIME);
    }

    float rotateAngle = mathfMod(gTimePassed * UNSELECTED_SPIN_FREQ + M_PI, M_PI * 2.0f);

    if (faction->rotateLerp <= 0.0f) {
        quatAxisAngle(&gUp, rotateAngle, &faction->transform.rotation);
    } else if (faction->rotateLerp >= 1.0f) {
        quatAxisAngle(&gUp, SELECT_ANGLE, &faction->transform.rotation);
    } else {
        quatAxisAngle(&gUp, mathfLerp(rotateAngle, SELECT_ANGLE, faction->rotateLerp), &faction->transform.rotation);
    }
}
 
void mainMenuStartLevel(struct MainMenu* mainMenu) {
    struct GameConfiguration gameConfig;
    gameConfig.humanPlayerCount = mainMenu->selectedPlayerCount + 1;

    if (gameConfig.humanPlayerCount == 1) {
        gameConfig.playerCount = 2;
    } else {
        gameConfig.playerCount = gameConfig.humanPlayerCount;
    }

    gameConfig.level = mainMenu->filteredLevels[mainMenu->selectedLevel];

    for (unsigned i = 0; i <= mainMenu->selectedPlayerCount; ++i) {
        gTeamFactions[i] = gFactions[mainMenu->factionSelection[i].selectedFaction];
    }

    sceneQueueLoadLevel(&gameConfig);
}

void mainMenuEnterFactionSelection(struct MainMenu* mainMenu) {
    mainMenu->menuState = MainMenuStateSelectingFaction;
    mainMenu->targetMenuState = MainMenuStateSelectingFaction;

    gfxInitSplitscreenViewport(mainMenu->selectedPlayerCount + 1);
}

void mainMenuEnterLevelSelection(struct MainMenu* mainMenu) {
    mainMenu->menuState = MainMenuStateSelectingLevel;
    mainMenu->targetMenuState = MainMenuStateSelectingLevel;
    
    mainMenu->selectedLevel = 0;
    mainMenu->levelCount = 0;

    for (unsigned i = 0; i < gLevelCount; ++i) {
        if ((gLevels[i].flags & LevelMetadataFlagsMultiplayer) != 0 && gLevels[i].maxPlayers >= mainMenu->selectedPlayerCount + 1) {
            mainMenu->filteredLevels[mainMenu->levelCount] = &gLevels[i];
            ++mainMenu->levelCount;
        }
    }

    textBoxInit(&gTextBox, mainMenu->filteredLevels[0]->name, 200, SCREEN_WD / 2, 46);
}

void mainMenuInit(struct MainMenu* mainMenu) {
    cameraInit(&mainMenu->camera, 22.0f, 100.0f, 18000.0f);
    mainMenu->camera.transform.position.z = 600.0f;
    transformInitIdentity(&mainMenu->marsTransform);
    mainMenu->marsTransform.position.x = 50.0f;
    mainMenu->menuState = gMainMenuTargetState;
    mainMenu->targetMenuState = gMainMenuTargetState;
    mainMenu->selectedPlayerCount = 0;
    mainMenu->selectedLevel = 0;
    mainMenu->filteredLevels = malloc(sizeof(struct ThemeMetadata*) * gLevelCount);
    initKickflipFont();

    for (unsigned i = 0; i < MAX_PLAYERS; ++i) {
        mainMenuFactionInit(&mainMenu->factionSelection[i], i);
    }
}

void mainMenuUpdatePlayerCount(struct MainMenu* mainMenu) {
    enum ControllerDirection direction = controllerGetDirectionDown(0);
    
    if ((direction & ControllerDirectionLeft) != 0 && mainMenu->selectedPlayerCount > 0) {
        --mainMenu->selectedPlayerCount;
    }
    if ((direction & ControllerDirectionRight) != 0 && mainMenu->selectedPlayerCount + 1 < MAX_PLAYERS) {
        ++mainMenu->selectedPlayerCount;
    }
    if ((direction & ControllerDirectionDown) != 0 && mainMenu->selectedPlayerCount + 2 < MAX_PLAYERS) {
        mainMenu->selectedPlayerCount += 2;
    }
    if ((direction & ControllerDirectionUp) != 0 && mainMenu->selectedPlayerCount > 1) {
        mainMenu->selectedPlayerCount -= 2;
    }

    if (controllerGetButtonDown(0, A_BUTTON)) {
        soundPlayerPlay(SOUNDS_UI_SELECT, 0);
        mainMenuEnterFactionSelection(mainMenu);
    }
}

void mainMenuUpdateFaction(struct MainMenu* mainMenu) {
    unsigned isReady = 1;

    if ((mainMenu->factionSelection[0].flags & MainMenuFactionFlagsSelected) == 0 && controllerGetButtonDown(0, B_BUTTON)) {
        soundPlayerPlay(SOUNDS_UI_SELECT, 0);
        mainMenu->menuState = MainMenuStateSelectingPlayerCount;
    }

    for (unsigned i = 0; i <= mainMenu->selectedPlayerCount; ++i) {
        if (!(mainMenu->factionSelection[i].flags & MainMenuFactionFlagsSelected)) {
            isReady = 0;
        }

        mainMenuFactionUpdate(&mainMenu->factionSelection[i], i);
    }

    if (isReady && controllerGetButtonDown(0, A_BUTTON)) {
        soundPlayerPlay(SOUNDS_UI_SELECT, 0);
        mainMenuEnterLevelSelection(mainMenu);
    }
}

void mainMenuUpdateLevelSelect(struct MainMenu* mainMenu) {
    textBoxUpdate(&gTextBox);

    if (mainMenu->targetMenuState == MainMenuStateStarting) {
        if (!textBoxIsVisible(&gTextBox)) {
            mainMenuStartLevel(mainMenu);
        }
        return;
    } else if (mainMenu->targetMenuState == MainMenuStateSelectingFaction) {
        if (!textBoxIsVisible(&gTextBox)) {
            mainMenu->menuState = MainMenuStateSelectingFaction;
            mainMenu->targetMenuState = MainMenuStateSelectingFaction;
        }
        return;
    }

    enum ControllerDirection direction = controllerGetDirectionDown(0);

    if ((direction & ControllerDirectionLeft) != 0 && mainMenu->selectedLevel > 0) {
        --mainMenu->selectedLevel;
    }

    if ((direction & ControllerDirectionRight) != 0 && mainMenu->selectedLevel + 1 < mainMenu->levelCount) {
        ++mainMenu->selectedLevel;
    }

    textBoxChangeText(&gTextBox, mainMenu->filteredLevels[mainMenu->selectedLevel]->name);

    if (controllerGetButtonDown(0, A_BUTTON)) {
        soundPlayerPlay(SOUNDS_UI_SELECT, 0);
        mainMenu->targetMenuState = MainMenuStateStarting;
        textBoxHide(&gTextBox);
    }

    if (controllerGetButtonDown(0, B_BUTTON)) {
        soundPlayerPlay(SOUNDS_UI_SELECT, 0);
        mainMenu->targetMenuState = MainMenuStateSelectingFaction;
        textBoxHide(&gTextBox);
    }
}

void mainMenuUpdate(struct MainMenu* mainMenu) {
    struct Quaternion axisSpin;
    struct Quaternion axisTilt;
    quatAxisAngle(&gUp, gTimePassed * MARS_ROTATE_RATE, &axisSpin);
    quatAxisAngle(&gForward, MARS_TILT, &axisTilt);
    quatMultiply(&axisTilt, &axisSpin, &mainMenu->marsTransform.rotation);

    switch (mainMenu->menuState) {
        case MainMenuStateSelectingPlayerCount:
            mainMenuUpdatePlayerCount(mainMenu);
            break;
        case MainMenuStateSelectingFaction:
            mainMenuUpdateFaction(mainMenu);
            break;
        case MainMenuStateSelectingLevel:
            mainMenuUpdateLevelSelect(mainMenu);
            break;
        case MainMenuStateStarting:
            break;
        case MainMenuStatePostGame:
            if (endGameMenuUpdate(&mainMenu->endGameMenu)) {
                mainMenu->menuState = MainMenuStateSelectingPlayerCount;
            }
            break;
    }
}

void mainMenuSelectionColor(struct Coloru8* result) {
    float timeLerp = mathfMod(gTimePassed, CYCLE_TIME) * (MAX_PLAYERS / CYCLE_TIME);
    int colorIndex = ((int)timeLerp) % MAX_PLAYERS;
    int nextColor = (colorIndex + 1) % MAX_PLAYERS;
    colorU8Lerp(&gTeamColors[colorIndex], &gTeamColors[nextColor], timeLerp - colorIndex, result);
    colorU8Lerp(result, &gColorWhite, 0.5f, result);
}

void mainMenuRenderPlayerCount(struct MainMenu* mainMenu, struct RenderState* renderState) {
    for (unsigned i = 0; i < MAX_PLAYERS; ++i) {
        struct Coloru8 color;
        if (mainMenu->selectedPlayerCount == i) {
            mainMenuSelectionColor(&color);
        } else {
            color = gDeselectedColor;
        }

        struct ButtonLayoutInfo* button = &gPlayerCountSelectButtons[i];

        graphicsCopyImage(
            renderState,
            button->imgData,
            button->w,
            button->h,
            0, 0,
            button->x,
            button->y,
            button->w,
            button->h,
            color
        );
    }
}

void mainMenuRenderFactions(struct MainMenu* mainMenu, struct RenderState* renderState) {
    unsigned isReady = 1;

    for (unsigned int i = 0; i < mainMenu->selectedPlayerCount+1; ++i) {
        gDPPipeSync(renderState->dl++);
        Vp* viewport = &gSplitScreenViewports[i];
        cameraSetupMatrices(
            &mainMenu->camera, 
            renderState, 
            (float)viewport->vp.vscale[0] / (float)viewport->vp.vscale[1],
            0.0f
        );
        gSPViewport(renderState->dl++, osVirtualToPhysical(viewport));
        gDPSetScissor(
            renderState->dl++, 
            G_SC_NON_INTERLACE, 
            gClippingRegions[i * 4 + 0],
            gClippingRegions[i * 4 + 1],
            gClippingRegions[i * 4 + 2],
            gClippingRegions[i * 4 + 3]
        );
        gDPPipeSync(renderState->dl++);
        
        Mtx* playerMatrix = renderStateRequestMatrices(renderState, 1);
        transformToMatrixL(&mainMenu->factionSelection[i].transform, playerMatrix);
        gSPMatrix(renderState->dl++, playerMatrix, G_MTX_MODELVIEW | G_MTX_PUSH | G_MTX_MUL);
        gSPDisplayList(renderState->dl++, gTeamPalleteTexture[i]);
        skRenderObject(&mainMenu->factionSelection[i].armature, renderState);
        gSPPopMatrix(renderState->dl++, G_MTX_MODELVIEW);

        if (!(mainMenu->factionSelection[i].flags & MainMenuFactionFlagsSelected)) {
            isReady = 0;
        }
    }

    gSPViewport(renderState->dl++, osVirtualToPhysical(&gFullScreenVP));
    gDPSetScissor(
        renderState->dl++, 
        G_SC_NON_INTERLACE, 
        0,
        0,
        SCREEN_WD,
        SCREEN_HT
    );

    if (isReady) {
        spriteSetColor(renderState, LAYER_SOLID_COLOR, gHalfTransparentBlack);
        spriteSolid(renderState, LAYER_SOLID_COLOR, 0, 88, SCREEN_WD, 64);

        char* message = "Ready?";
        unsigned messageX = (SCREEN_WD - fontMeasure(&gKickflipFont, message, 1)) >> 1;
        fontRenderText(renderState, &gKickflipFont, message, messageX, 104, 1);
    }
}

void mainMenuRenderLevels(struct MainMenu* mainMenu, struct RenderState* renderState) {
    textBoxRender(&gTextBox, renderState);
}

void mainMenuRender(struct MainMenu* mainMenu, struct RenderState* renderState) {
    spriteSetLayer(renderState, LAYER_SOLID_COLOR, gMainMenuSolidColor);
    spriteSetLayer(renderState, LAYER_KICKFLIP_FONT, gUseKickflipFont);

    graphicsCopyImage(renderState, gMenuBackground, SCREEN_WD, SCREEN_HT, 0, 0, 0, 0, SCREEN_WD, SCREEN_HT, gColorWhite);
    gDPSetTexturePersp(renderState->dl++, G_TP_PERSP);

    cameraSetupMatrices(&mainMenu->camera, renderState, (float)SCREEN_WD/(float)SCREEN_HT, 0.0f);
    gSPViewport(renderState->dl++, osVirtualToPhysical(&gFullScreenVP));
    gDPSetScissor(
        renderState->dl++, 
        G_SC_NON_INTERLACE, 
        0,
        0,
        SCREEN_WD,
        SCREEN_HT
    );
    gDPPipeSync(renderState->dl++);
    gDPSetCycleType(renderState->dl++, G_CYC_1CYCLE); 
    Mtx* marsMatrix = renderStateRequestMatrices(renderState, 1);
    transformToMatrixL(&mainMenu->marsTransform, marsMatrix);
    gSPMatrix(renderState->dl++, marsMatrix, G_MTX_MODELVIEW | G_MTX_PUSH | G_MTX_MUL);
    gSPDisplayList(renderState->dl++, Mars_Mars_mesh);
    gSPPopMatrix(renderState->dl++, G_MTX_MODELVIEW);
    gDPPipeSync(renderState->dl++);
    gSPSetGeometryMode(renderState->dl++, G_ZBUFFER);
    gDPSetRenderMode(renderState->dl++, G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2);

    switch (mainMenu->menuState) {
        case MainMenuStateSelectingPlayerCount:
            mainMenuRenderPlayerCount(mainMenu, renderState);
            break;
        case MainMenuStateSelectingFaction:
            mainMenuRenderFactions(mainMenu, renderState);
            break;
        case MainMenuStateSelectingLevel:
            mainMenuRenderLevels(mainMenu, renderState);
            break;
        case MainMenuStateStarting:
            break;
        case MainMenuStatePostGame:
            endGameMenuRender(&mainMenu->endGameMenu, renderState);
            break;
    }

    spriteFinish(renderState);
}