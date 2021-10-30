#include "dynamic_music.h"
#include "audio.h"
#include "util/time.h"
#include "util/memory.h"

extern ALSeqPlayer	   *gSequencePlayer;

struct DynamicMusicState gCurrentMusicState;

void dynamicMusicUpdate() {
    if (gCurrentMusicState.currentMarkers) {
        dynamicMusicApplyMarkers(gCurrentMusicState.currentMarkers, gCurrentMusicState.markerCount, gCurrentMusicState.currentIntensity);
    }
}

void dynamicMusicUseMarkers(struct DynamicMarker* markers, unsigned short markerCount) {
    gCurrentMusicState.currentMarkers = markers;
    gCurrentMusicState.markerCount = markerCount;
}

void dynamicMusicSetIntensity(unsigned short intensity) {
    gCurrentMusicState.currentIntensity = intensity;
}

unsigned short dynamicMusicGetIntensity() {
    return gCurrentMusicState.currentIntensity;
}

void dynamicMusicApplyMarkers(struct DynamicMarker* markers, unsigned count, unsigned short intensity) {
    unsigned atIndex = 0;

    while (atIndex < count && markers[atIndex].intensity <= intensity) {
        ++atIndex;
    }

    unsigned char channelVolume[CHANNEL_COUNT];

    if (atIndex + 1 >= count) {
        memCopy(channelVolume, markers[count-1].channelVolumes, CHANNEL_COUNT);
    } else if (atIndex == 0) {
        memCopy(channelVolume, markers[0].channelVolumes, CHANNEL_COUNT);
    } else {
        unsigned prevWeight = markers[atIndex].intensity - intensity;
        unsigned nextWeight = intensity - markers[atIndex - 1].intensity;
        unsigned totalWeight = markers[atIndex].intensity - markers[atIndex - 1].intensity;

        for (unsigned i = 0; i < CHANNEL_COUNT; ++i) {
            channelVolume[i] = (unsigned char)((prevWeight * markers[atIndex - 1].channelVolumes[i] + nextWeight * markers[atIndex].channelVolumes[i]) / totalWeight);
        }
    }

    for (unsigned i = 0; i < CHANNEL_COUNT; ++i) {
        if (alSeqpGetChlVol(gSequencePlayer, i) != channelVolume[i]) {
            alSeqpSetChlVol(gSequencePlayer, i, channelVolume[i]);
        }
    }
}