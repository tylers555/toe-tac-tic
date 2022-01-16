
//~ Helpers

internal void
ToggleWorldEditor(){
    if(GameMode == GameMode_WorldEditor){
        // To main game from editor
        ChangeState(GameMode_MainGame, WorldEditor.World->Name);
        Score = 0;
    }else if(GameMode == GameMode_MainGame){
        // To editor from main game
        ChangeState(GameMode_WorldEditor, MakeString(0));
        WorldEditor.EditModeEntity(0);
    }
}

#if 0
internal void
CleanupEntity(world_entity *Entity){
    switch(Entity->Type){
        case ENTITY_TYPE(entity_tilemap): {
            DefaultFree(Entity->Tilemap.Tiles);
        }break;
        case EntityType_Teleporter: {
            
        }break;
        case EntityType_Door: {
            Strings.RemoveBuffer(Entity->Door.RequiredLevel);
        }break;
    }
}

internal world_entity
CopyEntity(memory_arena *Arena, world_entity *Entity){
    world_entity Result = *Entity;
    switch(Result.Type){
        case ENTITY_TYPE(entity_tilemap): {
            u32 MapSize = Result.Tilemap.Width*Result.Tilemap.Height*sizeof(*Entity->Tilemap.Tiles);;
            if(Arena){
                Entity->Tilemap.Tiles = (tilemap_tile *)ArenaPush(Arena, MapSize);
            }else{
                Entity->Tilemap.Tiles = (tilemap_tile *)DefaultAlloc(MapSize);
            }
            CopyMemory(Result.Tilemap.Tiles, Entity->Tilemap.Tiles, MapSize);
        }break;
        case EntityType_Teleporter: {
            if(Arena){
                Result.Teleporter.Level = ArenaPushCString(Arena, Entity->Teleporter.Level);
                Result.Teleporter.RequiredLevel = ArenaPushCString(Arena, Entity->Teleporter.RequiredLevel);
            }else{
                Result.Teleporter.Level = Strings.MakeBuffer();
                Result.Teleporter.RequiredLevel = Strings.MakeBuffer();
            }
        }break;
        case EntityType_Door: {
            if(Arena){
                Result.Door.RequiredLevel = ArenaPushCString(Arena, Entity->Door.RequiredLevel);
            }else{
                Result.Door.RequiredLevel = Strings.MakeBuffer();
            }
        }break;
    }
    
    return(Result);
}
#endif

internal inline color
GetEditorColor(u64 ID, b8 Active, b8 Hidden=true){
    ui_button_state *State = FindOrCreateInHashTablePtr(&UIManager.ButtonStates, ID);
    color C;
    if(Hidden) { C = EDITOR_HOVERED_COLOR; C.A = 0.0f; }
    else{        C = EDITOR_BASE_COLOR; }
    State->T = Clamp(State->T, 0.0f, 1.0f);
    f32 T = 1.0f-Square(1.0f-State->T);
    C = MixColor(EDITOR_HOVERED_COLOR, C,  T);
    
    if(Active) State->ActiveT += 10*OSInput.dTime; 
    else       State->ActiveT -= 7*OSInput.dTime; 
    State->ActiveT = Clamp(State->ActiveT, 0.0f, 1.0f);
    
    f32 ActiveT = 1.0f - Square(1.0f-State->ActiveT);
    C = MixColor(EDITOR_SELECTED_COLOR, C,  ActiveT);
    return(C);
}

internal inline void
ToggleFlag(u32 *Flag, u32 Value){
    if(*Flag & Value) *Flag &= ~Value;
    else              *Flag |= Value;
}

inline b8
world_editor::IsEntityHidden(entity *Entity){
#if 0
    world_entity_group *Group = &World->EntityGroups[Entity->Base.GroupIndex];
    if((Group->Flags & WorldEntityEditFlag_Hide) &&  
       !(EditorFlags & WorldEditorFlag_UnhideHiddenGroups)){
        return true;
    }
#endif
    return false;
}

inline void
world_editor::EditModeEntity(entity *Entity){
    TilemapDoSelectorOverlay = true;
    TilemapEditMode = TilemapEditMode_Auto;
    AutoTileMode = AutoTile_Tile;
    ManualTileIndex = 0;
    Selected = Entity;
    EditMode = EditMode_Entity;
}

//~ Snapping

internal inline v2
SnapToGrid(v2 P, editor_grid Grid){
    P.X /= Grid.CellSize.X;
    P.Y /= Grid.CellSize.Y;
    v2 Result = V2(Round(P.X), Round(P.Y));
    Result.X *= Grid.CellSize.X;
    Result.Y *= Grid.CellSize.Y;
    
    return(Result);
}

internal inline rect
SnapToGrid(rect R, editor_grid Grid){
    rect Result;
    if(R.Min.X < R.Max.X){
        Result.Min.X = Floor(R.Min.X/Grid.CellSize.X);
        Result.Max.X =  Ceil(R.Max.X/Grid.CellSize.X);
    }else{
        Result.Min.X =  Ceil(R.Min.X/Grid.CellSize.X);
        Result.Max.X = Floor(R.Max.X/Grid.CellSize.X);
    }
    if(R.Min.Y < R.Max.Y){
        Result.Min.Y = Floor(R.Min.Y/Grid.CellSize.Y);
        Result.Max.Y =  Ceil(R.Max.Y/Grid.CellSize.Y);
    }else{
        Result.Min.Y =  Ceil(R.Min.Y/Grid.CellSize.Y);
        Result.Max.Y = Floor(R.Max.Y/Grid.CellSize.Y);
    }
    Result.X0 *= Grid.CellSize.X;
    Result.X1 *= Grid.CellSize.X;
    Result.Y0 *= Grid.CellSize.Y;
    Result.Y1 *= Grid.CellSize.Y;
    
    // TODO(Tyler): Maybe fix rect?
    Result = RectFix(Result);
    
    return(Result);
}

//~ Undo/redo

void
world_editor::ClearActionHistory(){
    for(u32 I=0; I<Actions.Count; I++){
        editor_action *Action = &Actions[I];
        switch(Action->Type){
            case EditorAction_AddEntity: {}break;
            case EditorAction_DeleteEntity: {}break;
        }
    }
    
    ArrayClear(&Actions);
    ArenaClear(&ActionMemory);
    
    ActionIndex = 0;
    IDCounter = 1;
}

void 
world_editor::Undo(){
    if(ActionIndex == 0) return;
    
    ActionIndex--;
    editor_action *Action = &Actions[ActionIndex];
    switch(Action->Type){
        case EditorAction_AddEntity: {
#if 0
            for(u32 I=0; I<World->Entities.Count; I++){
                world_entity *Entity = &World->Entities[I];
                if(Entity->ID == Action->Entity.ID){
                    Action->Entity = CopyEntity(&ActionMemory, Entity);
                    DeleteEntityAction(Entity, EditorDeleteFlag_UndoDeleteFlags);
                    break;
                }
            }
#endif
            
        }break;
        case EditorAction_DeleteEntity: {
#if 0
            world_entity *Entity = AddEntityAction(false);
            *Entity = CopyEntity(0, &Action->Entity);
            EditModeEntity(Entity);
#endif
        }break;
    }
}

void 
world_editor::Redo(){
    if(Actions.Count == 0) return;
    if(ActionIndex == Actions.Count) return;
    
    editor_action *Action = &Actions[ActionIndex];
    ActionIndex++;
    switch(Action->Type){
        case EditorAction_AddEntity: {
#if 0
            world_entity *Entity = AddEntityAction(false);
            *Entity = CopyEntity(0, &Action->Entity);
            EditModeEntity(Entity);
#endif
        }break;
        case EditorAction_DeleteEntity: {
#if 0
            for(u32 I=0; I<World->Entities.Count; I++){
                world_entity *Entity = &World->Entities[I];
                if(Entity->ID == Action->Entity.ID){
                    Action->Entity = CopyEntity(&ActionMemory, Entity);
                    DeleteEntityAction(Entity, EditorDeleteFlag_UndoDeleteFlags);
                    break;
                }
            }
#endif
            
        }break;
    }
}

editor_action *
world_editor::MakeAction(editor_action_type Type){
    while(ActionIndex < Actions.Count){
        ArrayOrderedRemove(&Actions, ActionIndex);
    }
    Assert(ActionIndex == Actions.Count);
    ActionIndex++;
    editor_action *Action = ArrayAlloc(&Actions);
    Action->Type = Type;
    return(Action);
}

#if 0
world_entity *
world_editor::AddEntityAction(b8 Commit){
    world_entity *Result = 0;
#if 0
    *Result = DefaultWorldEntity();
    Result->ID = IDCounter++;
    Assert(Result->ID);
    if(Commit){
        editor_action *Action = MakeAction(EditorAction_AddEntity);
        Action->Entity.ID = Result->ID;
    }
#endif
    return(Result);
}
#endif

inline void
world_editor::DeleteEntityAction(entity *Entity, editor_delete_flags Flags){
    EntityToDelete = Entity;
    DeleteFlags = Flags;
}

inline f32
world_editor::GetCursorZ(){
    f32 Result = (f32)World->Manager.EntityCount + 1.0f;
    return(Result);
}

//~ Selector
internal inline selector_context 
BeginSelector(v2 StartP, u32 SelectedIndex, u32 Max, os_key_flags ScrollModifier=SELECTOR_SCROLL_MODIFIER){
    selector_context Result = {};
    Result.StartP = StartP;
    Result.P = Result.StartP;
    Result.MaxItemSide = 100;
    Result.WrapWidth   = 250;
    Result.ScrollModifier = ScrollModifier;
    Result.SelectedIndex = SelectedIndex;
    Result.MaxIndex = Max;
    Result.ValidIndices = PushArray(&TransientStorageArena, b8, Result.MaxIndex);
    
    return(Result);
}

internal inline v2
SelectorClampSize(selector_context *Selector, v2 Size){
    v2 Result = Size;
    f32 AspectRatio = Size.Y/Size.X;
    if(Size.X > Selector->MaxItemSide){
        Result.X = Selector->MaxItemSide;
        Result.Y = Size.X*AspectRatio;
    }
    if(Size.Y > Selector->MaxItemSide){
        Result.Y = Selector->MaxItemSide;
        Result.X = Size.Y/AspectRatio;
    }
    
    return(Result);
}

internal void
SelectorDoItem(selector_context *Selector, u64 ID, v2 Size, u32 Index){
    b8 Result = (Index==Selector->SelectedIndex);
    ui_button_state *State = FindOrCreateInHashTablePtr(&UIManager.ButtonStates, ID);
    
    rect R = SizeRect(Selector->P, Size);
    switch(EditorButtonElement(&UIManager, ID, R, MouseButton_Left, 0, UIItem(0), KeyFlag_Any)){
        case UIBehavior_Activate: {
            Result = true;
        }break;
    }
    
    color C = EDITOR_HOVERED_COLOR; C.A = 0.0f;
    State->T = Clamp(State->T, 0.0f, 1.0f);
    f32 T = 1.0f-Square(1.0f-State->T);
    C = MixColor(EDITOR_HOVERED_COLOR, C,  T);
    
    if(Result) State->ActiveT += 10*OSInput.dTime; 
    else       State->ActiveT -= 7*OSInput.dTime; 
    State->ActiveT = Clamp(State->ActiveT, 0.0f, 1.0f);
    
    f32 ActiveT = 1.0f - Square(1.0f-State->ActiveT);
    C = MixColor(EDITOR_SELECTED_COLOR, C,  ActiveT);
    RenderRectOutline(R, -2.1f, C, ScaledItem(0), Selector->Thickness);
    
    f32 XAdvance = Selector->Spacer+Size.X;
    Selector->P.X += XAdvance;
    
    if(Selector->P.X > 300.0f){
        Selector->P.X = DEFAULT_SELECTOR_P.X;
        Selector->P.Y += 40.0f;
    }
    
    Selector->ValidIndices[Index] = true;
    if(Result){
        Selector->SelectedIndex = Index; 
        Selector->DidSelect = true;
    }
}

internal u32 
SelectorSelectItem(selector_context *Selector){
    u32 Result = Selector->SelectedIndex;
    
    if(!Selector->DidSelect){
        while(!Selector->ValidIndices[Result]){
            if(Result == Selector->MaxIndex-1) Result = 0; 
            else Result++; 
        }
    }
    
    if(UIManager.DoScrollElement(WIDGET_ID, -1, Selector->ScrollModifier)){
        s32 Range = 100;
        s32 Scroll = UIManager.ActiveElement.Scroll;
        if(Scroll > Range){
            do{
                if(Result == Selector->MaxIndex-1) Result = 0; 
                else Result++; 
            }while(!Selector->ValidIndices[Result]);
        }else if(Scroll < -Range){
            do{
                if(Result == 0) Result = Selector->MaxIndex-1;
                else Result--;
            }while(!Selector->ValidIndices[Result]);
        }
    }
    
    return(Result);
}


#define StringSelectorSelectItem(Array, Var) \
Var = Array[SelectorSelectItem(&Selector)]


//~ Edit things
void 
world_editor::DoEditThingTilemap(){
    //~ Selector
    u32 SelectedIndex = 0;
    array<string> Tilemaps = MakeArray<string>(&TransientStorageArena, AssetSystem.Tilemaps.BucketsUsed);
    for(u32 I=0; I < AssetSystem.Tilemaps.MaxBuckets; I++){
        string Key = AssetSystem.Tilemaps.Keys[I];
        if(Key.ID){ 
            if(Key == TilemapToAdd) SelectedIndex = Tilemaps.Count;
            ArrayAdd(&Tilemaps, Key); 
        }
    }
    
    selector_context Selector = BeginSelector(DEFAULT_SELECTOR_P, SelectedIndex, Tilemaps.Count);
    for(u32 I=0; I<Tilemaps.Count; I++){
        asset_tilemap *Tilemap = AssetSystem.GetTilemap(Tilemaps[I]);
        v2 Size = SelectorClampSize(&Selector, Tilemap->CellSize);
        
        RenderTileAtIndex(Tilemap, Selector.P, -2.0f, 0, 0);
        
        SelectorDoItem(&Selector, WIDGET_ID_CHILD(WIDGET_ID, I+1), Size, I);
    }
    StringSelectorSelectItem(Tilemaps, TilemapToAdd);
    
    //~ Cursor
    asset_tilemap *Asset = AssetSystem.GetTilemap(TilemapToAdd);
    v2 Size = Asset->CellSize;
    
    v2 P = SnapToGrid(MouseP, Grid);
    
    RenderTileAtIndex(Asset, P, GetCursorZ(), 1, 0);
    
    //~ Adding
    if(UIManager.DoClickElement(WIDGET_ID, MouseButton_Left, true, -2)){
        entity_tilemap *Entity = AllocEntity(&World->Manager, entity_tilemap);
        
        u32 WidthU32  = 10;
        u32 HeightU32 = 10;
        f32 Width  = (f32)WidthU32 * Asset->TileSize.X;
        f32 Height = (f32)HeightU32 * Asset->TileSize.Y;
        
        SetupDefaultEntity(Entity, ENTITY_TYPE(entity_tilemap), 
                           SnapToGrid(MaximumV2(MouseP-0.5f*V2(Width, Height), V2(0)), Grid),
                           Asset->Boundaries, Asset->BoundaryCount);
        
        Entity->Bounds = SizeRect(V2(0), V2(Width, Height));
        Entity->Asset = TilemapToAdd;
        Entity->Width = WidthU32;
        Entity->Height = HeightU32;
        u32 MapSize = Entity->Width*Entity->Height*sizeof(*Entity->Tiles);
        Entity->Tiles = (tilemap_tile *)DefaultAlloc(MapSize);
        Entity->TileSize = Asset->TileSize;
        Entity->Layer = 1;
        
        EditThing = EditThing_None;
        EditModeEntity(Entity);
    }
}

void
world_editor::DoEditThingTeleporter(){
    //~ Cursor
    rect R = SizeRect(CursorP, TILE_SIZE);
    RenderRect(R, GetCursorZ(), GREEN, GameItem(1));
    RenderRectOutline(R, -0.1f, BLACK, GameItem(1));
    
    //~ Adding
    if(UIManager.DoClickElement(WIDGET_ID, MouseButton_Left, true, -2)){
        entity_teleporter *Entity = AllocEntity(&World->Manager, entity_teleporter);
        SetupTriggerEntity(Entity, ENTITY_TYPE(entity_teleporter), CursorP,
                           World->TeleporterBoundary, 1,
                           TeleporterResponse);
        
        Entity->Level         = Strings.MakeBuffer();
        Entity->RequiredLevel = Strings.MakeBuffer();
        
        EditModeEntity(Entity);
    }
}

void
world_editor::DoEditThingDoor(){
    switch(UIManager.DoClickElement(WIDGET_ID, MouseButton_Left, false, -2)){
        case UIBehavior_None: {
            RenderRect(SizeRect(CursorP, TILE_SIZE), GetCursorZ(), BROWN, GameItem(1));
        }break;
        case UIBehavior_JustActivate: {
            DragRect.Min = MouseP;
        }case UIBehavior_Activate: {
            DragRect.Max = MouseP;
            
            rect R = SnapToGrid(DragRect, Grid);
            RenderRect(R, GetCursorZ(), BROWN, GameItem(1));
        }break;
        case UIBehavior_Deactivate: {
            //world_entity *Entity = ArrayAlloc(&World->Entities);
            entity_door *Entity = AllocEntity(&World->Manager, entity_door);
            rect R = SnapToGrid(DragRect, Grid);
            Entity->Type = ENTITY_TYPE(entity_door);
            Entity->P = R.Min;
            Entity->P = SnapToGrid(Entity->P, Grid);
            Entity->Bounds = R;
            Entity->Bounds -= Entity->Bounds.Min;
            Entity->RequiredLevel = Strings.MakeBuffer();
            EditModeEntity(Entity);
        }break;
    }
}

void
world_editor::DoEditThingArt(){
    //~ Selector
    u32 SelectedIndex = 0;
    array<string> Arts = MakeArray<string>(&TransientStorageArena, AssetSystem.Arts.BucketsUsed);
    for(u32 I=0; I < AssetSystem.Arts.MaxBuckets; I++){
        string Key = AssetSystem.Arts.Keys[I];
        if(Key.ID){ 
            if(Key == ArtToAdd) SelectedIndex = Arts.Count;
            ArrayAdd(&Arts, Key); 
        }
    }
    
    selector_context Selector = BeginSelector(DEFAULT_SELECTOR_P, SelectedIndex, Arts.Count);
    for(u32 I=0; I<Arts.Count; I++){
        asset_art *Asset = AssetSystem.GetArt(Arts[I]);
        v2 Size = SelectorClampSize(&Selector, 0.5f*Asset->Size);
        
        RenderTexture(SizeRect(Selector.P, Size), -2.0f, Asset->Texture, GameItem(0), MakeRect(V2(0), V2(1)), false);
        
        SelectorDoItem(&Selector, WIDGET_ID_CHILD(WIDGET_ID, I+1), Size, I);
    }
    StringSelectorSelectItem(Arts, ArtToAdd);
    
    //~ Cursor
    asset_art *Asset = AssetSystem.GetArt(ArtToAdd);
    v2 Size = Asset->Size;
    
    v2 P = SnapToGrid(MouseP, Grid);
    
    RenderArt(Asset, P, GetCursorZ(), 1);
    
    //~ Adding
    if(UIManager.DoClickElement(WIDGET_ID, MouseButton_Left, true, -2)){
        entity_art *Entity = AllocEntity(&World->Manager, entity_art);
        
        Entity->Type = ENTITY_TYPE(entity_art);
        Entity->P = SnapToGrid(P, Grid);
        Entity->Asset = ArtToAdd;
        Entity->Bounds = SizeRect(V2(0), Size);
    }
}

void
world_editor::DoEditThingMatch(){
    //~ Cursor
    rect R = SizeRect(CursorP, TILE_SIZE);
    RenderRect(R, GetCursorZ(), BLUE, GameItem(1));
    RenderRectOutline(R, -0.1f, BLACK, GameItem(1));
    
    //~ Adding
    if(UIManager.DoClickElement(WIDGET_ID, MouseButton_Left, true, -2)){
        entity_match *Entity = AllocEntity(&World->Manager, entity_match);
        SetupTriggerEntity(Entity, ENTITY_TYPE(entity_match), CursorP,
                           World->TeleporterBoundary, 1,
                           MatchResponse);
        
        EditModeEntity(Entity);
    }
}

//~ Group editing
void
world_editor::DoEditModeGroup(){
    
}

//~ Tilemap editing
internal void
ResizeTilemapData(entity_tilemap *Tilemap, s32 XChange, s32 YChange){
    u32 OldWidth = Tilemap->Width;
    u32 OldHeight = Tilemap->Height;
    
    if((s32)Tilemap->Width + XChange > 0){ Tilemap->Width += XChange; }
    if((s32)Tilemap->Height + YChange > 0){ Tilemap->Height += YChange; }
    
    u32 Width = Tilemap->Width;
    u32 Height = Tilemap->Height;
    tilemap_tile *NewTiles = (tilemap_tile *)DefaultAlloc(Width*Height*sizeof(*Tilemap->Tiles));
    for(u32 Y=0; Y < Minimum(OldHeight, Height); Y++){
        for(u32 X=0; X < Minimum(OldWidth, Width); X++){
            NewTiles[Y*Width + X] = Tilemap->Tiles[Y*OldWidth + X];;
        }
    }
    DefaultFree(Tilemap->Tiles);
    Tilemap->Tiles = NewTiles;
}

internal void
MoveTilemapData(entity_tilemap *Tilemap, s32 XOffset, s32 YOffset){
    u32 Width = Tilemap->Width;
    u32 Height = Tilemap->Height;
    tilemap_tile *TempTiles = PushArray(&TransientStorageArena, tilemap_tile, Width*Height);
    for(u32 Y=0; Y<Height; Y++){
        for(u32 X=0; X < Width; X++){
            s32 OldX = X+XOffset;
            s32 OldY = Y+YOffset;
            TempTiles[Y*Width + X] = {};
            if((0 <= OldX) && (OldX < (s32)Width) && 
               (0 <= OldY) && (OldY < (s32)Height)){
                TempTiles[Y*Width + X] = Tilemap->Tiles[OldY*Width + OldX];
            }
        }
    }
    CopyMemory(Tilemap->Tiles, TempTiles, Width*Height*sizeof(*Tilemap->Tiles));
}

enum tilemap_edge_action {
    TilemapEdgeAction_None,
    TilemapEdgeAction_Incrememt,
    TilemapEdgeAction_Decrement,
};

internal tilemap_edge_action
EditorTilemapEdge(f32 X, f32 Y, f32 Width, f32 Height, u64 ID){
    tilemap_edge_action Result = TilemapEdgeAction_None;
    rect R = SizeRect(V2(X, Y), V2(Width, Height));
    R = RectFix(R);
    
    ui_button_state *State = FindOrCreateInHashTablePtr(&UIManager.ButtonStates, ID);
    switch(EditorButtonElement(&UIManager, ID, R, MouseButton_Left, 1, ScaledItem(1), EDIT_TILEMAP_MODIFIER)){
        case UIBehavior_Activate: {
            State->ActiveT = 1.0f;
            Result = TilemapEdgeAction_Incrememt;
        }break;
    }
    
    switch(EditorButtonElement(&UIManager, WIDGET_ID_CHILD(ID, 1), R, MouseButton_Right, 1, ScaledItem(1), EDIT_TILEMAP_MODIFIER)){
        case UIBehavior_Activate: {
            State->ActiveT = 1.0f;
            Result = TilemapEdgeAction_Decrement;
        }break;
    }
    
    State->T = Clamp(State->T, 0.0f, 1.0f);
    f32 T = 1.0f-Square(1.0f-State->T);
    color C = MixColor(EDITOR_HOVERED_COLOR, EDITOR_BASE_COLOR, T);
    
    if(State->ActiveT > 0.0f){
        f32 ActiveT = Sin(State->ActiveT*PI);
        State->ActiveT -= 7*OSInput.dTime;
        C = MixColor(EDITOR_SELECTED_COLOR, C, ActiveT);
    }
    
    RenderRect(R, -0.1f, C, GameItem(1));
    return(Result);
}

internal inline b8
IsInTilemap(entity_tilemap *Tilemap, s32 X, s32 Y){
    b8 Result = !((X < 0)                    || (Y < 0)       || 
                  (X >= (s32)Tilemap->Width) || (Y >= (s32)Tilemap->Height));
    return(Result);
}

void
world_editor::MaybeEditTilemap(){
    TIMED_FUNCTION();
    
    if(!Selected) return;
    if(Selected->Type != ENTITY_TYPE(entity_tilemap)) return;
    if((EditMode == EditMode_Entity) || (EditMode == EditMode_TilemapTemporary)){
        b8 DoEdit = OSInput.TestModifier(KeyFlag_Any|EDIT_TILEMAP_MODIFIER);
        if(DoEdit) EditMode = EditMode_TilemapTemporary;
        else       EditMode = EditMode_Entity;
    }
    
    if((EditMode != EditMode_Tilemap) && (EditMode != EditMode_TilemapTemporary)) return;
    
    entity_tilemap *Tilemap = (entity_tilemap *)Selected;
    asset_tilemap *Asset = AssetSystem.GetTilemap(Tilemap->Asset);
    
    v2 TileP = MouseP - Tilemap->P;
    TileP = FloorV2(TileP);
    TileP.X /= Asset->TileSize.X;
    TileP.Y /= Asset->TileSize.Y;
    s32 X = (s32)(TileP.X);
    s32 Y = (s32)(TileP.Y);
    u32 MapIndex = Y*Tilemap->Width+X;
    
    //~ Resizing 
    b8 DidResize = false;
    {
        v2 Size = V2(10);
        rect R = CenterRect(Tilemap->P, Size);
        f32 EdgeSize = 3;
        
        //tilemap_edge_action RightEdge = EditorTilemapEdge(R.Max.X, R.Min.Y, EdgeSize, Size.Y, WIDGET_ID);
        tilemap_edge_action RightEdge = TilemapEdgeAction_None;
        if(     OSInput.KeyRepeat(KeyCode_Right, KeyFlag_Any|KeyFlag_Control)) RightEdge = TilemapEdgeAction_Incrememt;
        else if(OSInput.KeyRepeat(KeyCode_Left,  KeyFlag_Any|KeyFlag_Control)) RightEdge = TilemapEdgeAction_Decrement;
        switch(RightEdge){
            case TilemapEdgeAction_Incrememt: ResizeTilemapData(Tilemap,  1, 0); break;
            case TilemapEdgeAction_Decrement: ResizeTilemapData(Tilemap, -1, 0); break;
        }
        if(RightEdge != TilemapEdgeAction_None) DidResize = true;
        
        //tilemap_edge_action TopEdge = EditorTilemapEdge(R.Min.X, R.Max.Y, Size.X, EdgeSize, WIDGET_ID);
        tilemap_edge_action TopEdge = TilemapEdgeAction_None;
        if(     OSInput.KeyRepeat(KeyCode_Up,   KeyFlag_Any|KeyFlag_Control)) TopEdge = TilemapEdgeAction_Incrememt;
        else if(OSInput.KeyRepeat(KeyCode_Down, KeyFlag_Any|KeyFlag_Control)) TopEdge = TilemapEdgeAction_Decrement;
        switch(TopEdge){
            case TilemapEdgeAction_Incrememt: ResizeTilemapData(Tilemap, 0,  1); break;
            case TilemapEdgeAction_Decrement: ResizeTilemapData(Tilemap, 0, -1);break;
        }
        if(TopEdge != TilemapEdgeAction_None) DidResize = true;
    }
    
    //~ Moving map
    if(!DidResize){
        s32 XOffset = 0;
        s32 YOffset = 0;
        if(     OSInput.KeyRepeat(KeyCode_Up, KeyFlag_Any)) { YOffset = -1; } 
        else if(OSInput.KeyRepeat(KeyCode_Down, KeyFlag_Any)) { YOffset =  1; }
        if(     OSInput.KeyRepeat(KeyCode_Left, KeyFlag_Any)) { XOffset =  1; } 
        else if(OSInput.KeyRepeat(KeyCode_Right, KeyFlag_Any)) { XOffset = -1; }
        if((XOffset != 0) || (YOffset != 0)) MoveTilemapData(Tilemap, XOffset, YOffset); 
    }
    
    //~ Remove tiles
    if(UIManager.DoClickElement(WIDGET_ID, MouseButton_Right, false, -1, KeyFlag_Any) &&
       IsInTilemap(Tilemap, X, Y)){
        Tilemap->Tiles[MapIndex] = {};
    }
    
    //~ Edit mode
    if(OSInput.KeyJustDown('T', KeyFlag_Any)){
        TilemapDoSelectorOverlay = true;
        if(TilemapEditMode == TilemapEditMode_Auto){
            TilemapEditMode = TilemapEditMode_Manual;
        }else if(TilemapEditMode == TilemapEditMode_Manual){
            TilemapEditMode = TilemapEditMode_Auto;
        }else{ INVALID_CODE_PATH; }
    }
    
    //~ Manual tiles scrolling
    if(UIManager.DoScrollElement(WIDGET_ID, 0, KeyFlag_Shift|KeyFlag_Any) && 
       IsInTilemap(Tilemap, X, Y)){
        if(Tilemap->Tiles[MapIndex].Type){
            u32 Index = 0;
            if(Tilemap->Tiles[MapIndex].OverrideID == 0){
                Index = 0;
            }else{
                for(u32 I=0; I<Asset->TileCount; I++){
                    if(Asset->Tiles[I].ID == (Tilemap->Tiles[MapIndex].OverrideID-1)){
                        Index = I;
                        break;
                    }
                }
                
                s32 Range = 100;
                s32 Scroll = UIManager.ActiveElement.Scroll;
                if(Scroll > Range){
                    Index++;
                    Index %= Asset->TileCount;
                }else if(Scroll < Range){
                    if(Index > 0) Index--;
                    else Index = Asset->TileCount-1;
                }
            }
            
            Tilemap->Tiles[MapIndex].OverrideID = Asset->Tiles[Index].ID+1;
        }
    }
    
    //~ Variation scrolling
    if(UIManager.DoScrollElement(WIDGET_ID, 0, KeyFlag_Control|KeyFlag_Any) && 
       IsInTilemap(Tilemap, X, Y)){
        if(Tilemap->Tiles[MapIndex].Type){
            s32 Range = 100;
            s32 Scroll = UIManager.ActiveElement.Scroll;
            if(Scroll > Range){
                Tilemap->Tiles[MapIndex].OverrideVariation++;
            }else if(Scroll < Range){
                Tilemap->Tiles[MapIndex].OverrideVariation--;
            }
        }
    }
    
    //~ Toggle selector 
    if(OSInput.KeyJustDown('S', KeyFlag_Any)){
        TilemapDoSelectorOverlay = !TilemapDoSelectorOverlay;
    }
    
    //~ Reset tile
    if(UIManager.DoClickElement(WIDGET_ID, MouseButton_Middle, true, -1, KeyFlag_Shift|KeyFlag_Any) &&
       IsInTilemap(Tilemap, X, Y)){
        Tilemap->Tiles[MapIndex].OverrideID = 0;
    }
    
    if(UIManager.DoClickElement(WIDGET_ID, MouseButton_Middle, true, -1, KeyFlag_Control|KeyFlag_Any) &&
       IsInTilemap(Tilemap, X, Y)){
        Tilemap->Tiles[MapIndex].OverrideVariation = 0;
    }
    
    if(TilemapEditMode == TilemapEditMode_Auto){
        //~ Auto tile selector
        if(TilemapDoSelectorOverlay){
            selector_context Selector = BeginSelector(DEFAULT_SELECTOR_P, AutoTileMode, AutoTile_TOTAL, KeyFlag_Any);
            for(s32 I=0; I < AutoTile_TOTAL; I++){
                tile_type PreType = TILE_EDIT_MODE_TILE_TYPE_TABLE[I];
                
                tile_type Type = PreType;
                tile_flags Flags = 0;
                if(PreType & TileTypeFlag_Art){
                    Flags |= TileFlag_Art;
                }
                
                b8 FoundTile = false;
                u32 Index = 0;
                for(u32 I=0; I<Asset->TileCount; I++){
                    asset_tilemap_tile_data *Tile = &Asset->Tiles[I];
                    if(((Tile->Type & Type)) &&
                       (Tile->Flags == Flags)){
                        Index = Tile->OffsetMin;
                        FoundTile = true;
                        break;
                    }
                }
                if(!FoundTile){ continue; }
                
                v2 Size = SelectorClampSize(&Selector, Asset->CellSize);
                RenderTileAtIndex(Asset, Selector.P, -2.0f, 0, Index);
                
                SelectorDoItem(&Selector, WIDGET_ID_CHILD(WIDGET_ID, I+1), Size, I);
            }
            AutoTileMode = (auto_tile)SelectorSelectItem(&Selector);
        }
        
        //~ Add auto tiles
        if(UIManager.DoClickElement(WIDGET_ID, MouseButton_Left, false, -1, KeyFlag_Any) &&
           IsInTilemap(Tilemap, X, Y)){
            Tilemap->Tiles[MapIndex].Type = (u8)TILE_EDIT_MODE_TILE_TYPE_TABLE[AutoTileMode];
            Tilemap->Tiles[MapIndex].OverrideID = 0;
        }
        
    }else if(TilemapEditMode == TilemapEditMode_Manual){
        
        //~ Manual tile selector
        if(TilemapDoSelectorOverlay){
            selector_context Selector = BeginSelector(DEFAULT_SELECTOR_P, ManualTileIndex, Asset->TileCount, KeyFlag_Any);
            for(u32 I=0; I < Asset->TileCount; I++){
                asset_tilemap_tile_data *Tile = &Asset->Tiles[I];
                
                v2 Size = SelectorClampSize(&Selector, Asset->CellSize);
                RenderTileAtIndex(Asset, Selector.P, -2.0f, 0, Tile->OffsetMin, Tile->Transform);
                
                SelectorDoItem(&Selector, WIDGET_ID_CHILD(WIDGET_ID, I+1), Size, I);
            }
            ManualTileIndex = (u16)SelectorSelectItem(&Selector);
        }
        
        //~ Add manual tiles
        if(UIManager.DoClickElement(WIDGET_ID, MouseButton_Left, false, -1, KeyFlag_Any) &&
           IsInTilemap(Tilemap, X, Y)){
            Tilemap->Tiles[MapIndex].Type = (u8)1;
            Tilemap->Tiles[MapIndex].OverrideID = Asset->Tiles[ManualTileIndex].ID+1;
        }
        
        
    }else{ INVALID_CODE_PATH };
    
}

//~ Selected thing
b8
world_editor::DoButton(rect R, u64 ID){
    b8 Result = false;
    
    ui_button_state *State = FindOrCreateInHashTablePtr(&UIManager.ButtonStates, ID);
    
    switch(EditorButtonElement(&UIManager, ID, R, MouseButton_Left, -1, ScaledItem(1))){
        case UIBehavior_Activate: {
            State->ActiveT = 1.0f;
            Result = true;
        }break;
    }
    
    State->T = Clamp(State->T, 0.0f, 1.0f);
    f32 T = 1.0f-Square(1.0f-State->T);
    color C = MixColor(EDITOR_HOVERED_COLOR, EDITOR_BASE_COLOR, T);
    
    if(State->ActiveT > 0.0f){
        f32 ActiveT = Sin(State->ActiveT*PI);
        State->ActiveT -= 7*OSInput.dTime;
        C = MixColor(EDITOR_SELECTED_COLOR, C, ActiveT);
    }
    
    State->ActiveT = Clamp(State->ActiveT, 0.0f, 1.0f);
    f32 ActiveT = 1.0f-Square(1.0f-State->ActiveT);
    C = MixColor(EDITOR_SELECTED_COLOR, C, ActiveT);
    
    RenderRect(R, -0.2f, C, GameItem(1));
    
    return(Result);
}

void
world_editor::DoSelectedThingUI(){
    TIMED_FUNCTION();
    if(!Selected) return;
    if(EditMode == EditMode_Group) return;
    if(IsEntityHidden(Selected)) return;
    
    ui_window *Window = UIManager.BeginWindow("Edit entity", V2(0, OSInput.WindowSize.Y));
    if(Window->BeginSection("Entity options", WIDGET_ID, 10, true)){
        TOGGLE_FLAG(Window, "Hide overlays", EditorFlags, WorldEditorFlag_HideOverlays);
        
        //Window->Text("ID: %u", Selected->ID);
        
        Window->EndSection();
    }
    
    switch(Selected->Type){
        case ENTITY_TYPE(entity_tilemap): {
            if(Window->BeginSection("Edit tilemap", WIDGET_ID, 10, true)){
                entity_tilemap *Tilemap = (entity_tilemap *)Selected;
                
                Window->Text("Hold 'Alt' to edit tilemap");
                
                if(Window->Button("Delete tilemap", WIDGET_ID)){
                    DeleteEntityAction(Selected);
                }
                
                if(Window->Button("Match tilemap to world", WIDGET_ID)){
                    Tilemap->P = V2(0);
                    
                    ResizeTilemapData(Tilemap, 
                                      (s32)World->Width-(s32)Tilemap->Width,
                                      (s32)World->Height-(s32)Tilemap->Height);
                }
                
                b8 OverrideEditTilemap = (EditMode == EditMode_Tilemap);
                OverrideEditTilemap = Window->ToggleBox("Edit tilemap(E)", OverrideEditTilemap, WIDGET_ID);
                if(OSInput.KeyJustDown('E')) OverrideEditTilemap = !OverrideEditTilemap;
                if(OverrideEditTilemap != (EditMode == EditMode_Tilemap)){
                    if(EditMode == EditMode_Tilemap) EditMode = EditMode_Entity;
                    else EditMode = EditMode_Tilemap;
                }
                
#if 0    
                TOGGLE_FLAG(Window, "Treat edges as tiles", 
                            Selected->Base.Flags, WorldEntityTilemapFlag_TreatEdgesAsTiles);
#endif
                //NOT_IMPLEMENTED_YET;
                
                Window->EndSection();
            }
        }break;
        case ENTITY_TYPE(entity_player): {
            entity_player *Player = (entity_player *)Selected;
            
            if(EditorFlags & WorldEditorFlag_HideOverlays) break;
            //asset_entity *EntityInfo = AssetSystem.GetEntity(Selected->Base.Asset);
            
            asset_entity *EntityInfo = Player->EntityInfo;
            entity_player *Entity = (entity_player *)Selected;
            
            v2 EntitySize = EntityInfo->Size;
            v2 Size = V2(0.3f*EntitySize.X, EntitySize.Y);
            v2 Offset = V2(0.5f*EntitySize.X - 0.5f*Size.X, 0.0f);
            v2 A = Entity->P;
            v2 B = Entity->P+EntitySize-Size;
            
            if(DoButton(SizeRect(A, Size), WIDGET_ID)){
                Entity->Animation.Direction = Direction_Left;
            }
            
            if(DoButton(SizeRect(B, Size), WIDGET_ID)){
                Entity->Animation.Direction = Direction_Right;
            }
        }break;
        case ENTITY_TYPE(entity_teleporter): {
            if(Window->BeginSection("Edit teleporter", WIDGET_ID, 10, true)){
                entity_teleporter *Entity = (entity_teleporter *)Selected;
                
                Window->Text("Level:");
                Window->TextInput(Entity->Level, DEFAULT_BUFFER_SIZE, WIDGET_ID);
                Window->Text("Required level to unlock:");
                Window->TextInput(Entity->RequiredLevel, DEFAULT_BUFFER_SIZE, WIDGET_ID);
                Window->EndSection();
            }
        }break;
        case ENTITY_TYPE(entity_door): {
            if(Window->BeginSection("Edit door", WIDGET_ID, 10, true)){
                entity_door *Entity = (entity_door *)Selected;
                
                Window->Text("Required level to unlock:", 
                             Entity->RequiredLevel);
                Window->TextInput(Entity->RequiredLevel, DEFAULT_BUFFER_SIZE, WIDGET_ID);
                Window->EndSection();
            }
        }break;
    }
    
    Window->End();
}

//~ UI
void
world_editor::DoUI(){
    TIMED_FUNCTION();
    
    ui_window *Window = UIManager.BeginWindow("World Editor", OSInput.WindowSize);
    //ui_window *Window = UIManager.BeginWindow("World Editor", V2(900, OSInput.WindowSize.Y));
    //ui_window *Window = UIManager.BeginWindow("World Editor", V2(900, 800));
    
    Window->Text("Current world: %s", World->Name);
    
    if(Window->Button("Save", WIDGET_ID)){
        WorldManager.WriteWorldsToFiles();
    }
    
    Window->TextInput(NameBuffer, ArrayCount(NameBuffer), WIDGET_ID);
    
    Window->DoRow(2);
    
    if(Window->Button("Load or create world", WIDGET_ID)){
        ChangeWorld(WorldManager.GetOrCreateWorld(Strings.GetString(NameBuffer)));
    }
    
    if(Window->Button("Rename world", WIDGET_ID)){
        string WorldName = Strings.GetString(NameBuffer);
        world_data *NewWorld = WorldManager.GetWorld(WorldName);
        if(!NewWorld){
            NewWorld = WorldManager.CreateNewWorld(WorldName);
            *NewWorld = *World;
            WorldManager.RemoveWorld(World->Name);
            NewWorld->Name = WorldName;
            ChangeWorld(NewWorld);
        }
    }
    
    //~ Edit flags
    if(Window->BeginSection("Edit", WIDGET_ID, 10, true)){
        TOGGLE_FLAG(Window, "Hide art",      EditorFlags, WorldEditorFlag_HideArt);
        TOGGLE_FLAG(Window, "Hide overlays", EditorFlags, WorldEditorFlag_HideOverlays);
        Window->EndSection();
    }
    
    //~ Lighting
    if(Window->BeginSection("Lighting", WIDGET_ID, 15)){
        Window->Text("Ambient light color:");
        World->AmbientColor = Window->ColorPicker(World->AmbientColor, WIDGET_ID);
        Window->Text("Exposure: %.1f", World->Exposure);
        f32 MaxExposure = 2.0f;
        f32 ExposurePercent = World->Exposure / MaxExposure;
        ExposurePercent = Window->Slider(ExposurePercent, WIDGET_ID);
        World->Exposure =  ExposurePercent * MaxExposure;
        
        Window->EndSection();
    }
    
    //~ Groups
    if(Window->BeginSection("Groups", WIDGET_ID, 15, true)){
        if(World->EntityGroups.Count > 1){
            world_entity_group *SelectedGroup = &World->EntityGroups[SelectedGroupIndex];
            
            Window->TextInput(SelectedGroup->Name, DEFAULT_BUFFER_SIZE, WIDGET_ID);
            
            const char **GroupNames = PushArray(&TransientStorageArena, const char *, World->EntityGroups.Count);
            for(u32 I=1; I<World->EntityGroups.Count; I++){
                GroupNames[I-1] = World->EntityGroups[I].Name;
            }
            
            Assert(SelectedGroupIndex > 0);
            SelectedGroupIndex--;
            Window->DropDownMenu(GroupNames, World->EntityGroups.Count-1, &SelectedGroupIndex, WIDGET_ID);
            SelectedGroupIndex++;
        }
        
        Window->DoRow(3);
        if(Window->Button("Add group", WIDGET_ID)){
            SelectedGroupIndex = World->EntityGroups.Count;
            AllocEntityGroup(World);
        }
        
        b8 IsEditing = (EditMode == EditMode_Group);
        Window->ToggleButton("Done", "Edit group", &IsEditing, WIDGET_ID);
        if(OSInput.KeyJustDown('G', KeyFlag_Control)) IsEditing = !IsEditing;
        if(IsEditing != (EditMode == EditMode_Group)){
            if(IsEditing) EditMode = EditMode_Group;
            else          EditMode = EditMode_Entity;
        }
        
        if(Window->Button("Delete group", WIDGET_ID)){
            
        }
        
        if(World->EntityGroups.Count > 1){
            world_entity_group *SelectedGroup = &World->EntityGroups[SelectedGroupIndex];
            
            TOGGLE_FLAG(Window, "Hide group", SelectedGroup->Flags, WorldEntityEditFlag_Hide);
            
            Window->DoRow(2);
            Window->Text("Z:  %.1f", SelectedGroup->Z);
            Window->Text("Layer:  %u", SelectedGroup->Layer);
            
            Window->DoRow(2);
            if(Window->Button("+", WIDGET_ID)){
                SelectedGroup->Z += 1.0f;
            }
            if(Window->Button("+", WIDGET_ID)){
                SelectedGroup->Layer++;
            }
            
            
            Window->DoRow(2);
            if(Window->Button("-", WIDGET_ID)){
                SelectedGroup->Z -= 1.0f;
            }
            if(Window->Button("-", WIDGET_ID)){
                if(SelectedGroup->Layer > 0) SelectedGroup->Layer--;
            }
        }
        
        Window->EndSection();
    }
    
    //~ Debug
    if(Window->BeginSection("Debug", WIDGET_ID, 10, false)){
        Window->Text("ActionIndex: %u", ActionIndex);
        Window->Text("ActionMemory used: %u", ActionMemory.Used);
        Window->Text("Scale: %f", GameRenderer.CameraScale);
        
        {
            local_persist u32 SelectedIndex = 0;
            const char *Texts[] = {
                "Apple",
                "Pear",
                "Banana",
                "Cherry",
            };
            
            Window->DoRow(3);
            Window->DropDownMenu(Texts, ArrayCount(Texts), &SelectedIndex, WIDGET_ID);
            Window->Button("+", WIDGET_ID);
            Window->DropDownMenu(Texts, ArrayCount(Texts), &SelectedIndex, WIDGET_ID);
        }
        
        Window->EndSection();
    }
    
    Window->End();
    
}

//~ Selecting

inline b8
world_editor::IsSelectionDisabled(entity *Entity, os_key_flags KeyFlags){
    b8 Result = !((Selected == Entity) || 
                  OSInput.TestModifier(KeyFlags) || 
                  (EditMode != EditMode_Entity));
    Result = Result || !(OSInput.TestModifier(KeyFlags) || 
                         (EditMode != EditMode_Group));
    return(Result);
}

internal inline os_key_flags
SelectionKeyFlags(b8 Special){
    os_key_flags Result = Special ? SPECIAL_SELECT_MODIFIER : KeyFlag_None;
    return(Result);
}

inline b8
world_editor::DoDragEntity(entity *Entity, b8 Special){
    b8 Result = false;
    os_key_flags KeyFlags = SelectionKeyFlags(Special);
    u64 ID = (u64)Entity;
    
    rect R = Entity->Bounds+Entity->P;
    
    if(EditMode == EditMode_Entity){
        switch(EditorDraggableElement(&UIManager, ID, R, Entity->P, -2, ScaledItem(1), 
                                      KeyFlags, IsSelectionDisabled(Entity, KeyFlags))){
            case UIBehavior_Activate: {
                Result = true;
                
                v2 Offset = GameRenderer.ScreenToWorld(UIManager.ActiveElement.Offset, ScaledItem(0));
                Entity->P = SnapToGrid(MouseP + Offset, Grid);
                
                EditModeEntity(Entity);
            }break;
        }
        
        R = Entity->Bounds+Entity->P;
        if(!(EditorFlags & WorldEditorFlag_HideOverlays)){
            RenderRectOutline(R, -0.3f, GetEditorColor(ID, (Selected == Entity), false), GameItem(1));
        }
        
        if(Selected == Entity) Result = false;
        
    }else if(EditMode == EditMode_Group){
        NOT_IMPLEMENTED_YET;
        
#if 0  
        switch(EditorButtonElement(&UIManager, ID, R, MouseButton_Left, -2, ScaledItem(1), 
                                   KeyFlags, IsSelectionDisabled(Entity, KeyFlags))){
            case UIBehavior_Activate: {
                if(Entity->Base.GroupIndex == SelectedGroupIndex) Entity->Base.GroupIndex = 0; 
                else                                              Entity->Base.GroupIndex = SelectedGroupIndex;
            }break;
        }
        
        RenderRectOutline(R, -0.3f, GetEditorColor(ID, (Entity->Base.GroupIndex == SelectedGroupIndex), false), GameItem(1));
#endif
    }else if(((EditMode == EditMode_Tilemap) || (EditMode == EditMode_TilemapTemporary)) &&
             !(EditorFlags & WorldEditorFlag_HideOverlays)){
        RenderRectOutline(R, -0.3f, GetEditorColor(ID, (Selected == Entity), false), GameItem(1));
    }
    
    return(Result);
}

inline b8
world_editor::DoDeleteEntity(entity *Entity, b8 Special){
    b8 Result = false;
    os_key_flags KeyFlags = SelectionKeyFlags(Special);
    u64 ID = WIDGET_ID_CHILD(WIDGET_ID, (u64)Entity);
    rect R = Entity->Bounds + Entity->P;
    
    if(EditMode != EditMode_Entity) return(Result);
    
    switch(EditorButtonElement(&UIManager, ID, R, MouseButton_Right, -1, ScaledItem(1), 
                               KeyFlags, IsSelectionDisabled(Entity, KeyFlags))){
        case UIBehavior_Activate: Result = true; break;
    }
    if((Selected == Entity) && OSInput.KeyJustDown(KeyCode_Delete)){ Result = true; }
    
    return(Result);
}

//~ Input

void
world_editor::ProcessInput(){
    os_event Event;
    while(PollEvents(&Event)){
        if(UIManager.ProcessEvent(&Event)) continue;
        ProcessDefaultEvent(&Event);
    }
}

void
world_editor::ProcessHotKeys(){
    if(OSInput.KeyJustDown(KeyCode_Tab)) UIManager.HideWindows = !UIManager.HideWindows; 
    if(OSInput.KeyJustDown('E', KeyFlag_Control)) ToggleWorldEditor(); 
    if(OSInput.KeyJustDown('S', KeyFlag_Control)) WorldManager.WriteWorldsToFiles();
    
    if(OSInput.KeyJustDown('A')) ToggleFlag(&EditorFlags, WorldEditorFlag_HideArt); 
    if(OSInput.KeyJustDown('O')) ToggleFlag(&EditorFlags, WorldEditorFlag_HideOverlays); 
    if(OSInput.KeyJustDown('Q')) ToggleFlag(&EditorFlags, WorldEditorFlag_UnhideHiddenGroups); 
    
    if(OSInput.KeyJustDown('F', KeyFlag_Control)){
#if 0
        world_entity_player *Player = 0;
        for(u32 I=0; I<World->Entities.Count; I++){
            if(World->Entities[I].Type == EntityType_Player){
                Player = &World->Entities[I].Player;
                break;
            }
        }
        Assert(Player);
        
        asset_entity *EntityInfo = AssetSystem.GetEntity(Player->Asset);
        v2 Center = Player->P + 0.5f*EntityInfo->Size;
        GameRenderer.SetCameraTarget(Center);
#endif
    }
    
    //if(OSInput.KeyJustDown('Z', KeyFlag_Control)) Undo();
    //if(OSInput.KeyJustDown('Y', KeyFlag_Control)) Redo();
    //if(OSInput.KeyJustDown('B')) ClearActionHistory();
    
    if(EditMode != EditMode_Entity) return;
    if(OSInput.KeyJustDown('1')) EditThing = EditThing_None;          
    if(OSInput.KeyJustDown('2')) EditThing = EditThing_Tilemap;
    if(OSInput.KeyJustDown('3')) EditThing = EditThing_Art;
    if(OSInput.KeyJustDown('4')) EditThing = EditThing_Teleporter;
    if(OSInput.KeyJustDown('5')) EditThing = EditThing_Door;
    
}

//~
void
world_editor::UpdateEditorEntities(){
    entity_manager *Manager = &World->Manager;
    
    FOR_EACH_ENTITY(Manager){
        entity *Entity = It.Item;
        if(DoDragEntity(Entity, false)){
        }
        
        if(DoDeleteEntity(Entity, false)){
            
#define ENTITY_TYPE_(TypeName, ...)                       \
else if(ENTITY_TYPE(TypeName) == It.CurrentArray) {   \
Manager->RemoveEntityByIndex(ENTITY_TYPE(TypeName), It.Index); \
if(Selected == Entity) EditModeEntity(0);         \
}
            
            if(ENTITY_TYPE(entity_player) == It.CurrentArray) {
            } ENTITY_TYPES else{
            }
            
#undef ENTITY_TYPE_
            
        }
    }
    
    FOR_ENTITY_TYPE(Manager, entity_tilemap){
        entity_tilemap *Entity = It.Item;
        
        Entity->TilemapData = MakeTilemapData(&TransientStorageArena, 
                                              Entity->Width, Entity->Height);
        
        asset_tilemap *Asset = AssetSystem.GetTilemap(Entity->Asset);
        CalculateTilemapIndices(Asset, Entity->Tiles, &Entity->TilemapData);
    }
    
}

//~ 

void
world_editor::Initialize(){
    InitializeArray(&Actions, 128);
    {
        umw Size = Megabytes(512);
        void *Memory = AllocateVirtualMemory(Size);
        Assert(Memory);
        InitializeArena(&ActionMemory, Memory, Size);
    }
}

void
world_editor::ChangeWorld(world_data *World_){
    World = World_;
    ClearActionHistory();
    const char *Name = Strings.GetString(World->Name);
    CopyCString(NameBuffer, Name, DEFAULT_BUFFER_SIZE);
}

void
world_editor::UpdateAndRender(){
    TIMED_FUNCTION();
    if(!World){
        ChangeWorld(CurrentWorld);
        EntityInfoToAdd = Strings.GetString("snail");
        Grid.Offset = V2(0);
        Grid.CellSize = TILE_SIZE;
        EditModeEntity(0);
    }
    
    //~ Prep
    ProcessInput();
    
    GameRenderer.NewFrame(&TransientStorageArena, OSInput.WindowSize, MakeColor(0.30f, 0.55f, 0.70f));
    GameRenderer.CalculateCameraBounds(World);
    GameRenderer.SetCameraSettings(0.3f/OSInput.dTime);
    GameRenderer.SetLightingConditions(HSBToRGB(World->AmbientColor), World->Exposure);
    
    LastMouseP = MouseP;
    MouseP = GameRenderer.ScreenToWorld(OSInput.MouseP, ScaledItem(1));
    CursorP = SnapToGrid(MouseP-0.5*TILE_SIZE, Grid);
    
    if(SelectedGroupIndex == 0) SelectedGroupIndex = 1;
    
    //~ Dragging
    if(UIManager.DoClickElement(WIDGET_ID, MouseButton_Middle, false, -2)){
        v2 Difference = OSInput.MouseP-OSInput.LastMouseP;
        v2 Movement = GameRenderer.ScreenToWorld(Difference, ScaledItem(0));
        GameRenderer.MoveCamera(-Movement);
    }
    
    //~ 
    ProcessHotKeys();
    DoUI(); 
    DoSelectedThingUI();
    MaybeEditTilemap();
    
    //~ Editing
    if(EditMode == EditMode_Entity){
        if(UIManager.DoScrollElement(WIDGET_ID, -1, KeyFlag_None)){
            s32 Range = 100;
            s32 Scroll = UIManager.ActiveElement.Scroll;
            if(Scroll > Range){
                EditThing = WORLD_EDITOR_FORWARD_EDIT_MODE_TABLE[EditThing];
                Assert(EditThing != EditThing_TOTAL);
            }else if(Scroll < -Range){
                EditThing = WORLD_EDITOR_REVERSE_EDIT_MODE_TABLE[EditThing];
                Assert(EditThing != EditThing_TOTAL);
            }
        }
        
        switch(EditThing){
            case EditThing_None: break;
            case EditThing_Tilemap: {
                DoEditThingTilemap();
            }break;
            case EditThing_Teleporter: {
                DoEditThingTeleporter();
            }break;
            case EditThing_Door: {
                DoEditThingDoor();
            }break;
            case EditThing_Art: {
                DoEditThingArt();
            }break;
            case EditThing_Match: {
                DoEditThingMatch();
            }break;
            default: { INVALID_CODE_PATH }break;
        }
    }
    
    //~
    DoEditModeGroup();
    UpdateEditorEntities();
    
    //~ Rendering
    BEGIN_TIMED_BLOCK(RenderWorldEditor);
    
    World->Manager.RenderEntities();
    
#if 0 
    if(EntityToDelete == Entity){
        ui_button_state *State = FindOrCreateInHashTablePtr(&UIManager.ButtonStates, (u64)Entity);
        
        if(!(DeleteFlags & EditorDeleteFlag_DontCommit)){
            editor_action *Action = MakeAction(EditorAction_DeleteEntity);
            Action->Entity = CopyEntity(&ActionMemory, Entity);
        }
        
        EditModeEntity(0);
        CleanupEntity(Entity);
        ArrayOrderedRemove(&World->Entities, I);
        I--; // Repeat the last iteration because an item was removed
        State->T = 0.0f;
        State->ActiveT = 0.0f;
        EntityToDelete = 0;
        
        DeleteFlags = 0;
    }
#endif
    
    END_TIMED_BLOCK();
}
