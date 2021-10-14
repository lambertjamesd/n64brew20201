#ifndef _CAMERA_H
#define _CAMERA_H

#include <ultra64.h>

#include "math/quaternion.h"
#include "math/vector3.h"
#include "math/transform.h"

struct Camera {
    struct Transform transform;
};

void cameraBuildViewMatrix(struct Camera* camera, Mtx* matrix);

#endif