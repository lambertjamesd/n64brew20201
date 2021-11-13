#ifndef _SOUND_PLAYER_H
#define _SOUND_PLAYER_H

#include <ultra64.h>
#include "math/vector3.h"

#define MAX_SOUNDS 32

#define SOUND_SAMPLE_RATE 22500

#define SOUND_ID_NONE -1

extern char _soundsSegmentRomStart[];
extern char _soundsSegmentRomEnd[];
extern char _soundsTblSegmentRomStart[];
extern char _soundsTblSegmentRomEnd[];

enum SoundPlayerFlags {
    SoundPlayerFlagsLoop = (1 << 0),
    SoundPlayerFlags3D = (1 << 1),
};

struct ActiveSoundInfo {
    ALSndId soundId;
    ALSound* forSound;
    unsigned short flags;
    struct Vector3 position;
};

struct SoundList {
    unsigned short* options;
    unsigned short count;
};

typedef short SoundID;

void soundPlayerInit();
void soundPlayerUpdate();

SoundID soundPlayerPlay(unsigned clipId, enum SoundPlayerFlags flags);
void soundPlayerUpdatePosition(SoundID soundId, struct Vector3* position);
void soundPlayerStop(SoundID* soundId);
void soundPlayerSetPitch(SoundID soundId, float speed);
void soundPlayerSetVolume(SoundID soundId, float volume);
int soundPlayerIsPlaying(SoundID soundId);

unsigned soundListRandom(struct SoundList* list);

#endif