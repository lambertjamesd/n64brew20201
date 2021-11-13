#include "audio.h"
#include "soundplayer.h"
#include "soundarray.h"
#include "util/rom.h"
#include "math/mathf.h"

struct SoundArray* gSoundClipArray;
ALSndPlayer gSoundPlayer;
ALSndId gSoundIds[MAX_SOUNDS];
struct ActiveSoundInfo gActiveSounds[MAX_SOUNDS];

enum SoundMatchScore {
    SoundMatchScoreNone,
    SoundMatchScoreDifferentClip,
    SoundMatchScoreNoClip,
    SoundMatchScoreClipMatch,
};

int soundMatchScore(ALSound* forSound, struct ActiveSoundInfo* against) {
    if (!against->forSound) {
        return SoundMatchScoreNoClip;
    }

    alSndpSetSound(&gSoundPlayer, against->soundId);

    if (alSndpGetState(&gSoundPlayer) != AL_STOPPED || (against->flags & SoundPlayerFlagsLoop) != 0) {
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

    for (unsigned i = 0; i < MAX_SOUNDS; ++i) {
        enum SoundMatchScore score = soundMatchScore(forSound, &gActiveSounds[i]);

        if (!fallback && score == SoundMatchScoreDifferentClip) {
            fallback = &gActiveSounds[i];
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
}

SoundID soundPlayerPlay(unsigned clipId, enum SoundPlayerFlags flags) {
    if (clipId >= gSoundClipArray->soundCount) {
        return SOUND_ID_NONE;
    }

    struct ActiveSoundInfo* soundInfo = findSoundInfo(gSoundClipArray->sounds[clipId]);

    if (!soundInfo) {
        return SOUND_ID_NONE;
    }

    initActiveSoundForSound(gSoundClipArray->sounds[clipId], soundInfo);

    soundInfo->flags = flags;
    alSndpSetSound(&gSoundPlayer, soundInfo->soundId);
    alSndpPlay(&gSoundPlayer);
    alSndpSetPitch(&gSoundPlayer, (float)SOUND_SAMPLE_RATE / (float)SOUND_SAMPLE_RATE);

    return soundInfo - gActiveSounds;
}

void soundPlayerUpdatePosition(SoundID soundId, struct Vector3* position) {
    if (soundId == SOUND_ID_NONE) {
        return;
    }

    gActiveSounds[soundId].position = *position;
}

void soundPlayerUpdate() {
    for (unsigned i = 0; i < MAX_SOUNDS; ++i) {
        struct ActiveSoundInfo* activeSound = &gActiveSounds[i];
        
        if (activeSound->flags & SoundPlayerFlagsLoop) {
            alSndpSetSound(&gSoundPlayer, activeSound->soundId);
            if (alSndpGetState(&gSoundPlayer) == AL_STOPPED) {
                alSndpPlay(&gSoundPlayer);
            }
        }
    }
}

void soundPlayerStop(SoundID* soundId) {
    if (!soundId || *soundId == SOUND_ID_NONE) {
        return;
    }

    struct ActiveSoundInfo* soundInfo = &gActiveSounds[*soundId];
    soundInfo->flags = 0;
    alSndpSetSound(&gSoundPlayer, soundInfo->soundId);
    if (alSndpGetState(&gSoundPlayer) == AL_PLAYING) {
        alSndpStop(&gSoundPlayer);
    }
    *soundId = SOUND_ID_NONE;
}

void soundPlayerSetPitch(SoundID soundId, float speed) {
    if (soundId < 0 || soundId >= MAX_SOUNDS) {
        return;
    }

    alSndpSetSound(&gSoundPlayer, gActiveSounds[soundId].soundId);
    alSndpSetPitch(&gSoundPlayer, speed * ((float)SOUND_SAMPLE_RATE / (float)SOUND_SAMPLE_RATE));
}

void soundPlayerSetVolume(SoundID soundId, float volume) {
    if (soundId < 0 || soundId >= MAX_SOUNDS) {
        return;
    }

    int volumeAsInt = volume * 32767;

    if (volumeAsInt > 32767) {
        volumeAsInt = 32767;
    }

    if (volumeAsInt < 0) {
        volumeAsInt = 0;
    }

    alSndpSetSound(&gSoundPlayer, gActiveSounds[soundId].soundId);
    alSndpSetVol(&gSoundPlayer, volumeAsInt);
}

int soundPlayerIsPlaying(SoundID soundId) {
    if (soundId < 0 || soundId >= MAX_SOUNDS) {
        return 0;
    }

    alSndpSetSound(&gSoundPlayer, gActiveSounds[soundId].soundId);
    return alSndpGetState(&gSoundPlayer) == AL_PLAYING;
}

unsigned soundListRandom(struct SoundList* list) {
    return list->options[randomInRange(0, list->count)];
}