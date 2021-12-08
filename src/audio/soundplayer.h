#ifndef _SOUND_PLAYER_H
#define _SOUND_PLAYER_H

#include <ultra64.h>
#include "math/vector3.h"
#include "math/quaternion.h"

#define MAX_SOUNDS 128

#define SOUND_SAMPLE_RATE 22500

#define SOUND_ID_NONE -1

extern char _soundsSegmentRomStart[];
extern char _soundsSegmentRomEnd[];
extern char _soundsTblSegmentRomStart[];
extern char _soundsTblSegmentRomEnd[];

enum SoundPlayerFlags {
    SoundPlayerFlags3D = (1 << 1),
    SoundPlayerFlagsIsMusic = (1 << 2),
    // survives a single soundPlayerReset
    SoundPlayerFlagsTransition = (1 << 3),
    // to avoid reusing a sound effect that was just used
    SoundPlayerFlagsFresh = (1 << 4),
    SoundPlayerFlagsPending = (1 << 5),
    // Use if sound contains loop information
    SoundPlayerFlagsLooping = (1 << 6),
};

enum SoundPlayerPriority {
    SoundPlayerPriorityBackground,
    SoundPlayerPriorityNonPlayer,
    SoundPlayerPriorityPlayer,
    SoundPlayerPriorityMusic,
};

struct SoundSource {
    ALSndId soundId;
    short currentVolume;
    unsigned short flags;
    short playbackId;
    short clipId;
    float absoluteVolume;
    // float endTime;
    struct Vector3 position;
};

struct SoundList {
    unsigned short* options;
    unsigned short count;
};

struct SoundListener {
    struct Vector3 position;
    struct Vector3 right;
};

typedef struct {
    unsigned short soundId;
    unsigned short playbackId;
} SoundID;

extern SoundID SoundIDNone;

void soundPlayerInit();
void soundPlayerUpdate();

void soundPlayerUpdateListener(unsigned index, struct Vector3* position, struct Quaternion* rotation);
void soundPlayerSetListenerCount(unsigned count);

SoundID soundPlayerPlay(unsigned clipId, float volume, enum SoundPlayerPriority priority, enum SoundPlayerFlags flags, struct Vector3* pos);
void soundPlayerStopWithClipId(unsigned clipId);
void soundPlayerSetPosition(SoundID soundId, struct Vector3* position);
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