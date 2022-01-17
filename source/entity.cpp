
//~ Entity allocation and management
void
entity_manager::Reset(){
    Memory.Used = 0;
    
#define ENTITY_TYPE_(TypeName, ...) InitializeBucketArray(&EntityArray_##TypeName, &Memory);
    Player = PushStruct(&Memory, entity_player);
    ENTITY_TYPES;
    SPECIAL_ENTITY_TYPES;
#undef ENTITY_TYPE_
    EntityCount = 0;
    
    BucketArrayRemoveAll(&ParticleSystems);
    ArenaClear(&ParticleMemory);
    ArenaClear(&BoundaryMemory);
    
    PhysicsDebugger.Paused = {};
    PhysicsDebugger.StartOfPhysicsFrame = true;
}

void
entity_manager::Initialize(memory_arena *Arena){
    *this = {};
    Memory = MakeArena(Arena, Megabytes(10));
    
#define ENTITY_TYPE_(TypeName, ...) InitializeBucketArray(&EntityArray_##TypeName, &Memory);
    Player = PushStruct(&Memory, entity_player);
    ENTITY_TYPES;
    SPECIAL_ENTITY_TYPES;
#undef ENTITY_TYPE_
    
    //~ Physics
    InitializeBucketArray(&ParticleSystems, Arena);
    ParticleMemory          = MakeArena(Arena, 64*128*sizeof(physics_particle_x4));
    BoundaryMemory          = MakeArena(Arena, 3*128*sizeof(collision_boundary));
    PermanentBoundaryMemory = MakeArena(Arena, 128*sizeof(collision_boundary));
}

//~ Iteration
internal inline entity_iterator
EntityManagerBeginIteration(entity_manager *Manager){
    entity_iterator Result = {};
    
    Result.CurrentArray = ENTITY_TYPE(entity_player);
    Result.Item = Manager->Player;
    Result.Index = {};
    
    return Result;
}

internal inline b8
EntityManagerContinueIteration(entity_manager *Manager, entity_iterator *Iterator){
    if(!(Iterator->I < (Manager->EntityCount+1))){
        return false;
    }
    
#define ENTITY_TYPE_(TypeName, TypeFlags, ...) else if(ENTITY_TYPE(TypeName) == Iterator->CurrentArray) { \
bucket_array_iterator<TypeName> BucketIterator = {}; \
BucketIterator.Index = Iterator->Index; \
Assert(BucketArrayContinueIteration(&Manager->EntityArray_##TypeName, &BucketIterator)); \
Iterator->Item = BucketIterator.Item;   \
}
    
    if(ENTITY_TYPE(entity_player) == Iterator->CurrentArray) {
    }
    ENTITY_TYPES
        
#undef ENTITY_TYPE_
    
    return true;
}

internal inline void
EntityManagerNextIteration(entity_manager *Manager, entity_iterator *Iterator){
    b8 Found = false;
#define ENTITY_TYPE_(TypeName, TypeFlags, ...) else if(ENTITY_TYPE(TypeName) == Iterator->CurrentArray) { \
bucket_array_iterator<TypeName> BucketIterator = {}; \
BucketIterator.Index = Iterator->Index; \
Found = BucketArrayNextIteration(&Manager->EntityArray_##TypeName, &BucketIterator); \
Iterator->Index = BucketIterator.Index; \
}
    
    if(ENTITY_TYPE(entity_player) == Iterator->CurrentArray) {
        Found = false;
    }
    ENTITY_TYPES
        
#undef ENTITY_TYPE_
#define ENTITY_TYPE_(TypeName, TypeFlags, ...) else if((ENTITY_TYPE(TypeName) > Iterator->CurrentArray) && \
(Manager->EntityArray_##TypeName.Count > 0)){ \
auto BucketIterator = BucketArrayBeginIteration(&Manager->EntityArray_##TypeName); \
Iterator->CurrentArray = EntityArrayType_##TypeName; \
Iterator->Item  = BucketIterator.Item;               \
Iterator->Index = BucketIterator.Index;              \
} 
    
    if(!Found){
        if((ENTITY_TYPE(entity_player) > Iterator->CurrentArray)){ 
            Iterator->CurrentArray = ENTITY_TYPE(entity_player);
            Iterator->Item  = Manager->Player;
            Iterator->Index = {};
        } 
        ENTITY_TYPES;
    }
    
#undef ENTITY_TYPE_
    
    Iterator->I++;
}



//~ Animation stuff
internal void
ChangeEntityState(entity *Entity, asset_entity *EntityInfo, entity_state NewState){
    if(Entity->Animation.State != NewState){
        ChangeAnimationState(&EntityInfo->Animation, &Entity->Animation, NewState);
    }
}

//~ Helpers
inline void
entity_manager::DamagePlayer(u32 Damage){
    Player->P = Player->StartP;
    Player->dP = V2(0);
    EntityManager.Player->Health -= Damage;
    if(EntityManager.Player->Health <= 0){
        EntityManager.Player->Health = 9;
        Score = 0;
        
        asset_sound_effect *Asset = AssetSystem.GetSoundEffect(String("death"));
        AudioMixer.PlaySound(Asset);
    }
    asset_sound_effect *Asset = AssetSystem.GetSoundEffect(String("damage"));
    AudioMixer.PlaySound(Asset);
}

internal inline void
OpenDoor(entity_door *Door){
    if(!Door->IsOpen){ Door->Cooldown = 1.0f; }
    Door->IsOpen = true;
    Door->PhysicsFlags |= PhysicsStateFlag_Inactive;
}

//~ 

internal inline void
SetupDefaultEntity(entity *Entity, entity_array_type Type, v2 P, collision_boundary *Boundaries, u32 BoundaryCount,
                   collision_response_function *Response=CollisionResponseStub){
    Entity->P = P;
    Entity->Type = Type;
    Entity->Boundaries = Boundaries;
    Entity->BoundaryCount = (u8)BoundaryCount;
    Entity->Response = Response;
    Entity->Bounds = GetBoundsOfBoundaries(Entity->Boundaries, Entity->BoundaryCount);
    Entity->TypeFlags = ENTITY_TYPE_TYPE_FLAGS[Type];
    Entity->PhysicsLayer = ENTITY_TYPE_LAYER_FLAGS[Type];
}

internal inline void
SetupTriggerEntity(entity *Entity, entity_array_type Type, v2 P, collision_boundary *Boundaries, u32 BoundaryCount,
                   trigger_response_function *Response=TriggerResponseStub){
    Entity->P = P;
    Entity->Boundaries = Boundaries;
    Entity->BoundaryCount = (u8)BoundaryCount;
    Entity->TriggerResponse = Response;
    Entity->Bounds = GetBoundsOfBoundaries(Entity->Boundaries, Entity->BoundaryCount);
    Entity->TypeFlags = ENTITY_TYPE_TYPE_FLAGS[Type];
    Entity->PhysicsLayer = ENTITY_TYPE_LAYER_FLAGS[Type];
}

//~ 
entity *
entity_manager::AllocBasicEntity(entity_array_type Type){
#define ENTITY_TYPE_(TypeName, ...) \
case ENTITY_TYPE(TypeName): {   \
Result = BucketArrayAlloc(&ENTITY_ARRAY(TypeName)); \
EntityCount++;    \
}break;
    
    entity *Result = 0;
    
    switch(Type){
        ENTITY_TYPES;
        default: INVALID_CODE_PATH; break;
    }
    
    
    Result->Type = Type;
    
    return Result;
#undef ENTITY_TYPE_
}

void
entity_manager::RemoveEntityByIndex(entity_array_type Type, bucket_index Index){
    switch(Type){
        case ENTITY_TYPE(entity_tilemap): {
            entity_tilemap *Entity = BucketArrayGet(&ENTITY_ARRAY(entity_tilemap), Index);
            DefaultFree(Entity->Tiles);
        }break;
        case ENTITY_TYPE(entity_teleporter): {
            entity_teleporter *Entity = BucketArrayGet(&ENTITY_ARRAY(entity_teleporter), Index);
            Strings.RemoveBuffer(Entity->Level);
            Strings.RemoveBuffer(Entity->RequiredLevel);
        }break;
        case ENTITY_TYPE(entity_door): {
            entity_door *Entity = BucketArrayGet(&ENTITY_ARRAY(entity_door), Index);
            Strings.RemoveBuffer(Entity->RequiredLevel);
        }break;
    }
    
    switch(Type){
#define ENTITY_TYPE_(TypeName, ...)                       \
case ENTITY_TYPE(TypeName): {                         \
BucketArrayRemove(&ENTITY_ARRAY(TypeName), Index); \
EntityCount--;                                    \
}break;
        ENTITY_TYPES;
#undef ENTITY_TYPE_
        default: { INVALID_CODE_PATH; }break;
    }
}

//~ Collisions responses

internal void
TeleporterResponse(entity *Data, entity *EntityB){
    Assert(Data);
    entity_teleporter *Teleporter = (entity_teleporter *)Data;
    Assert(Teleporter->Type == ENTITY_TYPE(entity_teleporter));
    if(EntityB->Type != ENTITY_TYPE(entity_player)) return;
    
    Teleporter->IsSelected = true;
    
    return;
}

internal void
MatchResponse(entity *Data, entity *EntityB){
    Assert(Data);
    entity_match *Teleporter = (entity_match *)Data;
    Assert(Teleporter->Type == ENTITY_TYPE(entity_match));
    if(EntityB->Type != ENTITY_TYPE(entity_player)) return;
    
    Teleporter->IsSelected = true;
    
    return;
}

internal b8
PlayerCollisionResponse(physics_update *Update, physics_collision *Collision){
    Assert(Update);
    entity_player *Player = (entity_player *)Update->Entity;
    Assert(Player->Type == ENTITY_TYPE(entity_player));
    b8 Result = false;
    
    entity *CollisionEntity = Collision->EntityB;
    if(CollisionEntity == Player) return false;
    
    if((Collision->Normal.Y > 0.9f) &&
       (Player->dP.Y < -55.0f)){
        asset_sound_effect *Asset = AssetSystem.GetSoundEffect(String("entity_land"));
        AudioMixer.PlaySound(Asset);
    }
    
    return Result;
}

//~

internal inline void
MoveTopDown(physics_update_context *Context, entity *Entity, v2 Movement){
    physics_update *Update = MakeUpdate(Context, Entity, Entity->Layer);
    
    Entity->dP += 1.0f*(Entity->TargetdP-Entity->dP);
    
    Update->Delta = (OSInput.dTime*Entity->dP);
    
    Entity->TargetdP = {};
    Entity->TargetdP += Movement;
    
    Entity->Update = Update;
    
    if(Entity->Parent){
        ArrayAdd(&Context->ChildUpdates, Update);
    }
}

internal inline void
MovePlatformer(physics_update_context *Context, entity *Entity, f32 Movement, f32 Gravity=200.0f){
    physics_update *Update = MakeUpdate(Context, Entity, Entity->Layer);
    
    v2 ddP = {};
    if(Entity->PhysicsFlags & PhysicsStateFlag_Falling){
        Entity->FloorNormal = V2(0, 1);
        ddP.Y -= Gravity;
    }else if(Entity->PhysicsFlags & PhysicsStateFlag_DontFloorRaycast){
        Entity->FloorNormal = V2(0, 1);
    }
    v2 FloorNormal = Entity->FloorNormal;
    v2 FloorTangent = Normalize(TripleProduct(FloorNormal, V2(1, 0)));
    
    f32 DragCoefficient = 0.05f;
    ddP.X += -DragCoefficient*Entity->dP.X*AbsoluteValue(Entity->dP.X);
    ddP.Y += -DragCoefficient*Entity->dP.Y*AbsoluteValue(Entity->dP.Y);
    
    Entity->dP += 0.8f*(Entity->TargetdP-Entity->dP);
    
    Update->Delta = (OSInput.dTime*Entity->dP + 
                     0.5f*Square(OSInput.dTime)*ddP);
    
    Entity->TargetdP += OSInput.dTime*ddP;
    Entity->TargetdP -= FloorTangent*Dot(Entity->TargetdP, FloorTangent); 
    Entity->TargetdP += Movement*FloorTangent;
    
    Entity->Update = Update;
    
    if(Entity->Parent){
        ArrayAdd(&Context->ChildUpdates, Update);
    }
}

//~ 
void 
entity_manager::LoadTo(entity_manager *ToManager, memory_arena *Arena){
#define ENTITY_TYPE_(TypeName, ...)  \
FOR_ENTITY_TYPE(this, TypeName){ \
TypeName *Entity = It.Item;  \
TypeName *NewEntity = AllocEntity(ToManager, TypeName); \
*NewEntity = *Entity;        \
}
    ENTITY_TYPES;
#undef ENTITY_TYPE_
    
    *ToManager->Player = *Player;
    
    FOR_ENTITY_TYPE(ToManager, entity_tilemap){
        entity_tilemap *Entity = It.Item;
        Entity->TilemapData = MakeTilemapData(Arena, 
                                              Entity->Width, Entity->Height);
        Entity->PhysicsMap = PushArray(Arena, u8, Entity->Width*Entity->Height);
        
        asset_tilemap *Asset = AssetSystem.GetTilemap(Entity->Asset);
        CalculateTilemapIndices(Asset, Entity->Tiles, &Entity->TilemapData, Entity->PhysicsMap);
    }
}


//~ Entity updating and rendering

void
entity_manager::HandleInput(){
    if(OSInput.KeyJustDown(GameSettings.Jump)) PlayerInput.Jump = true;
    PlayerInput.Jump   = (PlayerInput.Jump && OSInput.KeyDown(GameSettings.Jump));
    PlayerInput.Select = OSInput.KeyJustDown(GameSettings.Select);
    PlayerInput.Left   = OSInput.KeyDown(GameSettings.PlayerLeft);
    PlayerInput.Right  = OSInput.KeyDown(GameSettings.PlayerRight);
}

void 
entity_manager::UpdateEntities(){
    TIMED_FUNCTION();
    
    physics_update_context UpdateContext = MakeUpdateContext(&TransientStorageArena, 512);
    
    
    //~ Teleporters @update_teleporter
    FOR_ENTITY_TYPE(this, entity_teleporter){
        entity_teleporter *Teleporter = It.Item;
        if(!Teleporter->IsLocked){
            if(Teleporter->IsSelected){
                
                if(Teleporter->IsSelected && PlayerInput.Select){
                    ChangeState(GameMode_MainGame, String(Teleporter->Level));
                }
                
                Teleporter->IsSelected = false;
            }
        }
    }
    
    
    //~ Match selectors @update_match
    FOR_ENTITY_TYPE(this, entity_match){
        entity_match *Entity = It.Item;
        
        if(Entity->IsSelected && PlayerInput.Select){
            ChangeState(GameMode_TicTacToe, String(""));
            asset_sound_effect *Asset = AssetSystem.GetSoundEffect(String("level_change"));
            AudioMixer.PlaySound(Asset);
        }
        
        Entity->IsSelected = false;
    }
    
    
    //~ Player @update_player
    {
        if(!DoesAnimationBlock(&Player->EntityInfo->Animation, &Player->Animation)){
            f32 MovementSpeed = Player->EntityInfo->Speed; // TODO(Tyler): Load this from a variables file
            f32 Movement = 0.0f;
            if(PlayerInput.Right && !PlayerInput.Left){
                Player->Animation.Direction = Direction_Right;
                Movement += MovementSpeed;
            }else if(PlayerInput.Left && !PlayerInput.Right){
                Player->Animation.Direction = Direction_Left;
                Movement -= MovementSpeed;
            }
            
            // TODO(Tyler): Load 'JumpTime' and 'JumpPower' from a variables file
            if(!(Player->PhysicsFlags & PhysicsStateFlag_Falling)) Player->JumpTime = 0.1f;
            local_constant f32 JumpPower = 50.0f;
            f32 Jump = 0.0f;
            if(EntityManager.PlayerInput.Jump &&
               (Player->JumpTime > 0.0f)){
                Jump += JumpPower;
                Player->JumpTime -= OSInput.dTime;
                Player->PhysicsFlags |= PhysicsStateFlag_Falling;
                
                if(Player->TargetdP.Y < 0.0f){ Player->TargetdP.Y = 0.0f; }
                if(Player->dP.Y < 0.0f){ Player->dP.Y = 0.0f; }
                Player->TargetdP += V2(0, Jump);
                
            }else if(!EntityManager.PlayerInput.Jump){
                Player->JumpTime = 0.0f;
            }
            
            if(Player->PhysicsFlags & PhysicsStateFlag_Falling){
                f32 Epsilon = 0.01f;
                if(Epsilon < Player->dP.Y){
                    ChangeEntityState(Player, Player->EntityInfo, State_Jumping);
                }else if((Player->dP.Y < -Epsilon)){
                    ChangeEntityState(Player, Player->EntityInfo, State_Falling);
                }
            }else{
                if(Movement != 0.0f) { ChangeEntityState(Player, Player->EntityInfo, State_Moving); }
                else {ChangeEntityState(Player, Player->EntityInfo, State_Idle); }
            }
            MovePlatformer(&UpdateContext, Player, Movement);
            
            if(Player->P.Y < -30.0f){
                EntityManager.DamagePlayer(2);
            }
        }
        
        v2 Center = Player->P + 0.5f*Player->EntityInfo->Size;
        GameRenderer.SetCameraTarget(Center);
    }
    
    
    DoPhysics(&UpdateContext);
}

void 
entity_manager::RenderEntities(){
    TIMED_FUNCTION();
    
    //~ Tilemaps @render_tilemap
    FOR_ENTITY_TYPE(this, entity_tilemap){
        entity_tilemap *Tilemap = It.Item;  
        asset_tilemap *Asset = AssetSystem.GetTilemap(Tilemap->Asset);
        RenderTilemap(Asset, &Tilemap->TilemapData, Tilemap->P, Tilemap->Z, Tilemap->Layer);
    }
    
    //~ Teleporters @render_teleporter
    FOR_ENTITY_TYPE(this, entity_teleporter){
        entity_teleporter *Teleporter = It.Item;
        if(!Teleporter->IsLocked){
            world_data *World = WorldManager.GetOrCreateWorld(Strings.GetString(Teleporter->Level));
            if(World){
                v2 StringP = Teleporter->P;
                StringP.Y += 0.5f;
                RenderCenteredString(&MainFont, GREEN, StringP, -1.0f, Teleporter->Level);
            }
            RenderRect(Teleporter->Bounds+Teleporter->P, 0.0f, GREEN, GameItem(1));
        }else{
            RenderRect(Teleporter->Bounds+Teleporter->P, 0.0f, 
                       MakeColor(0.0f, 0.0f, 1.0f, 0.5f), GameItem(1));
        }
    }
    
    //~ Doors @render_door
    FOR_ENTITY_TYPE(this, entity_door){
        entity_door *Door = It.Item;
        Door->Cooldown -= OSInput.dTime;
        rect R = Door->Bounds+Door->P;
        
        if(!Door->IsOpen){
            RenderRect(R, 0.0f, BROWN, GameItem(1));
        }else{
            color Color = BROWN;
            Color.A = Door->Cooldown;
            if(Color.A < 0.3f){
                Color.A = 0.3f;
            }
            RenderRect(R, 0.0f, Color, GameItem(1));
        }
    }
    
    //~ Arts @render_art
    FOR_ENTITY_TYPE(this, entity_art){
        entity_art *Art = It.Item;
        asset_art *Asset = AssetSystem.GetArt(Art->Asset);
        RenderArt(Asset, Art->P, Art->Z, Art->Layer);
        v2 Center = Art->P+0.5f*Asset->Size;
        f32 Radius = Asset->Size.Width;GameRenderer.AddLight(Center, MakeColor(1.0f, 0.6f, 0.3f, 1.0), 0.5f, Radius, GameItem(Art->Layer));
    }
    
    //~ Teleporters @render_match
    FOR_ENTITY_TYPE(this, entity_match){
        entity_match *Entity = It.Item;
        if(Entity->IsSelected){
            v2 StringP = Entity->P;
            v2 Size = RectSize(Entity->Bounds);
            StringP += 0.5f*Size;
            StringP.Y += 16;
            StringP = GameRenderer.WorldToScreen(StringP, ScaledItem(1));
            RenderCenteredString(&DebugFont, GREEN, StringP, -1.0f, "Press %s", OSKeyCodeName(GameSettings.Select));
            RenderRect(Entity->Bounds+Entity->P, 0.0f, PINK, GameItem(1));
        }else{
            RenderRect(Entity->Bounds+Entity->P, 0.0f, BLUE, GameItem(1));
        }
    }
    
    //~ Player @render_player
    {
        v2 P = Player->P;
        v2 Center = P + 0.5f*Player->EntityInfo->Size;
        GameRenderer.AddLight(Center, MakeColor(0.3f, 0.5f, 0.7f, 1.0), 1.0f, 15.0f, GameItem(1));
        DoEntityAnimation(Player->EntityInfo, &Player->Animation, P, Player->Z, 1);
        
        if(Player->Parent){
            v2 ParentP = Player->Parent->P;
            RenderRect(CenterRect(ParentP, V2(1.0f)), -10.0f, RED, GameItem(1));
        }
    }
    
#if 0
    //~ Backgrounds
    {
        TIMED_SCOPE(Backgrounds);
        asset_art *BackgroundBack   = AssetSystem.GetBackground(Strings.GetString("background_test_back"));
        asset_art *BackgroundMiddle = AssetSystem.GetBackground(Strings.GetString("background_test_middle"));
        asset_art *BackgroundFront  = AssetSystem.GetBackground(Strings.GetString("background_test_front"));
        //f32 YOffset = -200;
        f32 YOffset = 0;
        RenderArt(BackgroundBack,   V2(0*BackgroundBack->Size.Width,   YOffset), 15, 6);
        RenderArt(BackgroundBack,   V2(1*BackgroundBack->Size.Width,   YOffset), 15, 6);
        RenderArt(BackgroundMiddle, V2(0*BackgroundMiddle->Size.Width, YOffset), 14, 3);
        RenderArt(BackgroundMiddle, V2(1*BackgroundMiddle->Size.Width, YOffset), 14, 3);
        RenderArt(BackgroundFront,  V2(0*BackgroundFront->Size.Width,  YOffset), 13, 1);
        RenderArt(BackgroundFront,  V2(1*BackgroundFront->Size.Width,  YOffset), 13, 1);
        RenderArt(BackgroundFront,  V2(2*BackgroundFront->Size.Width,  YOffset), 13, 1);
    }
#endif
}