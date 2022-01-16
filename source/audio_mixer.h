#ifndef AUDIO_MIXER_H
#define AUDIO_MIXER_H

typedef u8 mixer_sound_flags;
enum mixer_sound_flags_ {
    MixerSoundFlag_None = (0 << 0),
    MixerSoundFlag_Loop = (1 << 0),
};

struct mixer_sound {
    mixer_sound_flags Flags;
    sound_data *Data;
    u32 SamplesPlayed;
    
    f32 Volume0;
    f32 Volume1;
    
    mixer_sound *Next;
};

struct audio_mixer {
    memory_arena SoundMemory;
    
    mixer_sound *FirstSound;
    mixer_sound *FirstFreeSound;
    u32 FreeSoundCounter;
    
    ticket_mutex FreeSoundMutex;
    
    v2 MasterVolume;
    
    void PlaySound(asset_sound_effect *Asset, mixer_sound_flags Flags=MixerSoundFlag_None, f32 Volume1=1.0f, f32 Volume2=1.0f);
    
    //~
    void Initialize(memory_arena *Arena);
    void OutputSamples(memory_arena *WorkingMemory, os_sound_buffer *SoundBuffer);
};

#endif //AUDIO_MIXER_H
