#ifndef _SOUND_PLAYER_H
#define _SOUND_PLAYER_H

#include <ultra64.h>
#include "math/vector3.h"
#include "math/quaternion.h"

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
    SoundPlayerFlagsIsMusic = (1 << 2),
    // survives a single soundPlayerReset
    SoundPlayerFlagsTransition = (1 << 3),
};

struct ActiveSoundInfo {
    ALSndId soundId;
    ALSound* forSound;
    unsigned short flags;
    struct Vector3 position;
    float volume;
};

struct SoundList {
    unsigned short* options;
    unsigned short count;
};

struct SoundListener {
    struct Vector3 position;
    struct Vector3 right;
};

typedef short SoundID;

void soundPlayerInit();
void soundPlayerUpdate();

void soundPlayerUpdateListener(unsigned index, struct Vector3* position, struct Quaternion* rotation);
void soundPlayerSetListenerCount(unsigned count);

SoundID soundPlayerPlay(unsigned clipId, enum SoundPlayerFlags flags, struct Vector3* pos);
void soundPlayerStopWithClipId(unsigned clipId);
void soundPlayerUpdatePosition(SoundID soundId, struct Vector3* position);
void soundPlayerStop(SoundID* soundId);
void soundPlayerSetPitch(SoundID soundId, float speed);
void soundPlayerSetVolume(SoundID soundId, float volume);
int soundPlayerIsPlaying(SoundID soundId);
void soundPlayerReset();

float soundPlayerGetSoundVolume();
float soundPlayerGetMusicVolume();

void soundPlayerSetSoundVolume(float value);
void soundPlayerSetMusicVolume(float value);

unsigned soundListRandom(struct SoundList* list);

#endif