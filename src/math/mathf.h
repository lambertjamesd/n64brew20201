
#ifndef _MATH_MATHF_H
#define _MATH_MATHF_H

int randomInt();

float fsign(float in);
float mathfLerp(float from, float to, float t);
float mathfMoveTowards(float from, float to, float maxMove);
float mathfBounceBackLerp(float t);
float mathfRandomFloat();
float mathfMod(float input, float divisor);
float clampf(float input, float min, float max);

float sqrtf(float in);

float cosf(float in);
float sinf(float in);
float fabs(float in);
float floorf(float in);

#endif