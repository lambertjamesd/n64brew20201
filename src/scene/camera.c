
#include "camera.h"
#include "math/transform.h"
#include "collision/staticscene.h"
#include "scene/level_scene.h"

void cameraInit(struct Camera* camera, float fov, float near, float far) {
    camera->mode = 0;
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
    if (camera->mode & CameraModeMapView) {
        guPerspective(matrix, perspectiveNormalize, camera->fov, aspectRatio, camera->nearPlane * 2.0, camera->farPlane * 2.0, 1.0f);
    } else {
        guPerspective(matrix, perspectiveNormalize, camera->fov, aspectRatio, camera->nearPlane, camera->farPlane, 1.0f);
    }
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

void cameraUpdate(struct Camera* camera, struct Vector3* target, float followDistance, float cameraHeight, float aspectRatio) {
    if (camera->mode & CameraModeMapView) {
        struct Vector2 mapCenter;
        struct Vector2 mapSize;

        vector2Add(&gCurrentLevel.definition->levelBoundaries.min, &gCurrentLevel.definition->levelBoundaries.max, &mapCenter);
        vector2Sub(&gCurrentLevel.definition->levelBoundaries.max, &gCurrentLevel.definition->levelBoundaries.min, &mapSize);

        vector2Scale(&mapCenter, 0.5f, &mapCenter);

        struct Vector3 targetPos;
        targetPos.x = mapCenter.x;
        targetPos.z = mapCenter.y;

        float fov = camera->fov * (M_PI / 180.0f / 2.0f);
        float cot = sinf(fov) / cosf(fov);

        float distByY = 2.3f * mapSize.y * cot;
        float distByX = 2.3f * mapSize.x * cot / aspectRatio;

        targetPos.y = MAX(distByY, distByX);

        struct Quaternion targetRotation;
        quatAxisAngle(&gRight, -M_PI * 0.5f, &targetRotation);
        quatLerp(&camera->transform.rotation, &targetRotation, 0.2f, &camera->transform.rotation);

        vector3Lerp(&camera->transform.position, &targetPos, 0.5f, &camera->transform.position);
    } else if (camera->mode & CameraModeLocked) {
        quatLerp(&camera->transform.rotation, &camera->lockedOrientation, 0.2f, &camera->transform.rotation);
        struct Vector3 targetPos;
        struct Vector3 forward;
        quatMultVector(&camera->transform.rotation, &gForward, &forward);
        vector3AddScaled(target, &forward, followDistance, &targetPos);
        vector3Lerp(&camera->transform.position, &targetPos, 0.5f, &camera->transform.position);
    } else {
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
    }
}

void cameraSetLockedMode(struct Camera* camera, struct Quaternion* rotation) {
    camera->mode |= CameraModeLocked;
    camera->lockedOrientation = *rotation;
}

void cameraSetFollowMode(struct Camera* camera) {
    camera->mode &= ~CameraModeLocked;
}

void cameraSetIsMapView(struct Camera* camera, int value) {
    if (value) {
        camera->mode |= CameraModeMapView;
    } else {
        camera->mode &= ~CameraModeMapView;
    }
}