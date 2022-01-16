
void 
audio_mixer::Initialize(memory_arena *Arena){
    SoundMemory = MakeArena(Arena, Kilobytes(16));
    MasterVolume = V2(1);
}

void
audio_mixer::PlaySound(asset_sound_effect *Asset, mixer_sound_flags Flags, f32 Volume0, f32 Volume1){
    TicketMutexBegin(&FreeSoundMutex);
    
    if(!FirstFreeSound){
        FirstFreeSound = PushStruct(&SoundMemory, mixer_sound);
        InterlockedIncrement(&FreeSoundCounter);
    }
    
    InterlockedDecrement(&FreeSoundCounter);
    
    mixer_sound *Sound = FirstFreeSound;
    FirstFreeSound = FirstFreeSound->Next;
    
    sound_data *Data = &Asset->Sound;
    
    *Sound = {};
    
    Sound->Flags = Flags;
    Sound->Volume0 = Volume0;
    Sound->Volume1 = Volume1;
    
    Sound->Next = FirstSound;
    FirstSound = Sound;
    
    Sound->Data = Data;
    
    TicketMutexEnd(&FreeSoundMutex);
}

//~ NOTE(Tyler): THIS RUNS IN A SEPARATE THREAD!
void
audio_mixer::OutputSamples(memory_arena *WorkingMemory, os_sound_buffer *SoundBuffer){
    //TIMED_FUNCTION();
    
    ZeroMemory(SoundBuffer->Samples, SoundBuffer->SamplesToWrite*2*sizeof(*SoundBuffer->Samples));
    u32 MaxChunksToWrite = SoundBuffer->SamplesToWrite / 4;
    Assert(!(SoundBuffer->SamplesToWrite % 4));
    
    memory_arena_marker Marker = ArenaBeginMarker(WorkingMemory);
    __m128 *OutputChannel0 = PushSpecialArray(WorkingMemory, __m128, MaxChunksToWrite, ZeroAndAlign(16));
    __m128 *OutputChannel1 = PushSpecialArray(WorkingMemory, __m128, MaxChunksToWrite, ZeroAndAlign(16));
    
    mixer_sound *PreviousSound = 0;
    mixer_sound *Sound = FirstSound;
    while(Sound){
        sound_data *SoundEffect = Sound->Data;
        Assert(SoundEffect->ChannelCount == 2);
        
        u32 RemainingSamples = SoundEffect->SampleCount-Sound->SamplesPlayed;
        u32 RemainingChunks = RemainingSamples / 4;
        if(Sound->Flags & MixerSoundFlag_Loop){
            RemainingChunks = MaxChunksToWrite;
        }
        
        u32 ChunksToWrite = Minimum(MaxChunksToWrite, RemainingChunks);
        __m128 Volume0 = _mm_set1_ps(Sound->Volume0);
        __m128 Volume1 = _mm_set1_ps(Sound->Volume1);
        
        __m128 MasterVolume0 = _mm_set1_ps(MasterVolume.E[0]);
        __m128 MasterVolume1 = _mm_set1_ps(MasterVolume.E[1]);
        
        s16 *InputSample = SoundEffect->Samples + SoundEffect->ChannelCount*Sound->SamplesPlayed;
        s16 *InputEnd = SoundEffect->Samples + SoundEffect->SampleCount*2;
        __m128 *Dest0 = OutputChannel0;
        __m128 *Dest1 = OutputChannel1;
        for(u32 I=0; I < ChunksToWrite; I++){
            
            __m128 D0 = _mm_load_ps((float*)Dest0);
            __m128 D1 = _mm_load_ps((float*)Dest1);
            
            // NOTE(Tyler): It would work to save this and use it for the next iteration of the loop
            // (NextSampleValueA & NextSampleValueB)
            __m128 SampleValueA;
            __m128 SampleValueB;
            __m128 NextSampleValueA;
            __m128 NextSampleValueB;
            if(Sound->Flags & MixerSoundFlag_Loop){
#define GetNextInputSample ((InputSample > InputEnd) ? *(InputSample = SoundEffect->Samples) : 0, *InputSample++)
                SampleValueA = _mm_setr_ps(GetNextInputSample,
                                           GetNextInputSample,
                                           GetNextInputSample,
                                           GetNextInputSample);
                SampleValueB = _mm_setr_ps(GetNextInputSample,
                                           GetNextInputSample,
                                           GetNextInputSample,
                                           GetNextInputSample);
                s16 *Temp = InputSample;
                NextSampleValueA = _mm_setr_ps(GetNextInputSample,
                                               GetNextInputSample,
                                               GetNextInputSample,
                                               GetNextInputSample);
                NextSampleValueB = _mm_setr_ps(GetNextInputSample,
                                               GetNextInputSample,
                                               GetNextInputSample,
                                               GetNextInputSample);
                InputSample = Temp;
#undef GetNextInputSample
            }else{
#define GetNextInputSample *InputSample++
                SampleValueA = _mm_setr_ps(GetNextInputSample,
                                           GetNextInputSample,
                                           GetNextInputSample,
                                           GetNextInputSample);
                SampleValueB = _mm_setr_ps(GetNextInputSample,
                                           GetNextInputSample,
                                           GetNextInputSample,
                                           GetNextInputSample);
                s16 *Temp = InputSample;
                NextSampleValueA = _mm_setr_ps(GetNextInputSample,
                                               GetNextInputSample,
                                               GetNextInputSample,
                                               GetNextInputSample);
                NextSampleValueB = _mm_setr_ps(GetNextInputSample,
                                               GetNextInputSample,
                                               GetNextInputSample,
                                               GetNextInputSample);
                InputSample = Temp;
#undef GetNextInputSample
            }
            
            __m128 SampleValue0 = _mm_shuffle_ps(SampleValueA, SampleValueB, 0b10001000);
            __m128 SampleValue1 = _mm_shuffle_ps(SampleValueA, SampleValueB, 0b11011101);
            
            __m128 NextSampleValue0 = _mm_shuffle_ps(NextSampleValueA, NextSampleValueB, 0b10001000);
            __m128 NextSampleValue1 = _mm_shuffle_ps(NextSampleValueA, NextSampleValueB, 0b11011101);
            
            D0 = _mm_add_ps(D0, _mm_mul_ps(MasterVolume0, _mm_mul_ps(Volume0, SampleValue0)));
            D1 = _mm_add_ps(D1, _mm_mul_ps(MasterVolume1, _mm_mul_ps(Volume1, SampleValue1)));
            
            _mm_store_ps((float*)Dest0, D0);
            _mm_store_ps((float*)Dest1, D1);
            
            Dest0++;
            Dest1++;
        }
        
        Sound->SamplesPlayed += SoundBuffer->SamplesPerFrame;
        if((Sound->SamplesPlayed > SoundEffect->SampleCount) &&
           !(Sound->Flags & MixerSoundFlag_Loop)){
            if(PreviousSound) PreviousSound->Next = Sound->Next;
            else FirstSound = Sound->Next;
            
            TicketMutexBegin(&FreeSoundMutex);
            
            mixer_sound *Temp = Sound->Next;
            Sound->Next = 0;
            
            Sound->Next = FirstFreeSound;
            FirstFreeSound = Sound;
            
            InterlockedIncrement(&FreeSoundCounter);
            Sound = Temp;
            
            TicketMutexEnd(&FreeSoundMutex);
        }else{
            if(Sound->SamplesPlayed > SoundEffect->SampleCount){
                Sound->SamplesPlayed = 0;
            }
            PreviousSound = Sound;
            Sound = Sound->Next;
        }
    }
    
    __m128 *Source0 = OutputChannel0;
    __m128 *Source1 = OutputChannel1;
    
    __m128i *SampleOut = (__m128i *)SoundBuffer->Samples;
    for(u32 I=0; I < MaxChunksToWrite; I++){
        __m128 S0 = _mm_load_ps((float *)Source0++);
        __m128 S1 = _mm_load_ps((float *)Source1++);
        
        __m128i L = _mm_cvtps_epi32(S0);
        __m128i R = _mm_cvtps_epi32(S1);
        
        __m128i LR0 = _mm_unpacklo_epi32(L, R);
        __m128i LR1 = _mm_unpackhi_epi32(L, R);
        
        __m128i S01 = _mm_packs_epi32(LR0, LR1);
        
        *SampleOut++ = S01;
    }
    
    ArenaEndMarker(WorkingMemory, &Marker);
}