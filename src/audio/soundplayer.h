#ifndef _SOUND_PLAYER_H
#define _SOUND_PLAYER_H

#include <ultra64.h>

#define MAX_SOUNDS 32

extern char _soundsSegmentRomStart[];
extern char _soundsSegmentRomEnd[];
extern char _soundsTblSegmentRomStart[];
extern char _soundsTblSegmentRomEnd[];

struct ActiveSoundInfo {
    ALSndId soundId;
    ALSound* forSound;
};

typedef int SoundID;

void soundPlayerInit();

SoundID soundPlayerPlay(unsigned clipId);

#endif