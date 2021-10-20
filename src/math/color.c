#include "color.h"
#include "mathf.h"

void colorU8Lerp(struct Coloru8* from, struct Coloru8* to, float lerp, struct Coloru8* output) {
    output->r = (unsigned char)(mathfLerp(from->r, to->r, lerp));
    output->g = (unsigned char)(mathfLerp(from->g, to->g, lerp));
    output->b = (unsigned char)(mathfLerp(from->b, to->b, lerp));
    output->a = (unsigned char)(mathfLerp(from->a, to->a, lerp));
}