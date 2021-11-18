#include "target_finder.h"
#include "level_scene.h"
#include "collision/circle.h"
#include "collision/collisionlayers.h"
#include "scene_management.h"

struct CollisionCircle gFinderCircle = {
    {CollisionShapeTypeCircle},
    MINION_DEFENSE_RADIUS,
};

struct TeamEntity* findCloserEntity(struct TeamEntity* current, struct TeamEntity* check, struct Vector3* from, float* currentBest) {
    struct Vector3* checkPosition = teamEntityGetPosition(check);
    
    if (checkPosition == from) {
        return current;
    }

    float checkDistSqrd = vector3DistSqrd(checkPosition, from);

    if (!current || checkDistSqrd < *currentBest) {
        *currentBest = checkDistSqrd;
        return check;
    }

    return current;
}

void targetFinderTrigger(struct DynamicSceneOverlap* overlap) {
    if (overlap->otherEntry->flags & DynamicSceneEntryHasTeam) {
        struct TargetFinder* finder = (struct TargetFinder*)overlap->thisEntry->data;
        struct TeamEntity* teamEntity = (struct TeamEntity*)overlap->otherEntry->data;

        struct Minion* minion = &gCurrentLevel.minions[finder->minionIndex];

        if (minionIsAlive(minion)) {
            float currentTargetDistnace = 0;

            if (minion->currentTarget) {
                currentTargetDistnace = vector3DistSqrd(teamEntityGetPosition(minion->currentTarget), &minion->transform.position);
            }

            minion->currentTarget = findCloserEntity(minion->currentTarget, teamEntity, &minion->transform.position, &currentTargetDistnace);
        }
    }
}

void targetFinderInit(struct TargetFinder* finder, unsigned startIndex) {
    finder->minionIndex = startIndex;
    finder->trigger = dynamicSceneNewEntry(
        &gFinderCircle.shapeCommon,
        finder,
        &gRight2,
        targetFinderTrigger,
        DynamicSceneEntryIsTrigger,
        0
    );
}

struct TeamEntity* targetFinderFindNearestTarget(struct LevelScene* forScene, struct Vector3* from, unsigned againstTeam) {
    struct TeamEntity* result = 0;
    float bestDistance = 0;

    for (unsigned i = 0; i < forScene->playerCount; ++i) {
        if (playerIsAlive(&forScene->players[i]) && forScene->players[i].team.teamNumber != againstTeam) {
            result = findCloserEntity(result, &forScene->players[i].team, from, &bestDistance);
        }
    }
    
    for (unsigned i = 0; i < forScene->minionCount; ++i) {
        if (minionIsAlive(&forScene->minions[i]) && forScene->minions[i].team.teamNumber != againstTeam) {
            result = findCloserEntity(result, &forScene->minions[i].team, from, &bestDistance);
        }
    }

    for (unsigned i = 0; i < forScene->baseCount; ++i) {
        if (forScene->bases[i].team.teamNumber != againstTeam || forScene->bases[i].state == LevelBaseStateNeutral) {
            result = findCloserEntity(result, &forScene->bases[i].team, from, &bestDistance);
        }
    }

    return result;
}

void targetFinderUpdate(struct TargetFinder* finder) {
    finder->minionIndex = (finder->minionIndex + 1) % gCurrentLevel.minionCount;

    struct Minion* minion = &gCurrentLevel.minions[finder->minionIndex];
    finder->trigger->collisionLayers = 0;
    // put the trigger off screen to minimize cycles checking collisions
    finder->trigger->center.x = (finder->minionIndex + 1000.0f) * 100.0f;
    finder->trigger->collisionLayers |= DynamicSceneEntryDirtyBox;

    if (minionIsAlive(minion)) {
        if (minion->currentCommand == MinionCommandDefend) {
            finder->trigger->collisionLayers = CollisionLayersAllTeams ^ COLLISION_LAYER_FOR_TEAM(minion->team.teamNumber);
            dynamicEntrySetPos3D(finder->trigger, &minion->defensePoint);
        } 
        else if (minion->currentCommand == MinionCommandAttack) {
            
            minion->currentTarget = targetFinderFindNearestTarget(&gCurrentLevel, &minion->transform.position, minion->team.teamNumber);

            if(minion->currentTarget->entityType == TeamEntityTypeBase){
                if(minion->pathfinder->currentNode == NODE_NONE || minion->currentTarget->teamNumber == minion->team.teamNumber)
                    pathfinderSetTarget(minion->pathfinder, &gCurrentLevel.definition->pathfinding, &minion->transform.position, teamEntityGetPosition(minion->currentTarget));
            }
        }
    }
}