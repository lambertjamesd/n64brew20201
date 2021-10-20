#ifndef _COLLISION_LAYERS_H
#define _COLLISION_LAYERS_H

enum CollisionLayers {
    CollisionLayersTangible = (1 << 0),
    CollisionLayersBase = (1 << 1),
    CollisionLayersStatic = (1 << 2),
};

#endif