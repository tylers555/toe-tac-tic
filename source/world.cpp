
//~ Helpers

internal inline world_entity_group *
AllocEntityGroup(world_data *World){
    world_entity_group *Result = ArrayAlloc(&World->EntityGroups);
    Result->Name = Strings.MakeBuffer();
    CopyCString(Result->Name, ALPHABET_ARRAY[World->EntityGroups.Count-1], DEFAULT_BUFFER_SIZE);
    return Result;
}

b8
world_manager::IsLevelCompleted(string LevelName){
    b8 Result = false;
    
    if(LevelName.ID){
        world_data *World = WorldManager.GetOrCreateWorld(LevelName);
        if(World){
            Result = World->Flags & WorldFlag_IsCompleted;
        }
    } 
    
    return(Result);
}


//~ World loading
internal void
AddPlayer(v2 P){
    EntityManager.PlayerInput = {};
    
    entity_player *Player = EntityManager.Player;
    *Player = {};
    asset_entity *EntityInfo = AssetSystem.GetEntity(Strings.GetString("player"));
    Player->EntityInfo = EntityInfo;
    Player->Animation.Direction = Direction_Left;
    
    Player->Z = 10.0f;
    
    Player->Health = 9;
    
    
    Player->P = P;
    Player->StartP = P;
    
    SetupDefaultEntity(Player, ENTITY_TYPE(entity_player), P, 
                       EntityInfo->Boundaries, EntityInfo->BoundaryCount, 
                       EntityInfo->Response);
}

internal void
AddParticles(entity_manager *Manager, v2 P){
    collision_boundary *Boundary = Manager->AllocBoundaries(1);
    *Boundary = MakeCollisionPoint();
    physics_particle_system *System = Manager->AddParticleSystem(P, Boundary, 100, 1.5f);
    System->StartdP = V2(0.0f, -3.0f);
}

void
world_manager::LoadWorld(const char *LevelName){
    TIMED_FUNCTION();
    
    if(LevelName){
        ArenaClear(&TransientMemory);
        EntityManager.Reset();
        
        string String = Strings.GetString(LevelName);
        world_data *World = GetOrCreateWorld(String);
        
        if(World){
            CurrentWorld = World;
            
            World->Manager.LoadTo(&EntityManager, &TransientMemory);
            
#if 0
            AddParticles(V2(3.0f, 3.0f));
            AddParticles(V2(5.0f, 3.0f));
            AddParticles(V2(7.0f, 3.0f));
            AddParticles(V2(9.0f, 3.0f));
#endif
            
        }
    }
    
}

//~ Loading from file

entity *
world_manager::LoadBasicEntity(world_data *World, entity_manager *Manager, stream *Stream){
    entity_array_type Type = *ConsumeType(Stream, entity_array_type);
    
    entity *Result = Manager->AllocBasicEntity(Type);
    
    ReadToVariable(Stream, Result->Flags);
    Result->TypeFlags = ENTITY_TYPE_TYPE_FLAGS[Result->Type];
    ReadToVariable(Stream, Result->Z);
    ReadToVariable(Stream, Result->Layer);
    
    ReadToVariable(Stream, Result->P);
    
    return Result;
}

world_data *
world_manager::LoadWorldFromFile(const char *Name){
    TIMED_FUNCTION();
    
    world_data *NewWorld = 0;
    char Path[512];
    stbsp_snprintf(Path, 512, "worlds/%s.sjw", Name);
    os_file *OSFile = OpenFile(Path, OpenFile_Read);
    CloseFile(OSFile);
    if(OSFile){
        entire_file File = ReadEntireFile(&TransientStorageArena, Path);
        string String = Strings.GetString(Name);
        NewWorld = CreateNewWorld(String);
        stream Stream = MakeReadStream(File.Data, File.Size);
        
        world_file_header *Header = ConsumeType(&Stream, world_file_header);
        if(!((Header->Header[0] == 'S') && 
             (Header->Header[1] == 'J') && 
             (Header->Header[2] == 'W'))){
            LogMessage("LoadWorldFromFile: Invalid header: %.3s!", Header->Header);
            RemoveWorld(String);
            return(0);
        }
        
        NewWorld->Width = Header->WidthInTiles;
        NewWorld->Height = Header->HeightInTiles;
        
        ArrayAlloc(&NewWorld->EntityGroups, Minimum(Header->EntityGroupCount, MAX_WORLD_ENTITY_GROUPS));
        u32 EntityCount = Header->EntityCount;
        
        NewWorld->AmbientColor = Header->AmbientColor;
        NewWorld->Exposure     = Header->Exposure;
        
        for(u32 I=1; I < NewWorld->EntityGroups.Count; I++){
            world_entity_group *Group = &NewWorld->EntityGroups[I];
            ReadStringAndMakeBuffer(&Stream, Group->Name);
            ReadToVariable(&Stream, Group->Z);
            ReadToVariable(&Stream, Group->Layer);
            ReadToVariable(&Stream, Group->Flags);
        }
        
        for(u32 I=0; I<Header->EntityCount; I++){
            
        }
        
#if 0
        for(u32 I=0; I < NewWorld->Entities.Count; I++){
            world_entity *Entity = &NewWorld->Entities[I];
            ReadToVariable(&Stream, Entity->Base.P);
            ReadToVariable(&Stream, Entity->Base.Flags);
            ReadToVariable(&Stream, Entity->Type);
            Entity->Base.Asset = Strings.GetString(ConsumeString(&Stream));
            
            u32 Index = *ConsumeType(&Stream, u32);
            Entity->Base.GroupIndex = Minimum(Index, NewWorld->EntityGroups.Count);
            
            switch(Entity->Type){
                case EntityType_Tilemap: {
                    ReadToVariable(&Stream, Entity->Tilemap.Width);
                    ReadToVariable(&Stream, Entity->Tilemap.Height);
                    ReadArrayToVariableAndDefaultAlloc(&Stream, Entity->Tilemap.Tiles, Entity->Tilemap.Width*Entity->Tilemap.Height);
                }break;
                case EntityType_Enemy: {
                    ReadToVariable(&Stream, Entity->Enemy.Direction);
                    ReadToVariable(&Stream, Entity->Enemy.PathStart);
                    ReadToVariable(&Stream, Entity->Enemy.PathEnd);
                }break;
                case EntityType_Player: {
                    ReadToVariable(&Stream, Entity->Player.Direction);
                }break;
                case EntityType_Art: {
                }break;
                case EntityType_Teleporter: {
                    ReadStringAndMakeBuffer(&Stream, Entity->Teleporter.Level);
                    ReadStringAndMakeBuffer(&Stream, Entity->Teleporter.RequiredLevel);
                }break;
                case EntityType_Door: {
                    ReadToVariable(&Stream, Entity->Door.Size);
                    ReadStringAndMakeBuffer(&Stream, Entity->Door.RequiredLevel);
                }break;
                default: {
                    RemoveWorld(String);
                    return(0);
                }break;
            }
        }
#endif
    }
    
    return(NewWorld);
}

//~ Writing to file

#define WriteVariableToFile(File, Offset, Number) { WriteToFile(File, *(Offset), &Number, sizeof(Number)); \
*(Offset) += sizeof(Number); }

internal inline void
WriteStringToFile(os_file *File, u32 *Offset, const char *S){
    u32 Length = CStringLength(S);
    WriteToFile(File, *Offset, S, Length+1);
    *Offset += Length+1;
}


internal inline void
WriteStringToFile(os_file *File, u32 *Offset, string S_){
    const char *S = Strings.GetString(S_);
    if(!S) S = "";
    u32 Length = CStringLength(S);
    WriteToFile(File, *Offset, S, Length+1);
    *Offset += Length+1;
}

internal inline void
WriteDataToFile(os_file *File, u32 *Offset, void *Data, u32 Size){
    WriteToFile(File, *Offset, Data, Size);
    *Offset += Size;
}

#define WriteArrayToFile(File, Offset, Array, Count) { WriteToFile(File, *(Offset), (Array), (Count)*sizeof(*(Array))); \
*(Offset) += (Count)*sizeof(*(Array)); }

void
world_manager::WriteBasicEntity(){
    
}

void
world_manager::WriteWorldsToFiles(){
    for(u32 I = 0; I < WorldTable.MaxBuckets; I++){
        if(WorldTable.Hashes[I] == 0) continue;
        
        world_data *World = &WorldTable.Values[I];
        
        char Path[512];
        stbsp_snprintf(Path, 512, "worlds/%s.sjw", World->Name);
        os_file *File = OpenFile(Path, OpenFile_Write);
        Assert(File);
        
        world_file_header Header = {};
        Header.Header[0] = 'S';
        Header.Header[1] = 'J';
        Header.Header[2] = 'W';
        Header.Version = CURRENT_WORLD_FILE_VERSION;
        Header.WidthInTiles = World->Width;
        Header.HeightInTiles = World->Height;
        Header.EntityGroupCount = World->EntityGroups.Count-1;
        //Header.EntityCount = World->Entities.Count;
        Header.AmbientColor  = World->AmbientColor;
        Header.Exposure      = World->Exposure;
        
        u32 Offset = 0;
        WriteVariableToFile(File, &Offset, Header);
        
        for(u32 I=1; I < World->EntityGroups.Count; I++){
            world_entity_group *Group = &World->EntityGroups[I];
            WriteStringToFile(File, &Offset, Group->Name);
            WriteVariableToFile(File, &Offset, Group->Z);
            WriteVariableToFile(File, &Offset, Group->Layer);
            world_entity_flags Flags = Group->Flags & ~WorldEntityEditFlags_All;
            WriteVariableToFile(File, &Offset, Flags);
        }
        
#if 0
        for(u32 I=0; I < World->Entities.Count; I++){
            world_entity *Entity = &World->Entities[I];
            WriteVariableToFile(File, &Offset, Entity->Base.P);
            WriteVariableToFile(File, &Offset, Entity->Base.Flags);
            WriteVariableToFile(File, &Offset, Entity->Type);
            WriteStringToFile(File, &Offset, Entity->Base.Asset);
            
            u32 Index = Entity->Base.GroupIndex;
            WriteVariableToFile(File, &Offset, Index);
            
            switch(Entity->Type){
                case EntityType_Tilemap: {
                    WriteVariableToFile(File, &Offset, Entity->Tilemap.Width);
                    WriteVariableToFile(File, &Offset, Entity->Tilemap.Height);
                    WriteArrayToFile(File, &Offset, Entity->Tilemap.Tiles, Entity->Tilemap.Width*Entity->Tilemap.Height);
                }break;
                case EntityType_Enemy: {
                    WriteVariableToFile(File, &Offset, Entity->Enemy.Direction);
                    WriteVariableToFile(File, &Offset, Entity->Enemy.PathStart);
                    WriteVariableToFile(File, &Offset, Entity->Enemy.PathEnd);
                }break;
                case EntityType_Art: {
                }break;
                case EntityType_Player: {
                    WriteVariableToFile(File, &Offset, Entity->Player.Direction);
                }break;
                case EntityType_Teleporter: {
                    WriteStringToFile(File, &Offset, Entity->Teleporter.Level);
                    WriteStringToFile(File, &Offset, Entity->Teleporter.RequiredLevel);
                }break;
                case EntityType_Door: {
                    WriteVariableToFile(File, &Offset, Entity->Door.Size);
                    WriteStringToFile(File, &Offset, Entity->Door.RequiredLevel);
                }break;
                default: INVALID_CODE_PATH; break;
            }
        }
#endif
        
        CloseFile(File);
    }
}

void
world_manager::Initialize(memory_arena *Arena){
    Memory          = MakeArena(Arena, Megabytes(100));
    TransientMemory = MakeArena(Arena, Kilobytes(512));
    WorldTable = PushHashTable<string, world_data>(Arena, 512);
}

world_data *
world_manager::GetWorld(string Name){
    world_data *Result = FindInHashTablePtr<string, world_data>(&WorldTable, Name);
    if(!Result){
        Result = LoadWorldFromFile(Strings.GetString(Name));
    }
    
    return(Result);
}

world_data *
world_manager::GetOrCreateWorld(string Name){
    world_data *Result = GetWorld(Name);
    if(!Result){
        Result = CreateNewWorld(Name);
        Result->Width = 32;
        Result->Height = 18;
        
#if 0
        world_entity *Entity = ArrayAlloc(&Result->Entities);
        *Entity = DefaultWorldEntity();
        Entity->Type = EntityType_Player;
        Entity->Player.P = V2(30.0f, 30.0f);
        Entity->Player.Asset = Strings.GetString("player");
        Entity->Player.Direction = Direction_Right;
#endif
    }
    return(Result);
}

world_data *
world_manager::CreateNewWorld(string Name){
    world_data *Result = FindInHashTablePtr(&WorldTable, Name);
    if(Result){
        Result = 0;
    }else{
        Result = CreateInHashTablePtr(&WorldTable, Name);
        Result->Name = Name;
        Result->EntityGroups = MakeArray<world_entity_group>(&Memory, MAX_WORLD_ENTITY_GROUPS);
        
        
        world_entity_group *Ungrouped = AllocEntityGroup(Result);
        Ungrouped->Name  = "Ungrouped";
        Ungrouped->Z     = 1.0f;
        Ungrouped->Layer = 1;
        
        Result->AmbientColor = HSBColor(1.0f, 0.0f, 1.0f);
        Result->Exposure = 1.0f;
        
        Result->Manager.Initialize(&Memory);
        
        Result->TeleporterBoundary = Result->Manager.AllocBoundaries(1);
        *Result->TeleporterBoundary = MakeCollisionRect(V2(0), TILE_SIZE);
        Result->TeleporterBoundary->Offset += V2(8);
        
        //~ Setup default player
        entity_player *Player = Result->Manager.Player;
        
        *Player = {};
        asset_entity *EntityInfo = AssetSystem.GetEntity(Strings.GetString("player"));
        Player->Type = ENTITY_TYPE(entity_player);
        Player->TypeFlags = ENTITY_TYPE_TYPE_FLAGS[ENTITY_TYPE(entity_player)];
        Player->Layer = ENTITY_TYPE_LAYER_FLAGS[ENTITY_TYPE(entity_player)];
        Player->EntityInfo = EntityInfo;
        Player->Animation.Direction = Direction_Left;
        
        Player->Z = 10.0f;
        
        Player->Health = 9;
        
        Player->StartP = V2(30.0f, 30.0f);
        
        SetupDefaultEntity(Player, ENTITY_TYPE(entity_player), V2(30.0f, 30.0f), 
                           EntityInfo->Boundaries, EntityInfo->BoundaryCount, 
                           EntityInfo->Response);
        //~ DEBUG
#if 1
        {
            entity_match *Entity = AllocEntity(&Result->Manager, entity_match);
            SetupTriggerEntity(Entity, ENTITY_TYPE(entity_match), V2(64, 16),
                               Result->TeleporterBoundary, 1,
                               MatchResponse);
        }
#endif
#if 1
        {
            string TilemapToAdd = Strings.GetString("grass_and_dirt");
            asset_tilemap *Asset = AssetSystem.GetTilemap(TilemapToAdd);
            entity_tilemap *Entity = AllocEntity(&Result->Manager, entity_tilemap);
            
            Entity->Z = 1.0f;
            u32 WidthU32  = 10;
            u32 HeightU32 = 10;
            f32 Width  = (f32)WidthU32 * Asset->TileSize.X;
            f32 Height = (f32)HeightU32 * Asset->TileSize.Y;
            
            SetupDefaultEntity(Entity, ENTITY_TYPE(entity_tilemap), 
                               V2(0), Asset->Boundaries, Asset->BoundaryCount);
            
            Entity->Bounds = SizeRect(V2(0), V2(Width, Height));
            Entity->Asset = TilemapToAdd;
            Entity->Width = WidthU32;
            Entity->Height = HeightU32;
            u32 MapSize = Entity->Width*Entity->Height*sizeof(*Entity->Tiles);
            Entity->Tiles = (tilemap_tile *)DefaultAlloc(MapSize);
            Entity->Tiles[0 + 0*WidthU32].Type = TileType_Tile;
            Entity->Tiles[1 + 0*WidthU32].Type = TileType_Tile;
            Entity->Tiles[2 + 0*WidthU32].Type = TileType_Tile;
            Entity->Tiles[3 + 0*WidthU32].Type = TileType_Tile;
            Entity->Tiles[4 + 0*WidthU32].Type = TileType_Tile;
            Entity->TileSize = Asset->TileSize;
            Entity->Layer = 1;
        }
#endif
        
    }
    
    return(Result);
}

void 
world_manager::RemoveWorld(string Name){
    char Buffer[DEFAULT_BUFFER_SIZE];
    stbsp_snprintf(Buffer, sizeof(Buffer), "worlds//%s.sjw", Strings.GetString(Name));
    DeleteFileAtPath(Buffer);
    RemoveFromHashTable(&WorldTable, Name);
}