#pragma pack(push, 1)
struct wav_header {
    char RIFF[4]; // 'R', 'I', 'F', 'F'
    u32  FileSize;
    char WAVE[4]; // 'W', 'A', 'V', 'E'
};

struct wav_fmt_chunk {
    u32 FmtSize;
    u16 TypeOfFormat;
    u16 ChannelCount;
    u32 SampleRate;
    u32 ByteRate;
    u16 BlockAlign;
    u16 BitsPerSample;
};

struct wav_data_chunk {
    u32 DataSize;
};
#pragma pack(pop)

internal inline b8
CheckChunkID(char ID[4], char *TestID){
    return ((ID[0] == TestID[0]) &&
            (ID[1] == TestID[1]) &&
            (ID[2] == TestID[2]) &&
            (ID[3] == TestID[3]));
}

internal sound_data
LoadWavFile(os_sound_buffer *SoundBuffer, memory_arena *Arena, const char *Path){
    sound_data Result = {};
    
    os_file *OSFile = OpenFile(Path, OpenFile_Read);
    if(!OSFile){
        return Result;
    }
    
    CloseFile(OSFile);
    entire_file File = ReadEntireFile(&TransientStorageArena, Path);
    stream Stream = MakeReadStream(File.Data, File.Size);
    wav_header *Header = ConsumeType(&Stream, wav_header);
    if(!CheckChunkID(Header->RIFF, "RIFF")){
        Assert(0);
    }
    
    if(!CheckChunkID(Header->WAVE, "WAVE")){
        Assert(0)
    }
    
    wav_fmt_chunk *FmtChunk = 0;
    wav_data_chunk *DataChunk = 0;
    while((!FmtChunk || !DataChunk) && PeekBytes(&Stream, 4)){
        char *ChunkID = (char *)ConsumeBytes(&Stream, 4);
        
        if(CheckChunkID(ChunkID, "fmt ")){
            FmtChunk = ConsumeType(&Stream, wav_fmt_chunk);
            
            if(FmtChunk->FmtSize != 16){
                Assert(0);
            }
            
            if(FmtChunk->TypeOfFormat != 1){
                Assert(0);
            }
            
            if(FmtChunk->SampleRate != SoundBuffer->SampleRate){
                Assert(0);
            }
        }else if(CheckChunkID(ChunkID, "data")){
            DataChunk = ConsumeType(&Stream, wav_data_chunk);
        }else if(CheckChunkID(ChunkID, "JUNK")){
            u32 ChunkSize = *ConsumeType(&Stream, u32);
            ConsumeBytes(&Stream, ChunkSize);
        }
    }
    
    Result.ChannelCount = FmtChunk->ChannelCount;
    Result.SampleCount = DataChunk->DataSize/(FmtChunk->BitsPerSample/8)/Result.ChannelCount;
    Result.Samples = PushSpecialArray(Arena, s16, Result.SampleCount*Result.ChannelCount, ZeroAndAlign(16)); 
    u8 *Data = ConsumeBytes(&Stream, DataChunk->DataSize);
    Assert(Data);
    
    if(FmtChunk->BitsPerSample == 8){
        for(u32 I=0; I<Result.SampleCount; I++){
            Result.Samples[I] = 2*Data[I];
        }
    }else if(FmtChunk->BitsPerSample == 16){
        CopyMemory(Result.Samples, Data, DataChunk->DataSize);
    }else if(FmtChunk->BitsPerSample == 32){
        for(u32 I=0; I<Result.SampleCount; I++){
            Result.Samples[I] = Data[I]/2;
        }
    }
    
    return Result;
}