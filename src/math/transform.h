#ifndef _TRANSFORM_H
#define _TRANSFORM_H

#include "./vector3.h"
#include "./quaternion.h"

struct Transform {
    struct Vector3 position;
    struct Quaternion rotation;
    struct Vector3 scale;
};

#endif