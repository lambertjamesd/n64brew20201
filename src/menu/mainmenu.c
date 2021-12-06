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
#include "savefile/savefile.h"
#include "menucommon.h"
#include "levelselection.h"

#include "../data/mainmenu/menu.h"
#include "../data/fonts/fonts.h"
#include "../data/models/characters.h"

#define MARS_ROTATE_RATE    (2.0f * M_PI / 30.0f)
#define MARS_TILT           (-10.0f * M_PI / 180.0f)

#define SELECT_ANGLE            (M_PI * 1.2f)
#define SELECT_SPIN_TIME        0.5f
#define UNSELECTED_SPIN_FREQ    (0.2f * (M_PI * 2.0f))

struct ButtonLayoutInfo {
    unsigned short x;
    unsigned short y;
    unsigned short w;
    unsigned short h;
    unsigned short* imgData;
};

struct ButtonLayoutInfo gPlayerCountSelectButtons[] = {
    {68, 22, 64, 77, players_1_img},
    {177, 22, 64, 77, players_2_img},
    {68, 107, 64, 77, players_3_img},
    {177, 107, 64, 77, players_4_img},
};

int mainMenuGetPlayerCount(struct MainMenu* menu) {
    return menu->selections.selectedPlayerCount + 1;
}

void mainMenuFactionInit(struct MainMenuFactionSelector* faction, unsigned index) {
    faction->selectedFaction = 0;

    for (unsigned i = 0; i < FACTION_COUNT; ++i) {
        if (gTeamFactions[index] == gFactions[i]) {
            faction->selectedFaction = i;
            break;
        }
    }

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
        
        if (controllerGetButtonDown(index, A_BUTTON) || (faction->flags & MainMenuFactionFlagsAI) != 0) {
            soundPlayerPlay(SOUNDS_UI_SELECT2, 0, 0);
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
            soundPlayerPlay(SOUNDS_UI_SELECT3, 0, 0);
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
    gameConfig.playerCount = mainMenuGetPlayerCount(mainMenu);

    unsigned aiPlayerMask = 0;

    for (unsigned i = 0; i < mainMenu->filteredLevels[mainMenu->selections.selectedLevel]->maxPlayers; ++i) {
        if ((mainMenu->factionSelection[i].flags & MainMenuFactionFlagsAI) != 0 ||
            i >= gameConfig.playerCount || 
            !controllerIsConnected(i)) {
            aiPlayerMask |= (1 << i);
        }
    }

    if (gameConfig.playerCount == 1) {
        gameConfig.playerCount = mainMenu->filteredLevels[mainMenu->selections.selectedLevel]->maxPlayers;
    }

    gameConfig.aiPlayerMask = aiPlayerMask;

    gameConfig.level = mainMenu->filteredLevels[mainMenu->selections.selectedLevel];

    for (unsigned i = 0; i <= mainMenu->selections.selectedPlayerCount; ++i) {
        gTeamFactions[i] = gFactions[mainMenu->factionSelection[i].selectedFaction];
    }

    sceneQueueLoadLevel(&gameConfig);
}

void mainMenuEnterFactionSelection(struct MainMenu* mainMenu) {
    mainMenu->selections.menuState = MainMenuStateSelectingFaction;
    mainMenu->selections.targetMenuState = MainMenuStateSelectingFaction;

    gfxInitSplitscreenViewport(mainMenuGetPlayerCount(mainMenu));

    for (unsigned i = 0; i < mainMenuGetPlayerCount(mainMenu); ++i) {
        mainMenu->factionSelection[i].flags = controllerIsConnected(i) ? 0 : MainMenuFactionFlagsAI;
    }

    for (unsigned i = mainMenuGetPlayerCount(mainMenu); i < MAX_PLAYERS; ++i) {
        mainMenu->factionSelection[i].flags = MainMenuFactionFlagsAI;
    }
}

void mainMenuEnterPlayerSelection(struct MainMenu* mainMenu) {
    mainMenu->selections.menuState = MainMenuStateSelectingPlayerCount;
    mainMenu->selections.targetMenuState = MainMenuStateSelectingPlayerCount;
}

void mainMenuInitSelections(struct MainMenu* mainMenu) {
    mainMenu->selections.menuState = MainMenuStateSelectingTitleScreen;
    mainMenu->selections.targetMenuState = MainMenuStateSelectingTitleScreen;

    mainMenu->selections.selectedPlayerCount = 0;
    mainMenu->selections.selectedLevel = 0;
}

void mainMenuInit(struct MainMenu* mainMenu) {
    cameraInit(&mainMenu->camera, 22.0f, 100.0f, 18000.0f);
    mainMenu->camera.transform.position.z = 600.0f;
    transformInitIdentity(&mainMenu->marsTransform);
    mainMenu->marsTransform.position.x = 50.0f;
    mainMenu->filteredLevels = malloc(sizeof(struct ThemeMetadata*) * gLevelCount);
    mainMenu->showingWireframe = 0;
    mainMenu->showWireframeDelay = 0;
    initKickflipFont();
    titleScreenInit(&mainMenu->titleScreen);
    optionsMenuInit(&mainMenu->optionsMenu);

    for (unsigned i = 0; i < MAX_PLAYERS; ++i) {
        mainMenuFactionInit(&mainMenu->factionSelection[i], i);
    }

    unsigned wireframeSize = 0;

    for (unsigned i = 0; i < gLevelCount; ++i) {
        wireframeSize = MAX(wireframeSize, gLevels[i].wireframe.romSegmentEnd - gLevels[i].wireframe.romSegmentStart);
    }

    gWireframeSegment = malloc(wireframeSize);

    if (mainMenu->selections.menuState == MainMenuStatePostGame) {
        mainMenu->musicSoundID = soundPlayerPlay(SOUNDS_VICTORYMUSIC, SoundPlayerFlagsIsMusic, 0);
    } else {
        mainMenu->musicSoundID = soundPlayerPlay(SOUNDS_MAP_CHARACTER_SELECT_OR_VICTORY_MUSIC, SoundPlayerFlagsIsMusic, 0);
    }
}

void mainMenuUpdatePlayerCount(struct MainMenu* mainMenu) {
    enum ControllerDirection direction = controllerGetDirectionDown(0);
    
    if ((direction & ControllerDirectionLeft) != 0 && mainMenu->selections.selectedPlayerCount > 0) {
        --mainMenu->selections.selectedPlayerCount;
    }
    if ((direction & ControllerDirectionRight) != 0 && mainMenuGetPlayerCount(mainMenu) <= MAX_PLAYERS) {
        ++mainMenu->selections.selectedPlayerCount;
    }
    if ((direction & ControllerDirectionDown) != 0 && mainMenu->selections.selectedPlayerCount + 2 <= MAX_PLAYERS) {
        mainMenu->selections.selectedPlayerCount += 2;
    }
    if ((direction & ControllerDirectionUp) != 0 && mainMenu->selections.selectedPlayerCount > 1) {
        mainMenu->selections.selectedPlayerCount -= 2;
    }

    if (controllerGetButtonDown(0, A_BUTTON)) {
        soundPlayerPlay(SOUNDS_UI_SELECT2, 0, 0);

        if (mainMenu->selections.selectedPlayerCount == MAX_PLAYERS) {
            mainMenu->selections.menuState = MainMenuStateSelectingOptions;
            mainMenu->selections.targetMenuState = MainMenuStateSelectingOptions;
            optionsMenuInit(&mainMenu->optionsMenu);
        } else {
            mainMenuEnterFactionSelection(mainMenu);
        }
    }
}

void mainMenuUpdateFaction(struct MainMenu* mainMenu) {
    unsigned isReady = 1;

    if ((mainMenu->factionSelection[0].flags & MainMenuFactionFlagsSelected) == 0 && controllerGetButtonDown(0, B_BUTTON)) {
        soundPlayerPlay(SOUNDS_UI_SELECT3, 0, 0);
        mainMenu->selections.menuState = MainMenuStateSelectingPlayerCount;
    }

    for (unsigned i = 0; i <= mainMenu->selections.selectedPlayerCount; ++i) {
        if (!(mainMenu->factionSelection[i].flags & MainMenuFactionFlagsSelected)) {
            isReady = 0;
        }

        mainMenuFactionUpdate(&mainMenu->factionSelection[i], i);
    }

    if (isReady && controllerGetButtonDown(0, A_BUTTON)) {
        soundPlayerPlay(SOUNDS_UI_SELECT2, 0, 0);
        mainMenuEnterLevelSelection(mainMenu);
    }
}

void mainMenuUpdate(struct MainMenu* mainMenu) {
    struct Quaternion axisSpin;
    struct Quaternion axisTilt;
    quatAxisAngle(&gUp, gTimePassed * MARS_ROTATE_RATE, &axisSpin);
    quatAxisAngle(&gForward, MARS_TILT, &axisTilt);
    quatMultiply(&axisTilt, &axisSpin, &mainMenu->marsTransform.rotation);

    switch (mainMenu->selections.menuState) {
        case MainMenuStateSelectingTitleScreen:
            if (titleScreenUpdate(&mainMenu->titleScreen)) {
                mainMenuEnterPlayerSelection(mainMenu);
            }
            break;
        case MainMenuStateSelectingPlayerCount:
            mainMenuUpdatePlayerCount(mainMenu);
            break;
        case MainMenuStateSelectingOptions:
            if (optionsMenuUpdate(&mainMenu->optionsMenu)) {
                mainMenu->selections.menuState = MainMenuStateSelectingPlayerCount;
            }
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
                if (mainMenu->selections.selectedPlayerCount == 0 && mainMenu->selections.selectedLevel == mainMenu->levelCount) {
                    sceneQueueCredits();
                } else {
                    if (mainMenu->musicSoundID != SOUND_ID_NONE) {
                        soundPlayerStop(&mainMenu->musicSoundID);
                    }

                    mainMenu->musicSoundID = soundPlayerPlay(SOUNDS_MAP_CHARACTER_SELECT_OR_VICTORY_MUSIC, SoundPlayerFlagsIsMusic, 0);

                    gfxInitSplitscreenViewport(mainMenuGetPlayerCount(mainMenu));
                    mainMenuEnterLevelSelection(mainMenu);
                }
            }
            break;
    }
}

void mainMenuRenderPlayerCount(struct MainMenu* mainMenu, struct RenderState* renderState) {
    for (unsigned i = 0; i < MAX_PLAYERS; ++i) {
        struct Coloru8 color;
        if (mainMenu->selections.selectedPlayerCount == i) {
            menuSelectionColor(&color);
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

    if (mainMenu->selections.selectedPlayerCount == MAX_PLAYERS) {
        struct Coloru8 color;
        menuSelectionColor(&color);
        spriteSetColor(renderState, LAYER_KICKFLIP_FONT, color);
    } else {
        spriteSetColor(renderState, LAYER_KICKFLIP_FONT, gDeselectedColor);
    }

    fontRenderText(renderState, &gKickflipFont, "Options", 68, 200, -1);
}

void mainMenuRenderFactions(struct MainMenu* mainMenu, struct RenderState* renderState) {
    unsigned isReady = 1;

    for (unsigned int i = 0; i < mainMenu->selections.selectedPlayerCount+1; ++i) {
        gDPPipeSync(renderState->dl++);
        Vp* viewport = &gSplitScreenViewports[i];
        cameraSetupMatrices(
            &mainMenu->camera, 
            renderState, 
            (float)viewport->vp.vscale[0] / (float)viewport->vp.vscale[1],
            0.0f
        );
        gSPViewport(renderState->dl++, osVirtualToPhysical(viewport));

        unsigned short* clippingRegion = &gClippingRegions[i * 4];

        gDPSetScissor(
            renderState->dl++, 
            G_SC_NON_INTERLACE, 
            clippingRegion[0],
            clippingRegion[1],
            clippingRegion[2],
            clippingRegion[3]
        );
        gDPPipeSync(renderState->dl++);
        
        Mtx* playerMatrix = renderStateRequestMatrices(renderState, 1);
        transformToMatrixL(&mainMenu->factionSelection[i].transform, playerMatrix);
        gSPMatrix(renderState->dl++, playerMatrix, G_MTX_MODELVIEW | G_MTX_PUSH | G_MTX_MUL);
        gDPSetTexturePersp(renderState->dl++, G_TP_PERSP);
        gSPDisplayList(renderState->dl++, gTeamTexture);
        gDPUseTeamPallete(renderState->dl++, i, 2);
        gDPSetRenderMode(renderState->dl++, G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2);
        skRenderObject(&mainMenu->factionSelection[i].armature, renderState);
        gSPPopMatrix(renderState->dl++, G_MTX_MODELVIEW);

        if (!(mainMenu->factionSelection[i].flags & MainMenuFactionFlagsSelected)) {
            isReady = 0;
        }

        unsigned midY = (clippingRegion[1] + clippingRegion[3]) / 2;

        unsigned srcOffset = (mainMenu->factionSelection[i].flags & MainMenuFactionFlagsSelected) ? 32 : 0;

        graphicsCopyImage(renderState, ArrowButtons_0_0, 32, 64, 0, 16 + srcOffset, clippingRegion[0] + 24, midY - 8, 16, 16, gColorWhite);
        graphicsCopyImage(renderState, ArrowButtons_0_0, 32, 64, 16, 16 + srcOffset, clippingRegion[2] - 40, midY - 8, 16, 16, gColorWhite);
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

void mainMenuRender(struct MainMenu* mainMenu, struct RenderState* renderState) {
    spriteSetLayer(renderState, LAYER_SOLID_COLOR, gMainMenuSolidColor);
    spriteSetLayer(renderState, LAYER_KICKFLIP_NUMBERS_FONT, gUseKickflipNumbersFont);
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

    switch (mainMenu->selections.menuState) {
        case MainMenuStateSelectingTitleScreen:
            titleScreenRender(&mainMenu->titleScreen, renderState);
            break;
        case MainMenuStateSelectingPlayerCount:
            mainMenuRenderPlayerCount(mainMenu, renderState);
            break;
        case MainMenuStateSelectingOptions:
            optionsMenuRender(&mainMenu->optionsMenu, renderState);
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