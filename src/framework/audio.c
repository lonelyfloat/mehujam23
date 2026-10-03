#include "audio.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

void LoadSoundCall(void** out, char* path, void* data);
void UnloadSoundCall(void* data);

#define MAX_AUDIO_CACHE_MEM 4000

AudioSystem* InitAudioSystem(Arena* arena, MIX_Mixer* mixer, size_t maxBuses, size_t maxSounds, size_t maxAudio) {
    AudioSystem* result = ArenaAllocStruct(arena, AudioSystem);
    result->mixer = mixer;

    result->cache = AllocResourceCache(arena, MAX_AUDIO_CACHE_MEM, maxSounds, sizeof(Sound), LoadSoundCall, UnloadSoundCall);
    result->buses = ArenaAllocArray(arena, AudioBus, maxBuses);
    for(int i = 0; i < maxBuses; ++i) {
        result->buses[i] = (AudioBus) {
            .firstTrack = NULL,
            .group = NULL,
            .tracks = NULL,
            .maxTracks = -1
        };
    }
    result->busCapacity = maxBuses;
    result->busCount = 0;

    result->audioData = ArenaAllocArray(arena, MIX_Audio*, maxAudio);
    result->audioDataCount = 0;

    return result;
}

void OnTrackStopCallback(void* userdata, MIX_Track* track) {
    Track* t = userdata;
    t->stopped = true;
    t->id.version++;
}

int AddAudioBus(Arena* arena, AudioSystem* audioSystem, size_t trackCount) {
    int idx = audioSystem->busCount;
    audioSystem->buses[idx].group = MIX_CreateGroup(audioSystem->mixer);
    if(!audioSystem->buses[idx].group) {
        fprintf(stderr,"Couldn't create mix group: %s\n", SDL_GetError());
    }
    audioSystem->buses[idx].maxTracks = trackCount;
    audioSystem->buses[idx].tracks = ArenaAllocArray(arena, Track, trackCount);
    audioSystem->buses[idx].firstTrack = &audioSystem->buses[idx].tracks[0];
    for(int i = 0; i < trackCount; ++i) {
        audioSystem->buses[idx].tracks[i].inUse = false;
        audioSystem->buses[idx].tracks[i].stopped = false;
        audioSystem->buses[idx].tracks[i].track = MIX_CreateTrack(audioSystem->mixer);
        if(!audioSystem->buses[idx].tracks[i].track) {
            fprintf(stderr, "Unable to create track: %s\n", SDL_GetError());
        }
        audioSystem->buses[idx].tracks[i].id = (TrackID){.index=i, .version=0};
        MIX_SetTrackGroup(audioSystem->buses[idx].tracks[i].track, audioSystem->buses[idx].group);
        MIX_SetTrackStoppedCallback(audioSystem->buses[idx].tracks[i].track, OnTrackStopCallback, &audioSystem->buses[idx].tracks[i]);
        if(i < trackCount - 1) {
            audioSystem->buses[idx].tracks[i].next = &audioSystem->buses[idx].tracks[i+1];
        }
    }
    audioSystem->buses[idx].tracks[trackCount-1].next = NULL;
    audioSystem->busCount++;
    return idx;
}

// Unloads all of the audio, tracks, and groups
void ResetAudioSystem(AudioSystem* audioSystem) {
    for(int i = 0; i < audioSystem->busCount; ++i) {
        MIX_DestroyGroup(audioSystem->buses[i].group);
        for(int j = 0; j < audioSystem->buses[i].maxTracks; ++j) {
            MIX_DestroyTrack(audioSystem->buses[i].tracks[j].track);
        }
    }
    audioSystem->busCount = 0;
    ClearCache(audioSystem->cache);
}

static inline float dBToVolume(float dB) {
    return powf(10.0f, 0.05f * dB);
}

static inline float VolumeTodB(float volume){
    return 20.0f * log10f(volume);
}

TrackID PlaySoundPro(AudioSystem* audioSystem, ResourceHandle handle, int bus, SDL_PropertiesID options, float volumeDB) {
    if(audioSystem->buses[bus].firstTrack == NULL) {
        fprintf(stderr, "ERROR: Unable to play sound because AudioBus is full. Increase the bus count.\n");
        return (TrackID){-1,0};
    }
    Track* newTrack = audioSystem->buses[bus].firstTrack;
    TrackID nextID = newTrack->id;
    audioSystem->buses[bus].firstTrack = newTrack->next;

    Sound* sound = GetResource(handle);
    if(sound->audioData == NULL) {
        return nextID;
    }
    MIX_Audio* audio = sound->audioData[0];
    if(sound->soundCount > 1) { // Choose a random audio file from the list, that is not the one that was last played
        int idx = SDL_rand(sound->soundCount);
        while(idx == sound->lastUsedSound) {
            idx = SDL_rand(sound->soundCount);
        }
        audio = sound->audioData[idx];
        sound->lastUsedSound = idx;
    }

    MIX_SetTrackAudio(newTrack->track, audio);
    MIX_SetTrackGain(newTrack->track, dBToVolume(volumeDB));
    if(!MIX_PlayTrack(newTrack->track, options)) {
        fprintf(stderr,"Unable to play track:  %s\n", SDL_GetError());
    }
    newTrack->inUse = true;
    return nextID;
}

void SetTrackVolume(AudioSystem* audioSystem, int bus, TrackID id, float volumeDB) {
    if(id.index < 0) {
        return;
    }
    Track* t = &audioSystem->buses[bus].tracks[id.index];
    if(t->id.version != id.version) {
        return;
    }
    MIX_SetTrackGain(t->track, dBToVolume(volumeDB));
}


TrackID PlaySound(AudioSystem* audioSystem, ResourceHandle handle, int bus, float volumeDB) {
    SDL_PropertiesID options = SDL_CreateProperties();
    TrackID result = PlaySoundPro(audioSystem, handle, bus, options, volumeDB);
    SDL_DestroyProperties(options);
    return result;
}

void UpdateAudioSystem(AudioSystem* audioSystem) {
    // For each bus, check if each track has stopped. If it has, add it to the free list.
    for(int b = 0; b < audioSystem->busCount; ++b) {
        for(int t = 0; t < audioSystem->buses[b].maxTracks; ++t) {
            if(audioSystem->buses[b].tracks[t].inUse && audioSystem->buses[b].tracks[t].stopped) {
                audioSystem->buses[b].tracks[t].next = audioSystem->buses[b].firstTrack;
                audioSystem->buses[b].tracks[t].stopped = false;
                audioSystem->buses[b].tracks[t].inUse = false;
                audioSystem->buses[b].firstTrack = &audioSystem->buses[b].tracks[t];
            }
        }
    }
}

// Load sound callback stuff

typedef struct SoundBankData {
    Sound* sound;
    AudioSystem* audioSystem;
} SoundBankData;

SDL_EnumerationResult LoadSoundBankCallback(void* userdata, const char *dirname, const char *fname) {
    SoundBankData* sd = userdata;
    char fileBuf[512];
    sprintf(fileBuf, "%s%s", dirname, fname);
    sd->audioSystem->audioData[sd->audioSystem->audioDataCount] = MIX_LoadAudio(sd->audioSystem->mixer, fileBuf, true);
    if(sd->sound->soundCount == 0) {
        // Make the sound point to the first element
        sd->sound->audioData = &sd->audioSystem->audioData[sd->audioSystem->audioDataCount];
        sd->sound->lastUsedSound = -1;
    }

    if(!sd->sound->audioData[sd->sound->soundCount]) {
        fprintf(stderr,"Unable to load audio file: %s\n", SDL_GetError());
        return SDL_ENUM_FAILURE;
    } 
    sd->sound->soundCount++;
    sd->audioSystem->audioDataCount++;
    return SDL_ENUM_CONTINUE;
}

ResourceHandle GetSound(AudioSystem* audioSystem, char* path) {
    ResourceHandle handle = GetResourceHandle(audioSystem->cache, path, audioSystem);
    return handle;
}

void LoadSoundCall(void** out, char* path, void* data) {
    Sound* output = *out;
    AudioSystem* audioSystem = data;

    // Check the asset path to see if it's a folder or a file
    SDL_Storage* storage = SDL_OpenTitleStorage("", 0);
    while (!SDL_StorageReady(storage)) {
        SDL_Delay(1);
    }

    SDL_PathInfo info = {};
    if(!SDL_GetStoragePathInfo(storage, path, &info)) {
        fprintf(stderr, "Unable to obtain path info: %s\n", SDL_GetError());
    }

    if(info.type == SDL_PATHTYPE_FILE) { // If the path is one file

        audioSystem->audioData[audioSystem->audioDataCount] = MIX_LoadAudio(audioSystem->mixer, path, true);
        output->audioData[0] = audioSystem->audioData[audioSystem->audioDataCount];
        if(!output->audioData[0]) {
            fprintf(stderr,"Unable to load audio file: %s\n", SDL_GetError());
        } 
        audioSystem->audioDataCount++;
        output->lastUsedSound = -1;
        output->soundCount = 1;
    } 
    else if (info.type == SDL_PATHTYPE_DIRECTORY) { // If the path is one folder
        output->soundCount = 0;
        output->lastUsedSound = -1;
        SoundBankData sd = {.sound = output, .audioSystem = audioSystem};
        SDL_EnumerateStorageDirectory(storage, path, LoadSoundBankCallback, &sd);
    }

    SDL_CloseStorage(storage);
}


// Unload sound callback

void UnloadSoundCall(void* data) {
    Sound* sound = data;
    for(int i = 0; i < sound->soundCount; ++i) {
        MIX_DestroyAudio(sound->audioData[i]);
    }
}
