#ifndef _TRANSFORM_H
#define _TRANSFORM_H

#include <ultra64.h>
#include "./vector3.h"
#include "./quaternion.h"

struct Transform {
    struct Vector3 position;
    struct Quaternion rotation;
    struct Vector3 scale;
};

void transformToMatrix(struct Transform* in, float mtx[4][4]);
void transformToMatrixL(struct Transform* in, Mtx* mtx);

#endif