
#include "camera.h"
#include "math/transform.h"

void cameraBuildViewMatrix(struct Camera* camera, Mtx* matrix) {
    struct Transform inverse;
    transformInvert(&camera->transform, &inverse);
    transformToMatrixL(&inverse, matrix);
}