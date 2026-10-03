#ifndef H_AUDIO
#define H_AUDIO

#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>
#include "resource.h"
#include "arena.h"

typedef struct TrackID {
    int index;
    uint32_t version;
} TrackID;

typedef struct Track { // Chunk of data for the pool of tracks in the Audio Engine
    bool inUse;
    bool stopped;
    TrackID id;
    MIX_Track* track;   // Pointer to the MIX_Track that the audio is in, if inUse is true
    struct Track* next; // Pointer to the next available track, if inUse is false
} Track;


typedef struct AudioBus {
    Track* tracks; // Data for the pool of tracks
    size_t maxTracks; // Size of the pool
    Track* firstTrack; // Pointer to the first unused track in the pool
    MIX_Group* group; // Group of all the tracks in the pool
} AudioBus;

typedef struct Sound {
    MIX_Audio** audioData; // Start of audio files for the sound (from audioData array in AudioSystem)
    size_t soundCount;
    int lastUsedSound;
} Sound;

typedef struct AudioSystem {
    MIX_Mixer* mixer;

    // Stores all of the actual data for the tracks.
    // When a bus is created, it takes the amount of tracks it needs & increments the trackCount variable.
    // This lets me pre-initialize all of the tracks and still allow buses to decide how many tracks they need after the system is initialized.
    AudioBus* buses;
    size_t busCount;
    size_t busCapacity;

    ResourceCache* cache;

    MIX_Audio** audioData;
    int audioDataCount;
} AudioSystem;

AudioSystem* InitAudioSystem(Arena* arena, MIX_Mixer* mixer, size_t busCount, size_t maxSounds, size_t maxAudio);

int AddAudioBus(Arena* arena, AudioSystem* audio, size_t trackCount);
ResourceHandle GetSound(AudioSystem* audio, char* path);
void ReleaseSound(ResourceHandle handle);

TrackID PlaySound(AudioSystem* audio, ResourceHandle sound, int bus, float volumeDB);
TrackID PlaySoundPro(AudioSystem* audio, ResourceHandle sound, int bus, SDL_PropertiesID options, float volumeDB);
void SetTrackVolume(AudioSystem* audio, int bus, TrackID id, float volumeDB);

void UpdateAudioSystem(AudioSystem* audio);
void ResetAudioSystem(AudioSystem* audio);

/*
IDEAL END-USER API:

int sfx = AddAudioBus(arena, audio, 5);

// playing a sound:
ResourceHandle sound = GetSound(audio, "assets/examplesound.ogg", false);
PlaySound(audio, sound, sfx, -10);

// playing a sound bank:
ResourceHandle soundBank = LoadSound(audio, "assets/sound_bank_folder", false);
PlaySound(audio, soundBank, sfx, -10);audio

*/

#endif
