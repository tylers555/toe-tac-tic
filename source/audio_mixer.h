#ifndef AUDIO_MIXER_H
#define AUDIO_MIXER_H

struct mixer_sound {
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
    
    mixer_sound *FirstSoundToAdd;
    
    void Initialize(memory_arena *Arena);
    void OutputSamples(memory_arena *WorkingMemory, os_sound_buffer *SoundBuffer);
    
    void PlaySound(asset_sound_effect *Asset, f32 Volume1=1.0f, f32 Volume2=1.0f);
};

#endif //AUDIO_MIXER_H
