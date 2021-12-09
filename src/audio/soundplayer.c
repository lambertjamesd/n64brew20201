#include "audio.h"
#include "soundplayer.h"
#include "soundarray.h"
#include "util/rom.h"
#include "util/time.h"
#include "math/mathf.h"
#include "game_defs.h"
#include "clips.h"

struct SoundArray* gSoundClipArray;
ALSndPlayer gSoundPlayer;
struct SoundSource gSoundSources[MAX_SOUNDS];
struct SoundSource* gSoundSourceById[SOUNDS_TOTAL_COUNT];
char gSoundSourceCount[SOUNDS_TOTAL_COUNT] = {
    [SOUNDS_FLAGCAP] = 3,
    [SOUNDS_ITEMSPAWN] = 2,
    [SOUNDS_SPAWN] = 2,
};
struct SoundListener gListeners[MAX_PLAYERS];
unsigned gListenerCount;

SoundID SoundIDNone = {0, 0};

#define FULL_VOLUME_RADIUS   (SCENE_SCALE * 20.0f)

float gSoundVolume = 1.0f;
float gMusicVolume = 1.0f;

enum SoundMatchScore {
    SoundMatchScoreNone,
    SoundMatchScoreClipMatchPlaying,
    SoundMatchScoreDifferentClip,
    SoundMatchScoreNoClip,
    SoundMatchScoreClipMatch,
};

short soundVolume(float floatValue) {
    float result = floatValue * (float)32767.0f;

    if (result > (float)32767.0f) {
        return 32767.0f;
    } else if (result < 0.0f) {
        return 0;
    } else {
        return floorf(result);
    }
}

void soundDetermine3DVolumePan(struct Vector3* position, float volumeIn, short* volume, short* pan) {
    struct SoundListener* nearestListener = 0;
    float bestDistance = 0;

    for (unsigned i = 0; i < gListenerCount; ++i) {
        float distance = vector3DistSqrd(&gListeners[i].position, position);
        if (!nearestListener || distance < bestDistance) {
            bestDistance = distance;
            nearestListener = &gListeners[i];
        }
    }

    if (!nearestListener) {
        *volume = soundVolume(volumeIn);
        *pan = 64;
        return;
    }

    *volume = soundVolume(volumeIn * FULL_VOLUME_RADIUS * FULL_VOLUME_RADIUS / bestDistance);

    struct Vector3 offset;
    vector3Sub(position, &nearestListener->position, &offset);
    vector3Normalize(&offset, &offset);
    float leftRight = vector3Dot(&offset, &nearestListener->right) * 64.0f + 64.0f;

    if (leftRight > 127.0f) {
        *pan = 127;
    } else if (leftRight < 0) {
        *pan = 0;
    } else {
        *pan = (short)leftRight;
    }
}

void soundPlayerUpdateListener(unsigned index, struct Vector3* position, struct Quaternion* rotation) {
    gListeners[index].position = *position;
    quatMultVector(rotation, &gRight, &gListeners[index].right);
}

void soundPlayerSetListenerCount(unsigned count) {
    gListenerCount = count;
}

struct SoundSource* soundGetFromID(SoundID id) {
    unsigned soundIndex = id.soundId - 1;
    
    if (soundIndex >= MAX_SOUNDS) {
        return 0;
    }

    struct SoundSource* result = &gSoundSources[soundIndex];

    if (result->soundId == SOUND_ID_NONE || result->playbackId != id.playbackId) {
        return 0;
    }

    return result;
}

void soundPlayerInit() {
    gSoundClipArray = alHeapAlloc(&gAudioHeap, 1, _soundsSegmentRomEnd - _soundsSegmentRomStart);
    romCopy(_soundsSegmentRomStart, (char*)gSoundClipArray, _soundsSegmentRomEnd - _soundsSegmentRomStart);
    soundArrayInit(gSoundClipArray, _soundsTblSegmentRomStart);

    ALSndpConfig sndConfig;
    sndConfig.maxEvents = MAX_EVENTS;
    sndConfig.maxSounds = MAX_SOUNDS;
    sndConfig.heap = &gAudioHeap;
    alSndpNew(&gSoundPlayer, &sndConfig);

    unsigned currentSoundId = 0;

    for (unsigned clipId = 0; clipId < SOUNDS_TOTAL_COUNT; ++clipId) {
        if (gSoundSourceCount[clipId] == 0) {
            gSoundSourceCount[clipId] = 1;
        }

        gSoundSourceById[clipId] = &gSoundSources[currentSoundId];

        for (unsigned i = 0; i < gSoundSourceCount[clipId]; ++i) {
            struct SoundSource* currSource = &gSoundSources[currentSoundId];
            currSource->soundId = alSndpAllocate(&gSoundPlayer, gSoundClipArray->sounds[clipId]);
            currSource->currentVolume = 0;
            currSource->flags = 0;
            currSource->playbackId = 0;
            currSource->clipId = clipId;
            currSource->absoluteVolume = 0.0f;
            currSource->position = gZeroVec;
            // currSource->endTime = 0.0f;

            ++currentSoundId;
        }
    }

    for (;currentSoundId < MAX_SOUNDS; ++currentSoundId) {
        struct SoundSource* currSource = &gSoundSources[currentSoundId];
        currSource->soundId = SOUND_ID_NONE;
    }
}

int soundSourceIsStopped(struct SoundSource* source) {
    if (source->flags & SoundPlayerFlagsFresh) {
        return 0;
    }

    alSndpSetSound(&gSoundPlayer, source->soundId);
    return alSndpGetState(&gSoundPlayer) == AL_STOPPED;
}

struct SoundSource* findSoundSource(unsigned clipId, short vol) {
    struct SoundSource* curr = gSoundSourceById[clipId];
    struct SoundSource* end = curr + gSoundSourceCount[clipId];

    struct SoundSource* backup = 0;

    for (; curr < end; ++curr) {
        if (soundSourceIsStopped(curr)) {
            return curr;
        }

        if (curr->currentVolume >= vol) {
            continue;
        }

        if (!backup || curr->currentVolume < backup->currentVolume) {
            backup = curr;
        }
    }

    return backup;
}

SoundID soundPlayerCreateID(struct SoundSource* source) {
    SoundID result;
    result.soundId = (source - gSoundSources) + 1;
    result.playbackId = source->playbackId;
    return result;
}

SoundID soundPlayerPlay(unsigned clipId, float volume, float pitch, enum SoundPlayerPriority priority, enum SoundPlayerFlags flags, struct Vector3* pos) {
    if (clipId >= gSoundClipArray->soundCount) {
        return SoundIDNone;
    }

    short vol;
    short pan;
    if (pos) {
        soundDetermine3DVolumePan(pos, gSoundVolume * volume, &vol, &pan);
    } else {
        vol = soundVolume(volume * ((flags & SoundPlayerFlagsIsMusic) ? gMusicVolume : gSoundVolume));
        pan = 64;
    }

    if (vol == 0) {
        return SoundIDNone;
    }

    struct SoundSource* soundSource = findSoundSource(clipId, vol);

    if (!soundSource) {
        return SoundIDNone;
    }

    soundSource->flags = flags;
    soundSource->absoluteVolume = volume;

    if (pos) {
        soundSource->position = *pos;
        soundSource->flags |= SoundPlayerFlags3D;
    } else {
        soundSource->flags &= ~SoundPlayerFlags3D;
    }

    soundSource->flags |= SoundPlayerFlagsFresh;

    alSndpSetSound(&gSoundPlayer, soundSource->soundId);
    int isStopped = alSndpGetState(&gSoundPlayer) == AL_STOPPED;
    if (!isStopped && !(flags & SoundPlayerFlagsLooping)) {
        alSndpStop(&gSoundPlayer);
    }
    alSndpSetPitch(&gSoundPlayer, pitch * (float)SOUND_SAMPLE_RATE / (float)OUTPUT_RATE);
    alSndpSetVol(&gSoundPlayer, vol);
    alSndpSetPan(&gSoundPlayer, pan);
    alSndpSetPriority(&gSoundPlayer, soundSource->soundId, priority);

    if (isStopped) {
        alSndpPlay(&gSoundPlayer);
    } else if (!(flags & SoundPlayerFlagsLooping)) {
        soundSource->flags |= SoundPlayerFlagsPending;
    }

    // soundSource->endTime = soundDetermineEndtime(soundSource);
    
    soundSource->playbackId++;
    return soundPlayerCreateID(soundSource);
}

void soundPlayerSetPosition(SoundID soundId, struct Vector3* position) {
    struct SoundSource* source = soundGetFromID(soundId);
    if (!source) {
        return;
    }

    source->position = *position;
    source->flags |= SoundPlayerFlags3D;
}

void soundPlayerUpdate() {
    for (unsigned i = 0; i < MAX_SOUNDS; ++i) {
        struct SoundSource* activeSound = &gSoundSources[i];

        if (activeSound->flags & SoundPlayerFlags3D) {
            if (!soundSourceIsStopped(activeSound)) {
                short vol;
                short pan;
                soundDetermine3DVolumePan(&activeSound->position, gSoundVolume * activeSound->absoluteVolume, &vol, &pan);
                activeSound->currentVolume = vol;
                alSndpSetSound(&gSoundPlayer, activeSound->soundId);
                alSndpSetVol(&gSoundPlayer, vol);
                alSndpSetPan(&gSoundPlayer, pan);
            }
        }

        activeSound->flags &= ~SoundPlayerFlagsFresh;

        if (activeSound->flags & SoundPlayerFlagsPending) {
            if (soundSourceIsStopped(activeSound)) {
                alSndpSetSound(&gSoundPlayer, activeSound->soundId);
                alSndpPlay(&gSoundPlayer);
                activeSound->flags &= ~SoundPlayerFlagsPending;
            }
        }

        // if (activeSound->endTime && activeSound->endTime < gTimePassed) {
        //     SoundID id;
        //     id.soundId = i + 1;
        //     id.playbackId = activeSound->playbackId;
        //     soundPlayerStop(&id);
        // }
    }
}

void soundPlayerStop(SoundID* soundId) {
    if (!soundId) {
        return;
    }

    struct SoundSource* source = soundGetFromID(*soundId);

    if (!source) {
        return;
    }

    source->flags = 0;
    alSndpSetSound(&gSoundPlayer, source->soundId);
    alSndpStop(&gSoundPlayer);
    *soundId = SoundIDNone;
}

void soundPlayerStopWithClipId(unsigned clipId) {
    for (unsigned i = 0; i < MAX_SOUNDS; ++i) {
        struct SoundSource* soundSource = &gSoundSources[i];

        if (soundSource->clipId == clipId) {
            alSndpSetSound(&gSoundPlayer, soundSource->soundId);
            SoundID id = soundPlayerCreateID(soundSource);
            soundPlayerStop(&id);
        }
    }
}

void soundPlayerSetPitch(SoundID soundId, float speed) {
    struct SoundSource* source = soundGetFromID(soundId);

    if (!source) {
        return;
    }

    alSndpSetSound(&gSoundPlayer, source->soundId);
    alSndpSetPitch(&gSoundPlayer, speed * ((float)SOUND_SAMPLE_RATE / (float)OUTPUT_RATE));
}

void soundPlayerSetVolume(SoundID soundId, float volume) {
    struct SoundSource* source = soundGetFromID(soundId);

    if (!source) {
        return;
    }

    source->absoluteVolume = volume;
    alSndpSetSound(&gSoundPlayer, source->soundId);

    if (source->flags & SoundPlayerFlags3D) {
        short vol;
        short pan;
        soundDetermine3DVolumePan(&source->position, gSoundVolume * source->absoluteVolume, &vol, &pan);
        source->currentVolume = vol;
        alSndpSetVol(&gSoundPlayer, vol);
        alSndpSetPan(&gSoundPlayer, pan);
    } else {
        alSndpSetVol(&gSoundPlayer, soundVolume(volume * ((source->flags & SoundPlayerFlagsIsMusic) ? gMusicVolume : gSoundVolume)));
    }
}

int soundPlayerIsPlaying(SoundID soundId) {
    struct SoundSource* source = soundGetFromID(soundId);

    if (!source) {
        return 0;
    }

    return !soundSourceIsStopped(source);
}

unsigned soundListRandom(struct SoundList* list) {
    return list->options[randomInRange(0, list->count)];
}

void soundPlayerReset() {
    for (unsigned i = 0; i < MAX_SOUNDS; ++i) {
        if (gSoundSources[i].flags & SoundPlayerFlagsTransition) {
            gSoundSources[i].flags &= ~SoundPlayerFlagsTransition;
            continue;
        }
        if (gSoundSources[i].soundId == SOUND_ID_NONE) {
            continue;
        }
        SoundID soundId = soundPlayerCreateID(&gSoundSources[i]);
        soundPlayerStop(&soundId);
    }
}

float soundPlayerGetSoundVolume() {
    return gSoundVolume;
}

float soundPlayerGetMusicVolume() {
    return gMusicVolume;
}

void soundPlayerSetSoundVolume(float value) {
    gSoundVolume = MAX(0.0f, MIN(1.0f, value));
}

void soundPlayerSetMusicVolume(float value) {
    gMusicVolume = MAX(0.0f, MIN(1.0f, value));

    for (unsigned i = 0; i < MAX_SOUNDS; ++i) {
        struct SoundSource* activeSound = &gSoundSources[i];
        
        if (activeSound->flags & SoundPlayerFlagsIsMusic) {
            soundPlayerSetVolume(soundPlayerCreateID(activeSound), 1.0f);
        }
    }
}

void soundPlayerSetLoopsActive(int value) {
    for (unsigned i = 0; i < MAX_SOUNDS; ++i) {
        struct SoundSource* activeSound = &gSoundSources[i];
        
        if (activeSound->flags & SoundPlayerFlagsLooping) {
            if (soundPlayerIsPlaying(soundPlayerCreateID(activeSound))) {
                if (value) {
                    soundPlayerSetVolume(soundPlayerCreateID(activeSound), activeSound->absoluteVolume);
                } else {
                    alSndpSetSound(&gSoundPlayer, activeSound->soundId);
                    alSndpSetVol(&gSoundPlayer, 0);
                }
            }
        }
    }
}