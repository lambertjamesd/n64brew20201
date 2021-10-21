#ifndef _COLLISION_LAYERS_H
#define _COLLISION_LAYERS_H

enum CollisionLayers {
    CollisionLayersTangible = (1 << 0),
    CollisionLayersBase = (1 << 1),
    CollisionLayersStatic = (1 << 2),
    CollisionLayersTeam0 = (1 << 4),
    CollisionLayersAllTeams = (0x1F << 4),
};

#define COLLISION_LAYER_FOR_TEAM(team)   (CollisionLayersTeam0 << (team))

#endif