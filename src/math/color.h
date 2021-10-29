#ifndef _MATH_COLOR_H
#define _MATH_COLOR_H

struct Coloru8 {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
};

extern struct Coloru8 gColorBlack;

void colorU8Lerp(struct Coloru8* from, struct Coloru8* to, float lerp, struct Coloru8* output);

#endif