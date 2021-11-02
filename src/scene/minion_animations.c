
#include "minion_animations.h"

#include "../data/models/minionanimations/geometry_anim.inc.h"
#include "../data/models/minionanimations/geometry_animdef.inc.h"

struct SKAnimationEvent minion_attack_animation_events[] = {
    {3, MINION_ANIMATION_EVENT_ATTACK},
};

void minionAnimationsInit() {
    minion_animations_animations[MINION_ANIMATIONS_MINION_ANIMATIONS_ARMATURE_ATTACK_INDEX].numEvents = sizeof(minion_attack_animation_events) / sizeof(*minion_attack_animation_events);
    minion_animations_animations[MINION_ANIMATIONS_MINION_ANIMATIONS_ARMATURE_ATTACK_INDEX].animationEvents = minion_attack_animation_events;
}