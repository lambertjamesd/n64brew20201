
#include "camera.h"
#include "math/transform.h"
#include "collision/staticscene.h"

void cameraInit(struct Camera* camera, float fov, float near, float far) {
    camera->mode = CameraModeFollow;
    quatIdent(&camera->lockedOrientation);
    transformInitIdentity(&camera->transform);
    camera->fov = fov;
    camera->nearPlane = near;
    camera->farPlane = far;
}

void cameraBuildViewMatrix(struct Camera* camera, Mtx* matrix, float rotateView) {
    struct Transform cameraTransCopy = camera->transform;

    if (rotateView) {
        struct Quaternion rotateBy;
        quatAxisAngle(&gForward, rotateView, &rotateBy);
        struct Quaternion tmp;
        quatMultiply(&cameraTransCopy.rotation, &rotateBy, &tmp);
        cameraTransCopy.rotation = tmp;
    }

    struct Transform inverse;
    transformInvert(&cameraTransCopy, &inverse);

    transformToMatrixL(&inverse, matrix);
}

void cameraBuildProjectionMatrix(struct Camera* camera, Mtx* matrix, u16* perspectiveNormalize, float aspectRatio) {
    guPerspective(matrix, perspectiveNormalize, camera->fov, aspectRatio, camera->nearPlane, camera->farPlane, 1.0f);
}

void cameraSetupMatrices(struct Camera* camera, struct RenderState* renderState, float aspectRatio, float rotateView) {
    Mtx* viewProjMatrix = renderStateRequestMatrices(renderState, 2);
    
    if (!viewProjMatrix) {
        return;
    }

    cameraBuildViewMatrix(camera, &viewProjMatrix[0], rotateView);
    gSPMatrix(renderState->dl++, osVirtualToPhysical(&viewProjMatrix[0]), G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);

    u16 perspectiveNormalize;
    cameraBuildProjectionMatrix(camera, &viewProjMatrix[1], &perspectiveNormalize, aspectRatio);
    gSPMatrix(renderState->dl++, osVirtualToPhysical(&viewProjMatrix[1]), G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
    gSPPerspNormalize(renderState->dl++, perspectiveNormalize);
}

void cameraUpdate(struct Camera* camera, struct Vector3* target, float followDistance, float cameraHeight) {
    if (camera->mode == CameraModeFollow) {
        struct Vector3 direction;
        struct Quaternion targetLook;
        vector3Sub(target, &camera->transform.position, &direction);
        quatLook(&direction, &gUp, &targetLook);
        quatLerp(&camera->transform.rotation, &targetLook, 0.05f, &camera->transform.rotation);
        quatNormalize(&camera->transform.rotation, &camera->transform.rotation);

        struct Vector3 offset;
        vector3Sub(&camera->transform.position, target, &offset);
        vector3Normalize(&offset, &offset);
        vector3AddScaled(target, &offset, followDistance, &offset);
        offset.y = target->y + cameraHeight;
        vector3Lerp(&camera->transform.position, &offset, 0.05f, &camera->transform.position);
    } else {
        quatLerp(&camera->transform.rotation, &camera->lockedOrientation, 0.2f, &camera->transform.rotation);
        struct Vector3 targetPos;
        struct Vector3 forward;
        quatMultVector(&camera->transform.rotation, &gForward, &forward);
        vector3AddScaled(target, &forward, followDistance, &targetPos);
        vector3Lerp(&camera->transform.position, &targetPos, 0.5f, &camera->transform.position);
    }
}

void cameraSetLockedMode(struct Camera* camera, struct Quaternion* rotation) {
    camera->mode = CameraModeLocked;
    camera->lockedOrientation = *rotation;
}

void cameraSetFollowMode(struct Camera* camera) {
    camera->mode = CameraModeFollow;
}