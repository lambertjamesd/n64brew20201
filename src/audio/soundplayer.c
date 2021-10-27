#include "audio.h"
#include "soundplayer.h"
#include "soundarray.h"
#include "util/rom.h"

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

    if (alSndpGetState(&gSoundPlayer) != AL_STOPPED) {
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

SoundID soundPlayerPlay(unsigned clipId) {
    if (clipId >= gSoundClipArray->soundCount) {
        return -1;
    }

    struct ActiveSoundInfo* soundInfo = findSoundInfo(gSoundClipArray->sounds[clipId]);

    if (!soundInfo) {
        return -1;
    }

    initActiveSoundForSound(gSoundClipArray->sounds[clipId], soundInfo);

    alSndpSetSound(&gSoundPlayer, soundInfo->soundId);
    alSndpPlay(&gSoundPlayer);

    return soundInfo - gActiveSounds;
}