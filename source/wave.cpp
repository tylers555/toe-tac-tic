#pragma pack(push, 1)
struct wav_header {
    char RIFF[4]; // 'R', 'I', 'F', 'F'
    u32  FileSize;
    char WAVE[4]; // 'W', 'A', 'V', 'E'
    char Fmt[4]; // 'f', 'm', 't', 0
    u32 FmtSize;
    u16 TypeOfFormat;
    u16 ChannelCount;
    u32 SampleRate;
    u32 ByteRate;
    u16 BlockAlign;
    u16 BitsPerSample;
    char Data[4]; // 'd', 'a', 't', 'a'
    u32 DataSize;
};
#pragma pack(pop)

struct sound_data {
    s16 *Samples;
    u16 ChannelCount;
    u32 SampleCount;
};

internal sound_data
LoadWaveFile(memory_arena *Arena, const char *Path){
    os_file *OSFile = OpenFile(Path, OpenFile_Read);
    if(!OSFile){
        Assert(0);
    }
    CloseFile(OSFile);
    entire_file File = ReadEntireFile(&TransientStorageArena, Path);
    stream Stream = MakeReadStream(File.Data, File.Size);
    wav_header *Header = ConsumeType(&Stream, wav_header);
    if(!((Header->RIFF[0] == 'R') &&
         (Header->RIFF[1] == 'I') &&
         (Header->RIFF[2] == 'F') &&
         (Header->RIFF[3] == 'F'))){
        Assert(0);
    }
    
    if(!((Header->WAVE[0] == 'W') &&
         (Header->WAVE[1] == 'A') &&
         (Header->WAVE[2] == 'V') &&
         (Header->WAVE[3] == 'E'))){
        Assert(0)
    }
    
    if(!((Header->Fmt[0] == 'f') &&
         (Header->Fmt[1] == 'm') &&
         (Header->Fmt[2] == 't') &&
         (Header->Fmt[3] == ' '))){
        Assert(0)
    }
    
    if(Header->FmtSize != 16){
        Assert(0);
    }
    
    if(Header->TypeOfFormat != 1){
        Assert(0);
    }
    
    if(Header->TypeOfFormat != 1){
        Assert(0);
    }
    
    if(Header->SampleRate != 48000){
        Assert(0);
    }
    
    if(Header->BitsPerSample != 16){
        Assert(0);
    }
    
    if(!((Header->Data[0] == 'd') &&
         (Header->Data[1] == 'a') &&
         (Header->Data[2] == 't') &&
         (Header->Data[3] == 'a'))){
        Assert(0)
    }
    
    sound_data Result = {};
    
    Result.SampleCount = Header->DataSize/(Header->BitsPerSample/8);
    Result.Samples = PushArray(Arena, s16, Result.SampleCount);
    u8 *Data = ConsumeBytes(&Stream, Header->DataSize);
    Assert(Data);
    CopyMemory(Result.Samples, Data, Result.SampleCount);
    
    Result.ChannelCount = Header->ChannelCount;
    
    return Result;
}