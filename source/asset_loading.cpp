
//~ Initialization

internal inline u32 
MakeTileID(tile_type Type, tile_flags Flags, u16 ID){
    u32 Result = (((u32)Type << 24) | 
                  ((u32)Flags << 16) |
                  ID);
    return(Result);
}

void
asset_system::InitializeLoader(memory_arena *Arena){
    StateTable = PushHashTable<const char *, entity_state>(Arena, State_TOTAL);
    InsertIntoHashTable(&StateTable, "state_none",       State_None);
    InsertIntoHashTable(&StateTable, "state_idle",       State_Idle);
    InsertIntoHashTable(&StateTable, "state_moving",     State_Moving);
    InsertIntoHashTable(&StateTable, "state_jumping",    State_Jumping);
    InsertIntoHashTable(&StateTable, "state_falling",    State_Falling);
    InsertIntoHashTable(&StateTable, "state_turning",    State_Turning);
    InsertIntoHashTable(&StateTable, "state_retreating", State_Retreating);
    InsertIntoHashTable(&StateTable, "state_stunned",    State_Stunned);
    InsertIntoHashTable(&StateTable, "state_returning",  State_Returning);
    
    DirectionTable = PushHashTable<const char *, direction>(Arena, Direction_TOTAL+8);
    InsertIntoHashTable(&DirectionTable, "north",     Direction_North);
    InsertIntoHashTable(&DirectionTable, "northeast", Direction_NorthEast);
    InsertIntoHashTable(&DirectionTable, "east",      Direction_East);
    InsertIntoHashTable(&DirectionTable, "southeast", Direction_SouthEast);
    InsertIntoHashTable(&DirectionTable, "south",     Direction_South);
    InsertIntoHashTable(&DirectionTable, "southwest", Direction_SouthWest);
    InsertIntoHashTable(&DirectionTable, "west",      Direction_West);
    InsertIntoHashTable(&DirectionTable, "northwest", Direction_NorthWest);
    InsertIntoHashTable(&DirectionTable, "up",        Direction_Up);
    InsertIntoHashTable(&DirectionTable, "down",      Direction_Down);
    InsertIntoHashTable(&DirectionTable, "left",      Direction_Left);
    InsertIntoHashTable(&DirectionTable, "right",     Direction_Right);
    
    EntityTypeTable = PushHashTable<const char *, entity_array_type>(Arena, EntityArrayType_TOTAL);
    InsertIntoHashTable(&EntityTypeTable, "player", ENTITY_TYPE(entity_player));
    
    CollisionResponses = PushHashTable<const char *, collision_response_function *>(Arena, 3);
    InsertIntoHashTable(&CollisionResponses, "PLAYER",    PlayerCollisionResponse);
}

//~ Base

#define ExpectPositiveInteger() \
ExpectPositiveInteger_();   \
HandleError();

#define EnsurePositive(Var) \
if(Var < 0){            \
LogError("'%d' must be positive!", Var); \
return(false);      \
}

#define Expect(Name) \
ExpectToken(FileTokenType_##Name).Name; \
HandleError();

#define HandleError() \
if(Reader.LastError == FileReaderError_InvalidToken) return(Result) \

#define HandleToken(Token)                   \
if(Token.Type == FileTokenType_BeginCommand) break; \
if(Token.Type == FileTokenType_EndFile)      break; \
if(Token.Type == FileTokenType_Invalid)      break; \

void 
asset_system::BeginCommand(const char *Name){
    CurrentCommand = Name;
    CurrentAttribute = 0;
}

void
asset_system::LogError(const char *Format, ...){
    va_list VarArgs;
    va_start(VarArgs, Format);
    
    char Buffer[DEFAULT_BUFFER_SIZE];
    if(CurrentAttribute){
        stbsp_snprintf(Buffer, sizeof(Buffer), "(%s,%s Line: %u) %s", CurrentCommand, CurrentAttribute, Reader.Line, Format);
    }else{
        stbsp_snprintf(Buffer, sizeof(Buffer), "(%s Line: %u) %s", CurrentCommand, Reader.Line, Format);
    }
    VLogMessage(Buffer, VarArgs);
    
    va_end(VarArgs);
}

void
asset_system::LogInvalidAttribute(const char *Attribute){
    LogMessage("(%s, Line: %u) Invalid attribute: %s", CurrentCommand, Reader.Line, Attribute);
}

file_token
asset_system::ExpectToken(file_token_type Type){
    Reader.LastError = FileReaderError_None;
    file_token Token = Reader.NextToken();
    if(Type == FileTokenType_Float){
        Token = MaybeTokenIntegerToFloat(Token);
    }
    
    if(Token.Type == Type){
        return(Token);
    }else {
        LogError("Expected %s, instead read: %s", TokenTypeName(Type), TokenToString(Token));
    }
    
    Reader.LastError = FileReaderError_InvalidToken;
    return(Token);
}

u32
asset_system::ExpectPositiveInteger_(){
    u32 Result = 0;
    s32 Integer = Expect(Integer);
    if(Integer < 0){
        LogError("Expected a positive integer, instead read '%d', which is negative", Integer);
        return(0);
    }
    
    return(Integer);
}

collision_boundary
asset_system::ExpectTypeBoundary(){
    collision_boundary Result = {};
    
    const char *Identifier = Expect(Identifier);
    if(CompareStrings(Identifier, "Boundary_Rect")){
        ExpectToken(FileTokenType_BeginArguments);
        HandleError();
        
        f32 XOffset = (f32)Expect(Integer);
        f32 YOffset = (f32)Expect(Integer);
        f32 Width  = (f32)Expect(Integer);
        f32 Height = (f32)Expect(Integer);
        
        ExpectToken(FileTokenType_EndArguments);
        HandleError();
        
        Result = MakeCollisionRect(V2(XOffset, YOffset), V2(Width, Height));
    }else if(CompareStrings(Identifier, "Boundary_Circle")){
        ExpectToken(FileTokenType_BeginArguments);
        HandleError();
        
        f32 XOffset = (f32)Expect(Integer);
        f32 YOffset = (f32)Expect(Integer);
        f32 Radius  = (f32)Expect(Integer);
        
        ExpectToken(FileTokenType_EndArguments);
        HandleError();
        Result = MakeCollisionCircle(V2(XOffset, YOffset), Radius, 12, &Memory);
        
    }else if(CompareStrings(Identifier, "Boundary_Pill")){
        ExpectToken(FileTokenType_BeginArguments);
        HandleError();
        
        f32 XOffset = (f32)Expect(Integer);
        f32 YOffset = (f32)Expect(Integer);
        f32 Radius = (f32)Expect(Integer);
        f32 Height = (f32)Expect(Integer);
        
        ExpectToken(FileTokenType_EndArguments);
        HandleError();
        
        Result = MakeCollisionPill(V2(XOffset, YOffset), Radius, Height, 4, &Memory);
        
    }else if(CompareStrings(Identifier, "Boundary_Wedge")){
        ExpectToken(FileTokenType_BeginArguments);
        HandleError();
        
        f32 XOffset = (f32)Expect(Integer);
        f32 YOffset = (f32)Expect(Integer);
        f32 X   = (f32)Expect(Integer);
        f32 Y  = (f32)Expect(Integer);
        
        ExpectToken(FileTokenType_EndArguments);
        HandleError();
        
        v2 Offset = -0.5f*V2(X, Y) + V2(XOffset, YOffset);
        Result = MakeCollisionWedge(Offset, X, Y, &Memory);
        
    }else if(CompareStrings(Identifier, "Boundary_Quad")){
        ExpectToken(FileTokenType_BeginArguments);
        HandleError();
        
        f32 XOffset = (f32)Expect(Integer);
        f32 YOffset = (f32)Expect(Integer);
        f32 X0 = (f32)Expect(Integer);
        f32 Y0 = (f32)Expect(Integer);
        f32 X1 = (f32)Expect(Integer);
        f32 Y1 = (f32)Expect(Integer);
        f32 X2 = (f32)Expect(Integer);
        f32 Y2 = (f32)Expect(Integer);
        f32 X3 = (f32)Expect(Integer);
        f32 Y3 = (f32)Expect(Integer);
        
        ExpectToken(FileTokenType_EndArguments);
        HandleError();
        
        Result.Type = BoundaryType_FreeForm;
        Result.Offset = V2(XOffset, YOffset);
        f32 MinX = Minimum(Minimum(Minimum(X0, X1), Minimum(X2, X3)), 0.0f);
        f32 MinY = Minimum(Minimum(Minimum(Y0, Y1), Minimum(Y2, Y3)), 0.0f);
        Result.Bounds.Min = V2(MinX, MinY);
        
        f32 MaxX = Maximum(Maximum(Maximum(X0, X1), Maximum(X2, X3)), 0.0f);
        f32 MaxY = Maximum(Maximum(Maximum(Y0, Y1), Maximum(Y2, Y3)), 0.0f);
        Result.Bounds.Max = V2(MaxX, MaxY);
        
        Result.FreeFormPointCount = 4;
        Result.FreeFormPoints = PushArray(&Memory, v2, 4);
        Result.FreeFormPoints[0] = V2(X0, Y0);
        Result.FreeFormPoints[1] = V2(X1, Y1);
        Result.FreeFormPoints[2] = V2(X2, Y2);
        Result.FreeFormPoints[3] = V2(X3, Y3);
        
    }else{
        Reader.LastError = FileReaderError_InvalidToken;
        return(Result);
    }
    
    return(Result);
}

array<s32>
asset_system::ExpectTypeArrayS32(){
    array<s32> Result = MakeArray<s32>(&TransientStorageArena, SJA_MAX_ARRAY_ITEM_COUNT);
    
    const char *Identifier = Expect(Identifier);
    if(CompareStrings(Identifier, "Array")){
        ExpectToken(FileTokenType_BeginArguments);
        HandleError();
        
        file_token Token = Reader.PeekToken();
        while(Token.Type != FileTokenType_EndArguments){
            s32 Integer = Expect(Integer);
            ArrayAdd(&Result, Integer);
            
            Token = Reader.PeekToken();
        }
        
        ExpectToken(FileTokenType_EndArguments);
        HandleError();
        
    }else{
        Reader.LastError = FileReaderError_InvalidToken;
        return(Result);
    }
    
    return(Result);
}

asset_sprite_sheet_frame
asset_system::ExpectTypeSpriteSheetFrame(){
    asset_sprite_sheet_frame Result = {};
    
    const char *Identifier = Expect(Identifier);
    if(CompareStrings(Identifier, "Frame")){
        ExpectToken(FileTokenType_BeginArguments);
        HandleError();
        
        u32 Index = Expect(Integer);
        Result.Index = (u8)Index;
        
        ExpectToken(FileTokenType_EndArguments);
        HandleError();
    }else if(CompareStrings(Identifier, "Flip")){
        ExpectToken(FileTokenType_BeginArguments);
        HandleError();
        
        u32 Index = Expect(Integer);
        
        Result.Flags |= SpriteSheetFrameFlag_Flip;
        Result.Index = (u8)Index;
        
        ExpectToken(FileTokenType_EndArguments);
        HandleError();
    }else{
        Reader.LastError = FileReaderError_InvalidToken;
        return(Result);
    }
    
    return(Result);
}

array<asset_sprite_sheet_frame>
asset_system::ExpectTypeArraySpriteSheetFrame(){
    array<asset_sprite_sheet_frame> Result = MakeArray<asset_sprite_sheet_frame>(&TransientStorageArena, SJA_MAX_ARRAY_ITEM_COUNT);
    
    const char *Identifier = Expect(Identifier);
    if(CompareStrings(Identifier, "Array")){
        ExpectToken(FileTokenType_BeginArguments);
        HandleError();
        
        file_token Token = Reader.PeekToken();
        while(Token.Type != FileTokenType_EndArguments){
            asset_sprite_sheet_frame Frame = ExpectTypeSpriteSheetFrame();
            HandleError();
            ArrayAdd(&Result, Frame);
            
            Token = Reader.PeekToken();
        }
        
        ExpectToken(FileTokenType_EndArguments);
        HandleError();
        
    }else{
        Reader.LastError = FileReaderError_InvalidToken;
        return(Result);
    }
    
    return(Result);
}

//~ 

b8 
asset_system::DoAttribute(const char *String, const char *Attribute){
    b8 Result = CompareStrings(String, Attribute);
    if(Result) CurrentAttribute = Attribute;
    return(Result);
}

// TODO(Tyler): This should be made into an actual comment type such as /* */ or #if 0 #endif
b8
asset_system::ProcessIgnore(){
    while(true){
        file_token Token = Reader.PeekToken();
        HandleToken(Token);
        Reader.NextToken();
    }
    return(true);
}

void
asset_system::LoadAssetFile(const char *Path){
    TIMED_FUNCTION();
    
    CurrentCommand = 0;
    CurrentAttribute = 0;
    
    b8 HitError = false;
    do{
        ArenaClear(&Memory);
        memory_arena_marker Marker = ArenaBeginMarker(&TransientStorageArena);
        
        os_file *File = OpenFile(Path, OpenFile_Read);
        u64 NewFileWriteTime = GetLastFileWriteTime(File);
        CloseFile(File);
        
        if(LastFileWriteTime < NewFileWriteTime){
            HitError = false;
            
            Reader = MakeFileReader(Path);
            
            while(!HitError){
                file_token Token = Reader.NextToken();
                
                switch(Token.Type){
                    case FileTokenType_BeginCommand: {
                        if(!ProcessCommand()){
                            HitError = true;
                            break;
                        }
                    }break;
                    case FileTokenType_EndFile: {
                        goto end_loop;
                    }break;
                    default: {
                        LogMessage("(Line: %u) Token: %s was not expected!", Reader.Line, TokenToString(Token));
                        HitError = true;
                    }break;
                }
            }
            end_loop:;
        }
        
        ArenaEndMarker(&TransientStorageArena, &Marker);
        
        if(HitError) OSSleep(10); // To prevent consuming the CPU
        LastFileWriteTime = NewFileWriteTime;
    }while(HitError); 
    // This loop does result in a missed FPS but for right now it works just fine.
}

#define IfCommand(Command)                 \
if(CompareStrings(String, #Command)) { \
BeginCommand(#Command);            \
if(!Process ## Command()){ return(false); } \
return(true);                      \
}       

b8
asset_system::ProcessCommand(){
    b8 Result = false;
    
    const char *String = Expect(Identifier);
    
    IfCommand(Ignore);
    IfCommand(SpriteSheet);
    IfCommand(Animation);
    IfCommand(Entity);
    IfCommand(Art);
    IfCommand(Background);
    IfCommand(Tilemap);
    IfCommand(Font);
    IfCommand(SoundEffect);
    
    LogMessage("(Line: %u) '%s' isn't a valid command!", Reader.Line, String);
    return(false);
}
#undef IfCommand

//~ Sprite sheets

entity_state
asset_system::ReadState(){
    entity_state Result = State_None;
    
    const char *String = ExpectToken(FileTokenType_Identifier).Identifier;
    if(!String) return(Result);
    b8 Found = false;
    Result = FindInHashTable(&StateTable, String, &Found);
    if(!Found){
        LogError("Invalid state '%s'", String);
        return(Result);
    }
    
    return(Result);
}

b8 
asset_system::ProcessSpriteSheetStates(const char *StateName, asset_sprite_sheet *Sheet){
    CurrentAttribute = 0;
    b8 Result = false;
    
    b8 Found = false;
    entity_state State = FindInHashTable(&StateTable, StateName, &Found);
    if(!Found) return false;
    
    while(true){
        file_token Token = Reader.PeekToken();
        if(Token.Type != FileTokenType_Identifier){ break; }
        
        const char *DirectionName = Token.Identifier;
        direction Direction = FindInHashTable(&DirectionTable, DirectionName);
        if(Direction == Direction_None) break; 
        Reader.NextToken();
        
        s32 Index = ExpectPositiveInteger();
        Sheet->StateTable[State][Direction] = Index;
    }
    
    return(true);
}

b8 
asset_system::ProcessSpriteSheet(){
    b8 Result = false;
    
    const char *Name = Expect(Identifier);
    asset_sprite_sheet *Sheet = Strings.GetInHashTablePtr(&SpriteSheets, Name);
    *Sheet = {};
    
    v2s FrameSize = V2S(0);
    v2s ImageSizes[MAX_SPRITE_SHEET_PIECES] = {};
    b8  DonePieces[MAX_SPRITE_SHEET_PIECES] = {};
    asset_sprite_sheet_piece *CurrentPiece = 0;
    u32 CurrentPieceCurrentAnimationIndex  = 0;
    while(true){
        file_token Token = Reader.PeekToken();
        HandleToken(Token);
        const char *String = Expect(Identifier);
        
        if(DoAttribute(String, "piece")){
            s32 Index = Expect(Integer);
            
            if(Index > MAX_SPRITE_SHEET_PIECES){
                LogError("Piece index must be between 0 and %d", MAX_SPRITE_SHEET_PIECES);
                return(false);
            }
            
            if(DonePieces[Index]){
                LogError("Piece %d already exists!", Index);
                return(false);
            }
            
            const char *Path = Expect(String);
            image *Image = LoadImageFromPath(Path);
            
            if(!Image){
                LogError("'%s' isn't a valid path to an image!", Path);
                return(false);
            }
            
            Sheet->Pieces[Index].Texture = Image->Texture;
            ImageSizes[Index] = Image->Size;
            DonePieces[Index] = true;
            Sheet->PieceCount++;
            
            CurrentPieceCurrentAnimationIndex = 0;
            CurrentPiece = &Sheet->Pieces[Index];
            
        }else if(DoAttribute(String, "consecutive")){
            if(!CurrentPiece){
                LogError("The piece must be specified before defining sprite sheet animations");
                return(false);
            }
            
            if(CurrentPieceCurrentAnimationIndex > MAX_SPRITE_SHEET_ANIMATIONS){
                LogError("Too many animations have been specified, the max number is: %d", MAX_SPRITE_SHEET_ANIMATIONS);
                return(false);
            }
            
            asset_sprite_sheet_animation *Animation = &CurrentPiece->Animations[CurrentPieceCurrentAnimationIndex];
            
            asset_sprite_sheet_frame_flags FrameFlags = 0;
            
            file_token Token = Reader.PeekToken();
            if(Token.Type == FileTokenType_Identifier){
                const char *S = Token.String;
                if(CompareStrings(S, "FLIP")){
                    FrameFlags |= SpriteSheetFrameFlag_Flip;
                }else{
                    LogError("Invalid flag: '%s'", S);
                    return(false);
                }
                
                Expect(Identifier);
            }
            
            s32 StartingFrame   = ExpectPositiveInteger();
            s32 FrameCount      = ExpectPositiveInteger();
            s32 FPS             = ExpectPositiveInteger();
            s32 YOffset         = Expect(Integer);
            
            Animation->FrameCount = (u8)FrameCount;
            Animation->YOffset = (f32)YOffset;
            Animation->FPS = (f32)FPS;
            
            for(s32 I=0; I<Minimum(FrameCount, (s32)MAX_SPRITE_SHEET_ANIMATION_FRAMES); I++){
                Animation->Frames[I].Flags = FrameFlags;
                Animation->Frames[I].Index = (u8)(StartingFrame+I);
            }
            
            CurrentPieceCurrentAnimationIndex++;
            
        }else if(DoAttribute(String, "not_consecutive")){
            if(!CurrentPiece){
                LogError("The piece must be specified before defining sprite sheet animations");
                return(false);
            }
            
            if(CurrentPieceCurrentAnimationIndex > MAX_SPRITE_SHEET_ANIMATIONS){
                LogError("Too many animations have been specified, the max number is: %d", MAX_SPRITE_SHEET_ANIMATIONS);
                return(false);
            }
            
            asset_sprite_sheet_animation *Animation = &CurrentPiece->Animations[CurrentPieceCurrentAnimationIndex];
            
            array<asset_sprite_sheet_frame> Array = ExpectTypeArraySpriteSheetFrame();
            s32 FPS             = ExpectPositiveInteger();
            s32 YOffset         = Expect(Integer);
            
            Animation->FrameCount = (u8)Array.Count;
            Animation->YOffset = (f32)YOffset;
            Animation->FPS = (f32)FPS;
            
            for(u32 I=0; I<Minimum(Array.Count, MAX_SPRITE_SHEET_ANIMATION_FRAMES); I++){
                Animation->Frames[I] = Array[I];
            }
            
            CurrentPieceCurrentAnimationIndex++;
            
        }else if(DoAttribute(String, "y_offsets")){
            s32 Index = ExpectPositiveInteger();
            
            if(Index > MAX_SPRITE_SHEET_ANIMATIONS){
                LogError("Index %d is too big; the max number is: %d", Index, MAX_SPRITE_SHEET_ANIMATIONS);
                return(false);
            }
            
            s32 FPS = ExpectPositiveInteger();
            
            array<s32> YOffsets = ExpectTypeArrayS32();
            HandleError();
            
            if(YOffsets.Count > MAX_SPRITE_SHEET_ANIMATION_FRAMES){
                LogError("Too many Y offsets specified; the max number is: %d", 
                         YOffsets.Count, MAX_SPRITE_SHEET_ANIMATION_FRAMES);
                return(false);
            }
            
            Sheet->YOffsetCounts[Index] = (u8)YOffsets.Count;
            for(u32 I=0; I<YOffsets.Count; I++){
                Sheet->YOffsets[Index][I] = (f32)YOffsets[I];
            }
            
            Sheet->YOffsetFPS = (f32)FPS;
            
        }else if(DoAttribute(String, "size")){
            v2s Size = {};
            Size.X = Expect(Integer);
            Size.Y = Expect(Integer);
            FrameSize = Size;
            Sheet->FrameSize = V2(Size);
            
        }else{ 
            if(!ProcessSpriteSheetStates(String, Sheet)){
                LogInvalidAttribute(String); 
                return(false);
            }
        }
    }
    
    for(u32 I=0; I<Sheet->PieceCount; I++){
        v2s ImageSize = ImageSizes[I];
        
        Assert((FrameSize.X != 0) && (FrameSize.Y != 0));
        Assert((ImageSize.X != 0) && (ImageSize.Y != 0));
        Sheet->Pieces[I].XFrames = ImageSize.X/FrameSize.X;
        Sheet->Pieces[I].YFrames = ImageSize.Y/FrameSize.Y;
    }
    
    
    return(true);
}

//~ Animations
b8
asset_system::ProcessAnimation(){
    b8 Result = false;
    
    const char *Name = Expect(Identifier);
    asset_animation *Animation = Strings.GetInHashTablePtr(&Animations, Name);
    
    while(true){
        file_token Token = Reader.PeekToken();
        HandleToken(Token);
        const char *String = Expect(Identifier);
        
        if(DoAttribute(String, "on_finish")){
            
            entity_state From = ReadState();
            if(From == State_None) return(false);
            entity_state To = ReadState();
            if(To == State_None) return(false);
            
            Animation->ChangeDatas[From].Condition = ChangeCondition_AnimationOver;
            Animation->NextStates[From] = To;
        }else if(DoAttribute(String, "after_time")){
            animation_change_data ChangeData = {};
            
            file_token Token = Reader.NextToken();
            Token = MaybeTokenIntegerToFloat(Token);
            if(Token.Type == FileTokenType_String){
                u64 Hash = HashString(Token.String);
                ChangeData.Condition = SpecialChangeCondition_CooldownVariable;
                ChangeData.VarHash = Hash;
                
            }else if(Token.Type == FileTokenType_Float){
                ChangeData.Condition = ChangeCondition_CooldownOver;
                ChangeData.Cooldown = Token.Float;
                
            }else{
                LogError("Expected a string or a float, instead read: %s", TokenToString(Token));
                return(false);
            }
            
            entity_state From = ReadState();
            if(From == State_None){
                LogError("State cannot be: state_none");
                return(false);
            }
            entity_state To = ReadState();
            if(To == State_None){
                LogError("State cannot be: state_none");
                return(false);
            }
            
            Animation->ChangeDatas[From] = ChangeData;
            Animation->NextStates[From] = To;
            
        }else if(DoAttribute(String, "blocking")){
            entity_state State = ReadState();
            if(State == State_None) return(false);
            Animation->BlockingStates[State] = true;
            
        }else{ LogInvalidAttribute(String); return(false); }
    }
    
    
    return(true);
}

//~ Entities
inline b8
asset_system::IsInvalidEntityType(asset_entity *Entity, entity_array_type Target){
    b8 Result = false;
    if(Entity->Type != Target){
        if(Entity->Type == EntityArrayType_None){
            LogError("Entity type must be defined before!");
        }else{
            LogError("Entity type must be: %s", ENTITY_TYPE_NAME_TABLE[Target]);
        }
        Result = true;
    }
    return(Result);
}

b8
asset_system::ProcessEntity(){
    b8 Result = false;
    
    const char *Name = Expect(Identifier);
    asset_entity *Entity = Strings.GetInHashTablePtr(&Entities, Name);
    *Entity = {};
    
    collision_boundary Boundaries[MAX_ENTITY_ASSET_BOUNDARIES];
    u32 BoundaryCount = 0;
    b8 HasSetAnimation = false;
    while(true){
        file_token Token = Reader.PeekToken();
        if(Token.Type == FileTokenType_BeginCommand) break;
        const char *String = Expect(Identifier);
        
        if(DoAttribute(String, "type")){
            const char *TypeName = Expect(Identifier);
            entity_array_type Type = FindInHashTable(&EntityTypeTable, TypeName);
            if(Type == EntityArrayType_None){
                LogError("Invalid type name: '%s'!", TypeName);
                return(false);
            }
            
            Entity->Type = Type;
        }else if(DoAttribute(String, "piece")){
            const char *SheetName = Expect(Identifier);
            asset_sprite_sheet *Sheet = Strings.FindInHashTablePtr(&SpriteSheets, SheetName);
            if(!Sheet){
                LogError("The sprite sheet: '%s' is undefined!", SheetName);
                return(false);
            }
            
            Entity->SpriteSheet = Sheet;
            f32 ZOffset = (f32)Expect(Integer);
            ZOffset *= 0.1f;
            
            Entity->Size = Sheet->FrameSize;
            
        }else if(DoAttribute(String, "animation")){
            const char *AnimationName = Expect(Identifier);
            asset_animation *Animation = Strings.FindInHashTablePtr(&Animations, AnimationName);
            if(!Animation){
                LogError("The animation: '%s' is undefined!", AnimationName);
                return(false);
            }
            
            Entity->Animation = *Animation;
            HasSetAnimation = true;
            
        }else if(DoAttribute(String, "animation_var")){
            if(!HasSetAnimation){
                LogError("Animation must be specified before 'animation_var' is used!");
                return(false);
            }
            //if(IsInvalidEntityType(Entity, ENTITY_TYPE(entity_enemy))) return(false);
            
            const char *VarName = Expect(String);
            u64 VarHash = HashString(VarName);
            
            f32 Time = Expect(Float);
            
            b8 FoundVar = false;
            for(u32 I=0; I<State_TOTAL; I++){
                animation_change_data *Data = &Entity->Animation.ChangeDatas[I];
                if((Data->Condition == SpecialChangeCondition_CooldownVariable) &&
                   (Data->VarHash == VarHash)){
                    Data->Condition = ChangeCondition_CooldownOver;
                    Data->Cooldown = Time;
                    FoundVar = true;
                }
            }
            
            if(!FoundVar){
                LogError("Couldn't find animation variable: '%s'!", VarName);
            }
            
        }else if(DoAttribute(String, "mass")){
            Entity->Mass = Expect(Float);
            
        }else if(DoAttribute(String, "speed")){
            Entity->Speed = Expect(Float);
            
        }else if(DoAttribute(String, "boundary")){
            u32 Index = ExpectPositiveInteger();
            if(Index > MAX_ENTITY_ASSET_BOUNDARIES){
                LogError("'%d' must be less than %d!", Index, MAX_ENTITY_ASSET_BOUNDARIES);
                return(false);
            }
            
            if(BoundaryCount < (u32)Index+1){ BoundaryCount = Index+1; }
            Boundaries[Index] = ExpectTypeBoundary();
            HandleError();
        }else if(DoAttribute(String, "collision_response")){
            const char *ResponseName = Expect(Identifier);
            
            collision_response_function *Response = FindInHashTable(&CollisionResponses, ResponseName);
            if(!Response){
                LogError("Invalid response function: '%s'!", ResponseName);
                return(false);
            }
            Entity->Response = Response;
            
        }else if(DoAttribute(String, "CAN_BE_STUNNED")){
            Entity->Flags |= EntityFlag_CanBeStunned;
        }else if(DoAttribute(String, "NO_GRAVITY")){
            Entity->Flags |= EntityFlag_NotAffectedByGravity;
        }else{ LogInvalidAttribute(String); return(false); }
    }
    
    if(!Entity->SpriteSheet){
        LogError("Sprite sheet must be set!");
        return(false);
    }
    
    Entity->Boundaries = PushArray(&Memory, collision_boundary, BoundaryCount);
    Entity->BoundaryCount = BoundaryCount;
    for(u32 I=0; I<BoundaryCount; I++){
        collision_boundary *Boundary = &Boundaries[I];
        v2 Size = RectSize(Boundary->Bounds);
        v2 Min   = Boundary->Bounds.Min;
        Boundary->Offset.Y -= Min.Y;
        Boundary->Offset.X += 0.5f*(Entity->Size.Width);
        Entity->Boundaries[I] = *Boundary;
    }
    
    
    
    return(true);
}

//~ Arts
b8 
asset_system::ProcessArt(){
    b8 Result = false;
    
    const char *Name = Expect(Identifier);
    asset_art *Art = Strings.GetInHashTablePtr(&Arts, Name);
    *Art = {};
    
    while(true){
        file_token Token = Reader.PeekToken();
        HandleToken(Token);
        const char *String = Expect(Identifier);
        
        if(DoAttribute(String, "path")){
            const char *Path = Expect(String);
            
            image *Image = LoadImageFromPath(Path);
            if(!Image){
                LogError("'%s' isn't a valid path to an image!", Path);
                return(false);
            }
            Art->Size = V2(Image->Size);
            Art->Texture = Image->Texture;
        }else{ LogInvalidAttribute(String); return(false); }
    }
    
    return(true);
}

//~ Backgrounds
b8
asset_system::ProcessBackground(){
    b8 Result = false;
    
    const char *Name = Expect(Identifier);
    asset_art *Art = Strings.GetInHashTablePtr(&Backgrounds, Name);
    *Art = {};
    
    while(true){
        file_token Token = Reader.PeekToken();
        HandleToken(Token);
        const char *String = Expect(Identifier);
        
        if(DoAttribute(String, "path")){
            const char *Path = Expect(String);
            
            image *Image = LoadImageFromPath(Path);
            if(!Image){
                LogError("'%s' isn't a valid path to an image!", Path);
                return(false);
            }
            Art->Size = V2(Image->Size);
            Art->Texture = Image->Texture;
        }else{ LogInvalidAttribute(String); return(false); }
    }
    
    return(true);
}

//~ Tilemaps

#define AssetLoaderProcessTilemapTransform(Array, Name_, Transform_) \
if(CompareStrings(S, Name_)){                             \
if((Array)->Count > 1){                               \
asset_tilemap_tile_data *PreviousTile = &(*(Array))[(Array)->Count-2]; \
Tile->OffsetMin = PreviousTile->OffsetMin;        \
Tile->OffsetMax = PreviousTile->OffsetMax;        \
Tile->Transform = Transform_;                     \
break;                                            \
}else{                                                \
LogError("'%s' can not apply to the first tile!", Name_); \
return(false);                                    \
}                                                     \
} 

b8
asset_system::ProcessTilemapTile(tile_array *Tiles, const char *TileType, u32 *TileOffset){
    b8 Result = false;
    CurrentAttribute = 0;
    
    asset_tilemap_tile_data *Tile = ArrayAlloc(Tiles);
    if(CompareStrings(TileType, "art")){
        Tile->Flags |= TileFlag_Art;
        TileType = Expect(Identifier);
    }
    
    if(CompareStrings(TileType, "tile")){
        Tile->Type |= TileType_Tile;
    }else if(CompareStrings(TileType, "wedge_up_left")){
        Tile->Type |= TileType_WedgeUpLeft;
    }else if(CompareStrings(TileType, "wedge_up_right")){
        Tile->Type |= TileType_WedgeUpRight;
    }else if(CompareStrings(TileType, "wedge_down_left")){
        Tile->Type |= TileType_WedgeDownLeft;
    }else if(CompareStrings(TileType, "wedge_down_right")){
        Tile->Type |= TileType_WedgeDownRight;
    }else if(CompareStrings(TileType, "connector")){
        Tile->Type |= TileType_Connector;
    }else{
        LogError("'%s' is not a valid tile type!", TileType);
        return(false);
    }
    
    if(!(Tile->Type & TileType_Connector)){
        u32 BoundaryIndex = ExpectPositiveInteger();
        Tile->BoundaryIndex = (u8)BoundaryIndex;
    }
    
    CurrentAttribute = TileType;
    
    const char *PlaceString = Expect(String);
    tilemap_tile_place Place = StringToTilePlace(PlaceString);
    if(Place == 0){
        LogError("'%s' is not a valid tile place pattern", PlaceString);
        return(false);
    }
    Tile->Place = Place;
    
    Tile->ID = MakeTileID(Tile->Type, Tile->Flags, Tile->Place);
    file_token Token = Reader.PeekToken();
    if(Token.Type == FileTokenType_Identifier){
        const char *S = Expect(Identifier);
        
        // NOTE(Tyler): This do while is here for macro reasons
        do{
            AssetLoaderProcessTilemapTransform(Tiles, "COPY_PREVIOUS",       TileTransform_None);
            AssetLoaderProcessTilemapTransform(Tiles, "REVERSE_PREVIOUS",    TileTransform_HorizontalReverse);
            AssetLoaderProcessTilemapTransform(Tiles, "V_REVERSE_PREVIOUS",  TileTransform_VerticalReverse);
            AssetLoaderProcessTilemapTransform(Tiles, "HV_REVERSE_PREVIOUS", TileTransform_HorizontalAndVerticalReverse);
            AssetLoaderProcessTilemapTransform(Tiles, "ROTATE_PREVIOUS_90",  TileTransform_Rotate90);
            AssetLoaderProcessTilemapTransform(Tiles, "ROTATE_PREVIOUS_180", TileTransform_Rotate180);
            AssetLoaderProcessTilemapTransform(Tiles, "ROTATE_PREVIOUS_270", TileTransform_Rotate270);
            AssetLoaderProcessTilemapTransform(Tiles, "REVERSE_AND_ROTATE_PREVIOUS_90",  TileTransform_ReverseAndRotate90);
            AssetLoaderProcessTilemapTransform(Tiles, "REVERSE_AND_ROTATE_PREVIOUS_180", TileTransform_ReverseAndRotate180);
            AssetLoaderProcessTilemapTransform(Tiles, "REVERSE_AND_ROTATE_PREVIOUS_270", TileTransform_ReverseAndRotate270);
            
            LogError("'%s' is not a valid transformation", S);
            return(false);
        }while(false);
        
    }else{
        u32 Count = Expect(Integer);
        
        Tile->OffsetMin = *TileOffset;
        *TileOffset += Count;
        Tile->OffsetMax = *TileOffset;
    }
    
    return(true);
}

b8
asset_system::ProcessTilemap(){
    b8 Result = false;
    
    const char *Name = Expect(Identifier);
    asset_tilemap *Tilemap = Strings.GetInHashTablePtr(&Tilemaps, Name);
    *Tilemap = {};
    
    tile_array Tiles;
    InitializeArray(&Tiles, 32, &TransientStorageArena);
    
    collision_boundary Boundaries[MAX_TILEMAP_BOUNDARIES];
    u32 BoundaryCount = 0;
    u32 TileOffset = 0;
    u32 TileCount = 0;
    image *Image = 0;
    while(true){
        file_token Token = Reader.PeekToken();
        HandleToken(Token);
        const char *String = Expect(Identifier);
        
        if(DoAttribute(String, "path")){
            const char *Path = Expect(String);
            
            Image = LoadImageFromPath(Path);
            if(!Image){
                LogError("'%s' isn't a valid path to an image!", Path);
                return(false);
            }
            Tilemap->Texture = Image->Texture;
        }else if(DoAttribute(String, "tile_size")){
            s32 XSize = ExpectPositiveInteger();
            s32 YSize = ExpectPositiveInteger();
            
            Tilemap->TileSize = V2((f32)XSize, (f32)YSize);
        }else if(DoAttribute(String, "dimensions")){
            s32 XTiles = ExpectPositiveInteger();
            s32 YTiles = ExpectPositiveInteger();
            
            if(XTiles == 0){
                LogError("Tilemap dimensions(X) cannot be 0!");
                return(false);
            }
            
            if(YTiles == 0){
                LogError("Tilemap dimensions(Y) cannot be 0!");
                return(false);
            }
            
            Tilemap->XTiles = XTiles;
            Tilemap->YTiles = YTiles;
        }else if(DoAttribute(String, "skip_tiles")){
            s32 Count = ExpectPositiveInteger();
            TileOffset += Count;
        }else if(DoAttribute(String, "boundary")){
            s32 Index = ExpectPositiveInteger();
            if(Index > MAX_TILEMAP_BOUNDARIES){
                LogError("'%d' must be less than %d!", Index, MAX_TILEMAP_BOUNDARIES);
                return(false);
            }
            
            if(BoundaryCount < (u32)Index+1){ BoundaryCount = Index+1; }
            Boundaries[Index] = ExpectTypeBoundary();
            HandleError();
        }else if(DoAttribute(String, "manual_tile")){
            asset_tilemap_tile_data *Tile = ArrayAlloc(&Tiles);
            Tile->Type  |= TileType_Tile;
            Tile->Flags |= TileFlag_Manual;
            
            {
                file_token Token = Reader.NextToken();
                if(Token.Type == FileTokenType_Identifier){
                    if(CompareStrings(Token.Identifier, "art")){
                        Tile->Flags |= TileFlag_Art;
                    }
                }else if(Token.Type == FileTokenType_Integer){
                    EnsurePositive(Token.Integer);
                    Tile->BoundaryIndex = (u8)Token.Integer;
                }else{
                    LogError("Expected an identifier or an integer, instead read: %s", TokenToString(Token));
                    return(false);
                }
            }
            
            u32 ID = Expect(Integer);
            Tile->ID = MakeTileID(Tile->Type, Tile->Flags, (u16)ID);
            Tile->Place = 0xffff;
            
            file_token Token = Reader.PeekToken();
            if(Token.Type == FileTokenType_Identifier){
                const char *S = Expect(Identifier);
                
                // NOTE(Tyler): This do while is here for macro reasons
                do{
                    AssetLoaderProcessTilemapTransform(&Tiles, "COPY_PREVIOUS",       TileTransform_None);
                    AssetLoaderProcessTilemapTransform(&Tiles, "REVERSE_PREVIOUS",    TileTransform_HorizontalReverse);
                    AssetLoaderProcessTilemapTransform(&Tiles, "V_REVERSE_PREVIOUS",  TileTransform_VerticalReverse);
                    AssetLoaderProcessTilemapTransform(&Tiles, "HV_REVERSE_PREVIOUS", TileTransform_HorizontalAndVerticalReverse);
                    AssetLoaderProcessTilemapTransform(&Tiles, "ROTATE_PREVIOUS_90",  TileTransform_Rotate90);
                    AssetLoaderProcessTilemapTransform(&Tiles, "ROTATE_PREVIOUS_180", TileTransform_Rotate180);
                    AssetLoaderProcessTilemapTransform(&Tiles, "ROTATE_PREVIOUS_270", TileTransform_Rotate270);
                    AssetLoaderProcessTilemapTransform(&Tiles, "REVERSE_AND_ROTATE_PREVIOUS_90",  TileTransform_ReverseAndRotate90);
                    AssetLoaderProcessTilemapTransform(&Tiles, "REVERSE_AND_ROTATE_PREVIOUS_180", TileTransform_ReverseAndRotate180);
                    AssetLoaderProcessTilemapTransform(&Tiles, "REVERSE_AND_ROTATE_PREVIOUS_270", TileTransform_ReverseAndRotate270);
                    
                    LogError("'%s' is not a valid string", S);
                    return(false);
                }while(false);
                
            }else{
                u32 Count = Expect(Integer);
                
                Tile->OffsetMin = TileOffset;
                TileOffset += Count;
                Tile->OffsetMax = TileOffset;
            }
            
            
        }else{
            if(!ProcessTilemapTile(&Tiles, String, &TileOffset)) return(false);
            
            if(TileOffset > (Tilemap->XTiles*Tilemap->YTiles)){
                LogError("The number of tiles(%u) adds up to more than than the dimensions of the tilemap would allow(%u)!",
                         TileOffset, Tilemap->XTiles*Tilemap->YTiles);
                return(false);
            }
        }
    }
    
    if((Tilemap->XTiles == 0) || (Tilemap->YTiles == 0)){
        LogError("Tilemap dimensions must be specified!");
        return(false);
    }
    
    if(!Image){
        LogError("An image was not specified!");
        return(false);
    }
    
    {
        s32 XSize = Image->Size.X/Tilemap->XTiles;
        s32 YSize = Image->Size.Y/Tilemap->YTiles;
        Tilemap->CellSize = V2((f32)XSize, (f32)YSize);
    }
    
    
    tilemap_tile_place CombinedPlacesTiles = 0;
    
    asset_tilemap_tile_data *UnsortedTiles = PushArray(&TransientStorageArena, asset_tilemap_tile_data, Tiles.Count);
    Tilemap->Connectors = PushArray(&Memory, asset_tilemap_tile_data, 8);
    for(u32 I=0; I<Tiles.Count; I++){
        if(Tiles[I].BoundaryIndex > BoundaryCount){
            LogError("Tile's boundary index cannot be greater than the number of boundaries specified(%u)",
                     BoundaryCount);
            return(false);
        }
        
        if(Tiles[I].Type == TileType_Connector){
            Tilemap->Connectors[Tilemap->ConnectorCount++] = Tiles[I];
        }else{
            UnsortedTiles[Tilemap->TileCount++]  = Tiles[I];
            if(Tiles[I].Type & TileType_Tile)  CombinedPlacesTiles  |= Tiles[I].Place;
        }
    }
    
    // TODO(Tyler): This is not a good sorting algorithm, this should be changed!
    Tilemap->Tiles = PushArray(&Memory, asset_tilemap_tile_data, Tilemap->TileCount);
    u32 *Popcounts = PushArray(&TransientStorageArena, u32, Tilemap->TileCount);
    for(u32 I=0; I<Tilemap->TileCount; I++){
        asset_tilemap_tile_data *Tile = &UnsortedTiles[I];
        u32 Popcount = PopcountU32(Tile->Place);
        u32 J = 0;
        for(; J<I; J++){
            if(Popcount < Popcounts[J]){
                MoveMemory(&Popcounts[J+1], &Popcounts[J], (I-J)*sizeof(*Popcounts));
                MoveMemory(&Tilemap->Tiles[J+1], &Tilemap->Tiles[J], (I-J)*sizeof(*Tilemap->Tiles));
                break;
            }
        }
        Popcounts[J] = Popcount;
        Tilemap->Tiles[J] = *Tile;
    }
    
    
    Tilemap->Boundaries = PushArray(&Memory, collision_boundary, BoundaryCount);
    Tilemap->BoundaryCount = BoundaryCount;
    
    for(u32 I=0; I<BoundaryCount; I++){
        collision_boundary *Boundary = &Boundaries[I];
        Tilemap->Boundaries[I] = *Boundary;
    }
    
    return(true);
}

//~ Fonts

b8
asset_system::ProcessFont(){
    return(ProcessIgnore());
}

//~ Sound effects

b8
asset_system::ProcessSoundEffect(){
    b8 Result = false;
    
    const char *Name = Expect(Identifier);
    asset_sound_effect *Sound = Strings.GetInHashTablePtr(&SoundEffects, Name);
    *Sound = {};
    
    while(true){
        file_token Token = Reader.PeekToken();
        HandleToken(Token);
        const char *String = Expect(Identifier);
        
        if(DoAttribute(String, "path")){
            const char *Path = Expect(String);
            sound_data Data = LoadWavFile(&OSSoundBuffer, &Memory, Path);
            if(!Data.Samples){
                LogError("'%s' isn't a valid path to a wav file!", Path);
            }
            Sound->Sound = Data;
            
        }else{ LogInvalidAttribute(String); return(false); }
    }
    
    
    return true;
}
