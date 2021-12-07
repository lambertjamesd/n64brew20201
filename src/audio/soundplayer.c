#include "audio.h"
#include "soundplayer.h"
#include "soundarray.h"
#include "util/rom.h"
#include "math/mathf.h"
#include "game_defs.h"

struct SoundArray* gSoundClipArray;
ALSndPlayer gSoundPlayer;
ALSndId gSoundIds[MAX_SOUNDS];
struct ActiveSoundInfo gActiveSounds[MAX_SOUNDS];
struct SoundListener gListeners[MAX_PLAYERS];
unsigned gListenerCount;

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

int soundIsPlaying(struct ActiveSoundInfo* sound) {
    if (!sound->forSound || sound->soundId == SOUND_ID_NONE) {
        return 0;
    }

    if (sound->flags & SoundPlayerFlagsFresh) {
        return 1;
    }

    alSndpSetSound(&gSoundPlayer, sound->soundId);
    return alSndpGetState(&gSoundPlayer) == AL_PLAYING;
}

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

int soundMatchScore(ALSound* forSound, struct ActiveSoundInfo* against) {
    if (!against->forSound) {
        return SoundMatchScoreNoClip;
    }

    alSndpSetSound(&gSoundPlayer, against->soundId);

    if (soundIsPlaying(against) || (against->flags & SoundPlayerFlagsLoop) != 0) {
        if (against->forSound == forSound) {
            return SoundMatchScoreClipMatchPlaying;
        }

        return SoundMatchScoreNone;
    }

    if (against->forSound == forSound) {
        return SoundMatchScoreClipMatch;
    } 

    return SoundMatchScoreDifferentClip;
}

void initActiveSoundForSound(ALSound* sound, struct ActiveSoundInfo* info) {
    if (info->forSound == sound) {
        return;
    }

    if (info->forSound) {
        alSndpDeallocate(&gSoundPlayer, info->soundId);
    }

    info->soundId = alSndpAllocate(&gSoundPlayer, sound);

    if (info->soundId == -1) {
        info->forSound = 0;
        return;
    }

    info->forSound = sound;
}

struct ActiveSoundInfo* findSoundInfo(ALSound* forSound) {
    struct ActiveSoundInfo* fallback = 0;
    enum SoundMatchScore fallbackScore = SoundMatchScoreNone;

    for (unsigned i = 0; i < MAX_SOUNDS; ++i) {
        enum SoundMatchScore score = soundMatchScore(forSound, &gActiveSounds[i]);

        if (!fallback && (score == SoundMatchScoreDifferentClip || score == SoundMatchScoreClipMatchPlaying)) {
            if (score > fallbackScore) {
                fallback = &gActiveSounds[i];
                fallbackScore = score;
            }
        } else if (score >= SoundMatchScoreNoClip) {
            return &gActiveSounds[i];
        }
    }

    return fallback;
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

    zeroMemory(gActiveSounds, sizeof(gActiveSounds));

    for (unsigned i = 0; i < MAX_SOUNDS; ++i) {
        gActiveSounds[i].soundId = -1;
    }
}

SoundID soundPlayerPlay(unsigned clipId, float volume, enum SoundPlayerFlags flags, struct Vector3* pos) {
    if (clipId >= gSoundClipArray->soundCount) {
        return SOUND_ID_NONE;
    }

    struct ActiveSoundInfo* soundInfo = findSoundInfo(gSoundClipArray->sounds[clipId]);

    if (!soundInfo) {
        return SOUND_ID_NONE;
    }

    initActiveSoundForSound(gSoundClipArray->sounds[clipId], soundInfo);

    soundInfo->flags = flags;
    soundInfo->volume = volume;

    if (pos) {
        soundInfo->position = *pos;
        soundInfo->flags |= SoundPlayerFlags3D;
    } else {
        soundInfo->flags &= ~SoundPlayerFlags3D;
    }

    soundInfo->flags |= SoundPlayerFlagsFresh;

    alSndpSetSound(&gSoundPlayer, soundInfo->soundId);
    alSndpSetPitch(&gSoundPlayer, (float)SOUND_SAMPLE_RATE / (float)OUTPUT_RATE);
    if (flags & SoundPlayerFlags3D) {
        short vol;
        short pan;
        soundDetermine3DVolumePan(&soundInfo->position, gSoundVolume * volume, &vol, &pan);
        alSndpSetVol(&gSoundPlayer, vol);
        alSndpSetPan(&gSoundPlayer, pan);
    } else {
        alSndpSetVol(&gSoundPlayer, soundVolume(volume * ((flags & SoundPlayerFlagsIsMusic) ? gMusicVolume : gSoundVolume)));
        alSndpSetPan(&gSoundPlayer, 64);
    }
    alSndpPlay(&gSoundPlayer);
    
    return soundInfo - gActiveSounds;
}

void soundPlayerUpdatePosition(SoundID soundId, struct Vector3* position) {
    if (soundId == SOUND_ID_NONE) {
        return;
    }

    gActiveSounds[soundId].position = *position;
    gActiveSounds[soundId].flags |= SoundPlayerFlags3D;
}

void soundPlayerUpdate() {
    for (unsigned i = 0; i < MAX_SOUNDS; ++i) {
        struct ActiveSoundInfo* activeSound = &gActiveSounds[i];

        if (!activeSound->forSound) {
            continue;
        }
        
        if (activeSound->flags & SoundPlayerFlagsLoop) {
            if (!soundIsPlaying(activeSound)) {
                alSndpPlay(&gSoundPlayer);
            }
        }

        if (activeSound->flags & SoundPlayerFlags3D) {
            if (soundIsPlaying(activeSound)) {
                short vol;
                short pan;
                soundDetermine3DVolumePan(&activeSound->position, gSoundVolume * activeSound->volume, &vol, &pan);
                alSndpSetVol(&gSoundPlayer, vol);
                alSndpSetPan(&gSoundPlayer, pan);
            }
        }

        activeSound->flags &= ~SoundPlayerFlagsFresh;
    }
}

void soundPlayerStop(SoundID* soundId) {
    if (!soundId || *soundId == SOUND_ID_NONE) {
        return;
    }

    struct ActiveSoundInfo* soundInfo = &gActiveSounds[*soundId];
    soundInfo->flags = 0;
    alSndpSetSound(&gSoundPlayer, soundInfo->soundId);
    if (soundIsPlaying(soundInfo)) {
        alSndpStop(&gSoundPlayer);
    }
    *soundId = SOUND_ID_NONE;
}

void soundPlayerStopWithClipId(unsigned clipId) {
    for (unsigned i = 0; i < MAX_SOUNDS; ++i) {
        struct ActiveSoundInfo* activeSound = &gActiveSounds[i];

        if (activeSound->forSound == gSoundClipArray->sounds[clipId]) {
            alSndpSetSound(&gSoundPlayer, activeSound->soundId);
            if (soundIsPlaying(activeSound)) {
                SoundID id = i;
                soundPlayerStop(&id);
            }
        }
    }
}

void soundPlayerSetPitch(SoundID soundId, float speed) {
    if (soundId < 0 || soundId >= MAX_SOUNDS) {
        return;
    }

    alSndpSetSound(&gSoundPlayer, gActiveSounds[soundId].soundId);
    alSndpSetPitch(&gSoundPlayer, speed * ((float)SOUND_SAMPLE_RATE / (float)OUTPUT_RATE));
}

void soundPlayerSetVolume(SoundID soundId, float volume) {
    if (soundId < 0 || soundId >= MAX_SOUNDS) {
        return;
    }

    struct ActiveSoundInfo* soundInfo = &gActiveSounds[soundId];

    soundInfo->volume = volume;
    alSndpSetSound(&gSoundPlayer, soundInfo->soundId);

    if (gActiveSounds[soundId].flags & SoundPlayerFlags3D) {
        short vol;
        short pan;
        soundDetermine3DVolumePan(&soundInfo->position, gSoundVolume * soundInfo->volume, &vol, &pan);
        alSndpSetVol(&gSoundPlayer, vol);
        alSndpSetPan(&gSoundPlayer, pan);
    } else {
        alSndpSetVol(&gSoundPlayer, soundVolume(volume * ((gActiveSounds[soundId].flags & SoundPlayerFlagsIsMusic) ? gMusicVolume : gSoundVolume)));
    }
}

int soundPlayerIsPlaying(SoundID soundId) {
    if (soundId < 0 || soundId >= MAX_SOUNDS) {
        return 0;
    }

    return soundIsPlaying(&gActiveSounds[soundId]);
}

unsigned soundListRandom(struct SoundList* list) {
    return list->options[randomInRange(0, list->count)];
}

void soundPlayerReset() {
    for (unsigned i = 0; i < MAX_SOUNDS; ++i) {
        if (gActiveSounds[i].flags & SoundPlayerFlagsTransition) {
            gActiveSounds[i].flags &= ~SoundPlayerFlagsTransition;
            continue;
        }
        SoundID soundId = i;
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
        struct ActiveSoundInfo* activeSound = &gActiveSounds[i];
        
        if (activeSound->flags & SoundPlayerFlagsIsMusic) {
            soundPlayerSetVolume(i, 1.0f);
        }
    }
}