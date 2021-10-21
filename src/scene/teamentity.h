#ifndef _TEAM_ENTITY_H
#define _TEAM_ENTITY_H

#include "math/vector3.h"
#include "collision/dynamicscene.h"

enum TeamEntityType {
    TeamEntityTypeMinion,
    TeamEntityTypeBase,
    TeamEntityTypePlayer,
};

struct TeamEntity {
    unsigned short entityType;
    unsigned short teamNumber;
};

struct Vector3* teamEntityGetPosition(struct TeamEntity* entity);

void teamEntityCorrectOverlap(struct DynamicSceneOverlap* overlap);
void teamEntityApplyDamage(struct TeamEntity* entity, float amount);

#endif