#ifndef _CAMERA_H
#define _CAMERA_H

#include <ultra64.h>

#include "math/quaternion.h"
#include "math/vector3.h"
#include "math/transform.h"
#include "graphics/render_state.h"

enum CameraMode {
    CameraModeFollow,
    CameraModeLocked,
};

struct Camera {
    enum CameraMode mode;
    struct Quaternion lockedOrientation;
    struct Transform transform;
    float nearPlane;
    float farPlane;
    float fov;
};

void cameraInit(struct Camera* camera, float fov, float near, float far);
void cameraBuildViewMatrix(struct Camera* camera, Mtx* matrix);
void cameraBuildProjectionMatrix(struct Camera* camera, Mtx* matrix, u16* perspectiveNorm, float aspectRatio);
void cameraSetupMatrices(struct Camera* camera, struct RenderState* renderState, float aspectRatio);

void cameraUpdate(struct Camera* camera, struct Vector3* target, float followDistance, float cameraHeight);

void cameraSetLockedMode(struct Camera* camera, struct Quaternion* rotation);
void cameraSetFollowMode(struct Camera* camera);

#endif