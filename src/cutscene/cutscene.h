#ifndef _CUTSCENE_H
#define _CUTSCENE_H

#include "../data/cutscenes/geometry.h"
#include "sk64/skelatool_animator.h"
#include "graphics/render_state.h"
#include "scene/camera.h"

struct Cutscene {
    struct SKAnimator animator;
    struct Transform* rootTransforms;
    struct Camera camera;
    unsigned short currentSetMask;
    unsigned short cutsceneIndex;
    float currentFade;
    float targetFade;
    float skipTimer;
};

extern struct Cutscene gCutscene;

enum CutsceneIndex {
    CutsceneIndexIntro,
    CutsceneIndexEnding,
    CutsceneIndexEndingCat,
};

void cutsceneInit(struct Cutscene* cutscene, enum CutsceneIndex index);
void cutsceneUpdate(struct Cutscene* cutscene);
void cutsceneRender(struct Cutscene* cutscene, struct RenderState* renderState);

#endif