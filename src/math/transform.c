
#include "transform.h"

void transformToMatrix(struct Transform* in, float mtx[4][4]) {
    quatToMatrix(&in->rotation, mtx);

    mtx[0][0] *= in->scale.x; mtx[0][1] *= in->scale.x; mtx[0][2] *= in->scale.x;
    mtx[1][0] *= in->scale.y; mtx[1][1] *= in->scale.y; mtx[1][2] *= in->scale.y;
    mtx[2][0] *= in->scale.z; mtx[2][1] *= in->scale.z; mtx[2][2] *= in->scale.z;

    mtx[3][0] = in->position.x;
    mtx[3][1] = in->position.y;
    mtx[3][2] = in->position.z;
}


void transformToMatrixL(struct Transform* in, Mtx* mtx) {
    float mtxf[4][4];
    transformToMatrix(in, mtxf);
    guMtxF2L(mtxf, mtx);
}