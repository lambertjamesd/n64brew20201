
#include "skelatool_object.h"
#include "skelatool_defs.h"

void skRenderObject(struct SkelatoolObject* object, Gfx** intoDL) {
    Gfx* dl = *intoDL;

    gSPSegment(dl++, MATRIX_TRANSFORM_SEGMENT,  osVirtualToPhysical(object->boneTransforms));
    gSPDisplayList(dl++, object->displayList);

    *intoDL = dl;
}