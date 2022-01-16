
//~ Helpers
internal inline u32
GameBoardI(game_board *Board, v3s CellP){
    u32 Result = CellP.X + CellP.Y*Board->N + CellP.Z*Board->N*Board->N;
    return Result;
}

internal inline game_board_cell
GameBoardMark(game_board *Board, v3s CellP){
    u32 I = GameBoardI(Board, CellP);
    game_board_cell Result = Board->Board[I];
    return Result;
}

internal inline v3s
GameBoardCellP(game_board *Board, u32 I){
    v3s Result;
    Result.Z = I / (Board->N*Board->N);
    I %= Board->N*Board->N;
    Result.Y = I / Board->N;
    I %= Board->N;
    Result.X = I;
    return Result;
}

internal inline v2
GameBoardCellPV2(tic_tac_toe_state *State, game_board *Board, v3s CellP, v2 P){
    v2 Result = P+State->CellSize*V2(CellP.XY);
    Result.X += State->CellSize*Board->N*CellP.Z;
    Result.Y -= State->ZYAdvance*CellP.Z;
    return Result;
}


internal inline ttt_position
GameBoardPosition(tic_tac_toe_state *State, game_board *Board, v2 Point){
    ttt_position Result = {};
    
    u32 N = Board->N;
    
    v2 P = State->P;
    f32 CellSize = State->CellSize;
    f32 Advance = CellSize*N;
    
    rect BoardRect = SizeRect(V2(0), V2(N*CellSize));
    v2 BoardP = P;
    v3s CellP = V3S(0);
    if(IsPointInRect(Point, BoardRect+BoardP)){
        v2 CursorP = Point-BoardP;
        CursorP /= CellSize;
        CursorP = V2(Floor(CursorP.X), Floor(CursorP.Y));
        CellP.XY = V2S(CursorP);
        
        Result.IsValid = true;
        Result.VisualP = GameBoardCellPV2(State, Board, CellP, P);
        Result.I = GameBoardI(Board, CellP);
        Result.XY = CellP.XY;
        
        BoardP.X += Advance;
        BoardP.Y -= State->ZYAdvance;
    }
    
    return Result;
}


//~ 2D
internal inline game_board
MakeGameBoard(memory_arena *Arena, u32 N){
    game_board Result = {};
    Result.N = N;
    Result.Size = N*N;
    Result.Board = PushArray(Arena, game_board_cell, Result.Size);
    
    return Result;
}

internal inline void
GameBoardRender(tic_tac_toe_state *State, game_board *Board, v2 P, f32 Padding, f32 Z){
    u32 N = Board->N;
    Assert(N > 0);
    
    asset_tilemap *Tilemap = State->Tilemap;
    RenderTilemap(Tilemap, &State->Data, P, Z+0.1f, 0);
    
    f32 CellSize = Tilemap->TileSize.X;
    f32 Advance = CellSize;
    
    {
        u32 X = 0;
        u32 Y = 0;
        for(u32 I=0; I<N*N; I++){
            if(Board->Board[I].Mark == TTTMark_X){
                v2 CellP = P+CellSize*V2((f32)X, (f32)Y);
                RenderSpriteSheetAnimationFrame(State->Marks, CellP, Z, 0, 1, I);
            }else if(Board->Board[I].Mark == TTTMark_O){
                v2 CellP = P+CellSize*V2((f32)X, (f32)Y);
                RenderSpriteSheetAnimationFrame(State->Marks, CellP, Z, 0, 2, I);
            }else if(Board->Board[I].Mark == TTTMark_E){
                v2 CellP = P+CellSize*V2((f32)X, (f32)Y);
                RenderSpriteSheetAnimationFrame(State->Marks, CellP, Z, 0, 3, I);
            }
            
            X++;
            if(X >= N){
                X = 0;
                Y++;
            }
        }
    }
}

internal void
UpdateAndRenderPuzzle(){
    b8 LeaveKeyboardMode = false;
    os_event Event;
    while(PollEvents(&Event)){
        if(UIManager.ProcessEvent(&Event)) continue;
        ProcessDefaultEvent(&Event);
        switch(Event.Kind){
            case OSEventKind_MouseMove: {
                LeaveKeyboardMode = true;
            }break;
        }
    }
    
    GameRenderer.NewFrame(&TransientStorageArena, OSInput.WindowSize, PINK);
    GameRenderer.SetLightingConditions(WHITE, 1.0f);
    GameRenderer.SetCameraSettings(0.5f);
    
    RenderRect(SizeRect(V2(0), GameRenderer.ScreenToWorld(OSInput.WindowSize, ScaledItem(0))), 
               5.5f, MakeColor(0.30f, 0.55f, 0.70f), GameItem(0));
    
    local_persist game_board Board = {};
    if(!TicTacToeState.Turn){
        u32 N = 4;
        
        Board = MakeGameBoard(&PermanentStorageArena, N);
        TicTacToeState.WinningIndices = PushArray(&PermanentStorageArena, u32, N);
        TicTacToeState.Turn = TTTMark_Player;
        TicTacToeState.Tilemap = AssetSystem.GetTilemap(String("ttt_board_fancy"));
        TicTacToeState.Marks = AssetSystem.GetSpriteSheet(String("ttt_marks_fancy"));
        TicTacToeState.CursorP = MakeTTTPosition(TicTacToeState.P, V2S(0), 0);
        TicTacToeState.Cursor1P = TicTacToeState.P;
        TicTacToeState.Cursor2P = TicTacToeState.P;
        
        TicTacToeState.Data = MakeTilemapData(&PermanentStorageArena, N, N);
        tilemap_tile *Tiles = PushArray(&TransientStorageArena, tilemap_tile, N*N);
        for(u32 I=0; I<N*N; I++) Tiles[I].Type = TileType_Tile;
        CalculateTilemapIndices(TicTacToeState.Tilemap, Tiles, &TicTacToeState.Data);
        
        TicTacToeState.CellSize = TicTacToeState.Tilemap->TileSize.X;
        TicTacToeState.ZYAdvance = 0.5f*TicTacToeState.CellSize;
    }
    
    tic_tac_toe_state *State = &TicTacToeState;
    
    u32 N = Board.N;
    
    v2 P = State->P;
    f32 CellSize = State->CellSize;
    f32 Padding = State->Padding;
    f32 Advance = CellSize*N;
    
    b8 PlayerJustAdded = false;
    if(!(State->Flags & TTTStateFlag_HasAWinner)){
        if(OSInput.KeyRepeat(KeyCode_Left, KeyFlag_Any) && (0 < State->CursorP.X)){
            State->CursorP.VisualP.X -= CellSize;
            State->CursorP.X--;
            State->CursorP.I--;
            State->Flags |= TTTStateFlag_KeyboardMode|TTTStateFlag_PlayerHasMoved;
        }
        if(OSInput.KeyRepeat(KeyCode_Right, KeyFlag_Any) && (State->CursorP.X < (s32)N-1)){
            State->CursorP.VisualP.X += CellSize;
            State->CursorP.X++;
            State->CursorP.I++;
            State->Flags |= TTTStateFlag_KeyboardMode|TTTStateFlag_PlayerHasMoved;
        }
        if(OSInput.KeyRepeat(KeyCode_Down, KeyFlag_Any) && (0 < State->CursorP.Y)){
            State->CursorP.VisualP.Y -= CellSize;
            State->CursorP.Y--;
            State->CursorP.I -= N;
            State->Flags |= TTTStateFlag_KeyboardMode|TTTStateFlag_PlayerHasMoved;
        }
        if(OSInput.KeyRepeat(KeyCode_Up, KeyFlag_Any) && (State->CursorP.Y < (s32)N-1)){
            State->CursorP.VisualP.Y += CellSize;
            State->CursorP.Y++;
            State->CursorP.I += N;
            State->Flags |= TTTStateFlag_KeyboardMode|TTTStateFlag_PlayerHasMoved;
        }
        if(OSInput.KeyRepeat('S', KeyFlag_Any) && ((s32)State->CursorP.I < (s32)Board.Size-(s32)(N*N))){
            State->CursorP.VisualP.X += Advance;
            State->CursorP.VisualP.Y -= State->ZYAdvance;
            State->CursorP.I += N*N;
            State->Flags |= TTTStateFlag_KeyboardMode|TTTStateFlag_PlayerHasMoved;
        }
        if(OSInput.KeyRepeat('W', KeyFlag_Any) && (N*N <= State->CursorP.I)){
            State->CursorP.VisualP.X -= Advance;
            State->CursorP.VisualP.Y += State->ZYAdvance;
            State->CursorP.I -= N*N;
            State->Flags |= TTTStateFlag_KeyboardMode|TTTStateFlag_PlayerHasMoved;
        }
        
        ttt_position CellP = State->CursorP;
        if(!(State->Flags & TTTStateFlag_KeyboardMode)){
            v2 MouseP = GameRenderer.ScreenToWorld(OSInput.MouseP, ScaledItem(0));
            CellP = GameBoardPosition(State, &Board, MouseP);
#if 0
            if(CellP.IsValid && Board.Board[CellP.I].Mark){
                TTTBoardMaybeShiftPosition(State, Board.Board, Board.N, &CellP, MouseP);
            }
#endif
        }
        
        if(CellP.IsValid && !Board.Board[CellP.I].Mark) State->CursorP = CellP;
        ttt_mark *Mark = &Board.Board[State->CursorP.I].Mark;
        
        if(!*Mark){
            if(OSInput.MouseJustDown(MouseButton_Left, KeyFlag_Any) ||
               OSInput.KeyJustDown(KeyCode_Space, KeyFlag_Any)){
                *Mark = TTTMark_Player;
                State->Turn = TTTMark_Computer;
                State->Flags &= ~TTTStateFlag_PlayerHasMoved;
                PlayerJustAdded = true;
                
                asset_sound_effect *Asset = AssetSystem.GetSoundEffect(String("ttt_place_mark"));
                AudioMixer.PlaySound(Asset);
            }
        }
        
        if(LeaveKeyboardMode) State->Flags &= ~TTTStateFlag_KeyboardMode;
    }
    
    ttt_mark Winner = TTTMark_None;
    GameBoardRender(State, &Board, State->P, Padding, 5.0f);
    
    if(!Board.Board[State->CursorP.I].Mark ||
       ((State->Flags & TTTStateFlag_KeyboardMode) && 
        (State->Flags & TTTStateFlag_PlayerHasMoved))){
        State->Cursor1P += State->Cursor1MoveFactor*(State->CursorP.VisualP-State->Cursor1P);
        State->Cursor2P += State->Cursor2MoveFactor*(State->CursorP.VisualP-State->Cursor2P);
        
        RenderSpriteSheetAnimationFrame(State->Marks, State->Cursor1P, 5.0f, 0, 3, 0);
        GameRenderer.AddLight(State->Cursor2P+V2(0.5f*CellSize), MakeColor(0.0f, 0.6f, 1.0f), 
                              0.7f, CellSize, GameItem(0));
    }
    
    if(State->Flags & TTTStateFlag_ComputerHasMoved){
        State->ComputerP += State->ComputerMoveFactor*(State->TargetComputerP-State->ComputerP);
        GameRenderer.AddLight(State->ComputerP+V2(0.5f*CellSize), MakeColor(1.0f, 0.3f, 0.3f), 
                              0.7f, CellSize, GameItem(0));
    }
}