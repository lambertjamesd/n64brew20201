#ifndef _DYNAMIC_MUSIC_H
#define _DYNAMIC_MUSIC_H

#define CHANNEL_COUNT     16

struct DynamicMarker {
    unsigned short intensity;
    unsigned char channelVolumes[CHANNEL_COUNT];
};

struct DynamicMusicState {
    struct DynamicMarker* currentMarkers;
    unsigned short markerCount;
    unsigned short currentIntensity;
};

void dynamicMusicUpdate();
void dynamicMusicUseMarkers(struct DynamicMarker* markers, unsigned short markerCount);
void dynamicMusicSetIntensity(unsigned short intensity);
unsigned short dynamicMusicGetIntensity();
void dynamicMusicApplyMarkers(struct DynamicMarker* markers, unsigned count, unsigned short intensity);

#endif