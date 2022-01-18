
//~ 
internal inline puzzle_update_node *
PuzzleGetUpdateNode(puzzle_state *State){
    if(!State->FirstFreeUpdate){
        State->FirstFreeUpdate = PushStruct(&State->Memory, puzzle_update_node);
    }
    
    puzzle_update_node *Result = State->FirstFreeUpdate;
    *Result = {};
    State->FirstFreeUpdate = Result->Next;
    
    if(!State->FirstUpdate){
        State->FirstUpdate = Result;
        State->LastUpdate = State->FirstUpdate;
    }else{
        State->LastUpdate->Next = Result;
        State->LastUpdate = Result;
    }
    
    return Result;
}

//~ Helpers
internal inline b8
PuzzleIsCellTypeAction(puzzle_cell_type Type){
    switch(Type){
        case PuzzleCell_SlideUp:    return true;
        case PuzzleCell_SlideDown:  return true;
        case PuzzleCell_SlideLeft:  return true;
        case PuzzleCell_SlideRight: return true;
        default: return false;
    }
}

internal inline u32
PuzzleBoardI(puzzle_board *Board, v2s CellP){
    u32 Result = CellP.X + CellP.Y*Board->N;
    return Result;
}

internal inline puzzle_cell
PuzzleBoardMark(puzzle_board *Board, v2s CellP){
    u32 I = PuzzleBoardI(Board, CellP);
    puzzle_cell Result = Board->Board[I];
    return Result;
}

internal inline v3s
PuzzleBoardCellP(puzzle_board *Board, u32 I){
    v3s Result;
    Result.Z = I / (Board->N*Board->N);
    I %= Board->N*Board->N;
    Result.Y = I / Board->N;
    I %= Board->N;
    Result.X = I;
    return Result;
}

internal inline v2
PuzzleBoardCellPV2(puzzle_state *State, puzzle_board *Board, v2s CellP, v2 P){
    v2 Result = P+State->CellSize*V2(CellP);
    return Result;
}


internal inline ttt_position
PuzzleBoardPosition(puzzle_state *State, puzzle_board *Board, v2 Point){
    ttt_position Result = {};
    
    u32 N = Board->N;
    
    v2 P = State->P;
    f32 CellSize = State->CellSize;
    f32 Advance = CellSize*N;
    
    rect BoardRect = SizeRect(V2(0), V2(N*CellSize));
    v2 BoardP = P;
    v2s CellP = V2S(0);
    if(IsPointInRect(Point, BoardRect+BoardP)){
        v2 CursorP = Point-BoardP;
        CursorP /= CellSize;
        CursorP = V2(Floor(CursorP.X), Floor(CursorP.Y));
        CellP = V2S(CursorP);
        
        Result.IsValid = true;
        Result.VisualP = PuzzleBoardCellPV2(State, Board, CellP, P);
        Result.I = PuzzleBoardI(Board, CellP);
        Result.XY = CellP;
    }
    
    return Result;
}

internal puzzle_mark
PuzzleSwitchMark(puzzle_mark Mark){
    if(Mark == PuzzleMark_X)      return PuzzleMark_O;
    else if(Mark == PuzzleMark_O) return PuzzleMark_X;
    else INVALID_CODE_PATH;
    return PuzzleMark_None;
}

internal inline ttt_position
PuzzleShiftPosition(puzzle_board *Board, ttt_position CellP, v2s Delta){
    CellP.XY += Delta;
    CellP.IsValid = true;
    
    if(CellP.X < 0) CellP.IsValid = false;
    if(CellP.Y < 0) CellP.IsValid = false;
    if(CellP.X >= (s32)Board->N) CellP.IsValid = false;
    if(CellP.Y >= (s32)Board->N) CellP.IsValid = false;
    
    CellP.I += Delta.X + Board->N*Delta.Y;
    if(CellP.IsValid) Assert(CellP.I < Board->N*Board->N);
    
    return CellP;
}

internal void
PuzzlePlaceMark(puzzle_state *State, puzzle_board *Board, ttt_position CellP, puzzle_mark Mark){
    puzzle_cell *Cell = &Board->Board[CellP.I];
    if(Cell->Mark) return;
    Cell->Mark = Mark;
    
    if(Cell->Type){
        puzzle_update_node *Node = PuzzleGetUpdateNode(State);
        Node->CellP = CellP;
        Node->Mark = PuzzleSwitchMark(Mark);
    }
}

internal void
PuzzleExecuteUpdates(puzzle_state *State, puzzle_board *Board){
    puzzle_update_node *Executing = State->FirstUpdate;
    State->FirstUpdate = 0;
    
    while(Executing){
        ttt_position CellP = Executing->CellP;
        
        puzzle_cell *Cell = &Board->Board[CellP.I];
        v2s Delta = V2S(0);
        if(Cell->Type == PuzzleCell_SlideUp){
            Delta.Y = 1;
        }else if(Cell->Type == PuzzleCell_SlideDown){
            Delta.Y = -1;
        }else if(Cell->Type == PuzzleCell_SlideLeft){
            Delta.X = -1;
        }else if(Cell->Type == PuzzleCell_SlideRight){
            Delta.X = 1;
        }
        
        while(true){
            ttt_position NewCellP = PuzzleShiftPosition(Board, CellP, Delta);
            if(!NewCellP.IsValid) break;
            if(Board->Board[NewCellP.I].Mark) break;
            CellP = NewCellP;
        }
        
        PuzzlePlaceMark(State, Board, CellP, Executing->Mark);
        
        puzzle_update_node *Next = Executing->Next;
        Executing->Next = State->FirstFreeUpdate;
        State->FirstFreeUpdate = Executing;
        Executing = Next;
    }
}

//~ 2D
internal inline puzzle_board
MakePuzzleBoard(memory_arena *Arena, u32 N){
    puzzle_board Result = {};
    Result.N = N;
    Result.Size = N*N;
    Result.Board = PushArray(Arena, puzzle_cell, Result.Size);
    
    return Result;
}

internal inline void
PuzzleBoardRender(puzzle_state *State, puzzle_board *Board, v2 P, f32 Padding, f32 Z){
    u32 N = Board->N;
    Assert(N > 0);
    
    asset_tilemap *Tilemap = State->Tilemap;
    RenderTilemap(Tilemap, &State->Data, P, Z+0.1f, 0);
    
    f32 CellSize = State->CellSize;
    f32 Advance = CellSize;
    
    {
        u32 X = 0;
        u32 Y = 0;
        for(u32 I=0; I<N*N; I++){
            v2 CellP = P+CellSize*V2((f32)X, (f32)Y);
            v2 StringP = GameRenderer.WorldToScreen(CellP+V2(0.5f*CellSize), ScaledItem(0));
            
            if(Board->Board[I].Type == PuzzleCell_SlideUp){
                RenderCenteredString(&DebugFont, BLACK, StringP, 0.0f, "UP");
            }else if(Board->Board[I].Type == PuzzleCell_SlideDown){
                RenderCenteredString(&DebugFont, BLACK, StringP, 0.0f, "DOWN");
            }else if(Board->Board[I].Type == PuzzleCell_SlideLeft){
                RenderCenteredString(&DebugFont, BLACK, StringP, 0.0f, "LEFT");
            }else if(Board->Board[I].Type == PuzzleCell_SlideRight){
                RenderCenteredString(&DebugFont, BLACK, StringP, 0.0f, "RIGHT");
            }else if(Board->Board[I].Type == PuzzleCell_NeededToWinVertical){
                RenderCenteredString(&DebugFont, BLACK, StringP, 0.0f, "Win vertical");
                StringP.Y -= DebugFont.Size;
                RenderCenteredString(&DebugFont, BLACK, StringP, 0.0f, "%d", Board->Board[I].NeededInARow);
            }else if(Board->Board[I].Type == PuzzleCell_NeededToWinHorizontal){
                RenderCenteredString(&DebugFont, BLACK, StringP, 0.0f, "Win horizontal");
                StringP.Y -= DebugFont.Size;
                RenderCenteredString(&DebugFont, BLACK, StringP, 0.0f, "%d", Board->Board[I].NeededInARow);
            }else if(Board->Board[I].Type == PuzzleCell_NeededToWinAny){
                RenderCenteredString(&DebugFont, BLACK, StringP, 0.0f, "Win any");
                StringP.Y -= DebugFont.Size;
                RenderCenteredString(&DebugFont, BLACK, StringP, 0.0f, "%d", Board->Board[I].NeededInARow);
            }
            
            if(Board->Board[I].Mark == PuzzleMark_X){
                RenderSpriteSheetAnimationFrame(State->Marks, CellP, Z-0.1f, 0, 1, I);
            }else if(Board->Board[I].Mark == PuzzleMark_O){
                RenderSpriteSheetAnimationFrame(State->Marks, CellP, Z-0.1f, 0, 2, I);
            }else if(Board->Board[I].Mark == PuzzleMark_E){
                RenderSpriteSheetAnimationFrame(State->Marks, CellP, Z-0.1f, 0, 8, I);
            }
            
            X++;
            if(X >= N){
                X = 0;
                Y++;
            }
        }
    }
}

internal inline b8
PuzzleBoardTestWin(puzzle_state *State, puzzle_board *Board){
    b8 Result = false;
    
    
    
    return Result;
}

//~

internal void
UpdateAndRenderPuzzle(){
    b8 LeaveKeyboardMode = true;
    
    GameRenderer.NewFrame(&TransientStorageArena, OSInput.WindowSize, PINK);
    GameRenderer.SetLightingConditions(WHITE, 1.0f);
    GameRenderer.SetCameraSettings(0.5f);
    
    RenderRect(SizeRect(V2(0), GameRenderer.ScreenToWorld(OSInput.WindowSize, ScaledItem(0))), 
               5.5f, MakeColor(0.30f, 0.55f, 0.70f), GameItem(0));
    
    local_persist puzzle_board Board = {};
    if(!PuzzleState.Turn){
        u32 N = 5;
        
        Board = MakePuzzleBoard(&PermanentStorageArena, N);
        Board.Board[ 0].Type = PuzzleCell_SlideUp;
        Board.Board[20].Type = PuzzleCell_SlideRight;
        Board.Board[24].Type = PuzzleCell_SlideDown;
        Board.Board[ 4].Type = PuzzleCell_SlideLeft;
        Board.Board[12].Type = PuzzleCell_NeededToWinVertical;
        
        PuzzleState.Turn = PuzzleMark_X;
        PuzzleState.Tilemap = AssetSystem.GetTilemap(String("ttt_board_fancy"));
        PuzzleState.Marks = AssetSystem.GetSpriteSheet(String("ttt_marks_fancy"));
        
        PuzzleState.Data = MakeTilemapData(&PermanentStorageArena, N, N);
        tilemap_tile *Tiles = PushArray(&TransientStorageArena, tilemap_tile, N*N);
        for(u32 I=0; I<N*N; I++) Tiles[I].Type = TileType_Tile;
        CalculateTilemapIndices(PuzzleState.Tilemap, Tiles, &PuzzleState.Data);
        
        PuzzleState.CellSize = PuzzleState.Tilemap->TileSize.X;
    }
    
    puzzle_state *State = &PuzzleState;
    
    u32 N = Board.N;
    
    v2 P = State->P;
    f32 CellSize = State->CellSize;
    f32 Padding = State->Padding;
    f32 Advance = CellSize*N;
    
    b8 PlayerJustAdded = false;
    if(State->Turn == PuzzleMark_X){
        v2 MouseP = GameRenderer.ScreenToWorld(OSInput.MouseP, ScaledItem(0));
        ttt_position CellP = PuzzleBoardPosition(State, &Board, MouseP);
        
        puzzle_mark *Mark = 0;
        if(CellP.IsValid) Mark = &Board.Board[CellP.I].Mark;
        
        if(CellP.IsValid && !*Mark){
            v2 VisualP = CellP.VisualP;
            RenderSpriteSheetAnimationFrame(State->Marks, VisualP, 5.0f, 0, 3, 0);
            
            if(OSInput.MouseJustDown(MouseButton_Left, KeyFlag_Any) ||
               OSInput.KeyJustDown(KeyCode_Space, KeyFlag_Any)){
                PuzzlePlaceMark(State, &Board, CellP, PuzzleMark_X);
                State->Turn = PuzzleMark_O;
                PlayerJustAdded = true;
                
                asset_sound_effect *Asset = AssetSystem.GetSoundEffect(String("ttt_place_mark"));
                AudioMixer.PlaySound(Asset);
            }
        }
    }else if(State->Turn == PuzzleMark_O){
        PuzzleExecuteUpdates(State, &Board);
        State->Turn = PuzzleMark_X;
    }
    
    PuzzleBoardRender(State, &Board, State->P, Padding, 5.0f);
}