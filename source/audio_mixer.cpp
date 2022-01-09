
void 
audio_mixer::Initialize(memory_arena *Arena){
    Memory = MakeArena(Arena, Kilobytes(16));
}

void
audio_mixer::PlaySound(asset_sound_effect *Asset, f32 Volume1, f32 Volume2){
    if(!FirstFreeSound){
        FirstFreeSound = PushStruct(&Memory, mixer_sound);
    }
    
    sound_data *Data = &Asset->Sound;
    
    mixer_sound *Sound = FirstFreeSound;
    *Sound = {};
    FirstFreeSound = Sound->Next;
    Sound->Volume1 = Volume1;
    Sound->Volume2 = Volume2;
    
    Sound->Next = FirstSound;
    FirstSound = Sound;
    
    Sound->Data = Data;
}

void
audio_mixer::Output(os_sound_buffer *SoundBuffer){
    TIMED_FUNCTION();
    
    mixer_sound *PreviousSound = 0;
    mixer_sound *Sound = FirstSound;
    while(Sound){
        sound_data *SoundEffect = Sound->Data;
        
        s16 *DestSample = (s16 *)SoundBuffer->Samples;
        for(u32 I=0; I < SoundBuffer->SamplesToWrite; I++){
            if(Sound->SamplesPlayed < SoundEffect->SampleCount){
                s16 *InputSample = SoundEffect->Samples + SoundEffect->ChannelCount*Sound->SamplesPlayed;
                if(SoundEffect->ChannelCount == 1){
                    *DestSample++ += *InputSample;
                    *DestSample++ += *InputSample;
                }else{
                    *DestSample++ += *InputSample++;
                    *DestSample++ += *InputSample++;
                }
            }else{
                if(PreviousSound) PreviousSound->Next = Sound->Next;
                else FirstSound = Sound->Next;
                
                mixer_sound *Temp = Sound->Next;
                Sound->Next = FirstFreeSound;
                FirstFreeSound = Sound;
                
                Sound = Temp;
                goto end_of_loop_;
            }
            Sound->SamplesPlayed++;
        }
        
        PreviousSound = Sound;
        Sound = Sound->Next;
        
        end_of_loop_:;
    }
}