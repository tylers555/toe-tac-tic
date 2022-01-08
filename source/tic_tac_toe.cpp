
//~ 
#define XLoop for(u32 X=0; X<N; X++)
#define YLoop for(u32 Y=0; Y<N*N; Y+=N)
#define ZLoop for(u32 Z=0; Z<N*N*N; Z+=N*N)
#define WLoop for(u32 W=0; W<N*N*N*N; W+=N*N*N)

//~ 3D helpers
internal inline u32
TTT3DBoardI(ttt_board *Board, v3s CellP){
 u32 Result = CellP.X + CellP.Y*Board->N + CellP.Z*Board->N*Board->N;
 return Result;
}

internal inline ttt_mark
TTT3DBoardMark(ttt_board *Board, v3s CellP){
 u32 I = TTT3DBoardI(Board, CellP);
 ttt_mark Result = Board->Board[I];
 return Result;
}

internal inline v3s
TTT3DBoardCellP(ttt_board *Board, u32 I){
 v3s Result;
 Result.Z = I / (Board->N*Board->N);
 I %= Board->N*Board->N;
 Result.Y = I / Board->N;
 I %= Board->N;
 Result.X = I;
 return Result;
}

internal inline v2
TTT3DBoardCellPV2(tic_tac_toe_state *State, ttt_board *Board, v3s CellP, v2 P){
 v2 Result = P+State->CellSize*V2(CellP.XY);
 Result.X += State->CellSize*Board->N*CellP.Z;
 Result.Y -= State->ZYAdvance*CellP.Z;
 return Result;
}

//~ 4D helpers
internal inline u32
TTT4DBoardI(ttt_board *Board, v4s CellP){
 u32 Result = CellP.X + CellP.Y*Board->N + CellP.Z*Board->N*Board->N + CellP.W*Board->N*Board->N*Board->N;
 return Result;
}

internal inline ttt_mark
TTT4DBoardMark(ttt_board *Board, v4s CellP){
 u32 I = TTT4DBoardI(Board, CellP);
 ttt_mark Result = Board->Board[I];
 return Result;
}

internal inline v4s
TTT4DBoardCellP(ttt_board *Board, u32 I){
 v4s Result;
 Result.W = I / (Board->N*Board->N*Board->N);
 I %= Board->N*Board->N*Board->N;
 Result.Z = I / (Board->N*Board->N);
 I %= Board->N*Board->N;
 Result.Y = I / Board->N;
 I %= Board->N;
 Result.X = I;
 return Result;
}

internal inline v2
TTT4DBoardCellPV2(ttt_board *Board, v4s CellP, v2 P, f32 Padding, f32 CellSize){
 v2 Result = P+CellSize*V2(CellP.XY);
 f32 Advance = CellSize*Board->N + Padding;
 Result.X += Advance*CellP.Z;
 Result.Y += Advance*CellP.W;
 return Result;
}

//~ 
internal inline void
TTTBoardMaybeShiftPosition(tic_tac_toe_state *State, ttt_mark *Board, u32 N,
                           ttt_position *CellP, v2 Point){
 ttt_position Result = {};
 f32 CellSize = State->Tilemap->TileSize.X;
 
 Point -= CellP->VisualP;
 local_constant f32 MinOverlap = 0.15f;
 local_constant f32 MaxOverlap = 1.0f-MinOverlap;
 
 u32 OldI = CellP->I;
 if((Point.X < MinOverlap*CellSize) && (CellP->X > 0) && 
    (!Board[OldI-1])){
  CellP->I--;
  CellP->VisualP.X -= CellSize;
 }else if((Point.X > MaxOverlap*CellSize) && (CellP->X < (s32)N-1) &&
          (!Board[OldI+1])){
  CellP->I++;
  CellP->VisualP.X += CellSize;
 }
 
 if((Point.Y < MinOverlap*CellSize) && (CellP->Y > 0) && 
    (!Board[OldI-N])){
  CellP->I -= N;
  CellP->VisualP.Y -= CellSize;
 }else if((Point.Y > MaxOverlap*CellSize) && (CellP->Y < (s32)N-1) &&
          (!Board[OldI+N])){
  CellP->I += N;
  CellP->VisualP.Y += CellSize;
 }
 
 ttt_mark Mark = Board[CellP->I];
 if(Mark){
  *CellP = State->CursorP;
 }
}

internal inline ttt_position
MakeTTTPosition(v2 VisualP, v2s CellP, u32 I){
 ttt_position Result = {};
 Result.VisualP = VisualP;
 Result.XY = CellP;
 Result.I = I;
 return Result;
}

internal inline ttt_position
TTT4DBoardPosition(tic_tac_toe_state *State, ttt_board *Board, v2 Point){
 ttt_position Result = {};
 
 u32 N = Board->N;
 
 v2 P = State->P;
 f32 CellSize = State->Tilemap->TileSize.X;
 f32 Advance = CellSize*N + State->Padding;
 
 rect BoardRect = SizeRect(V2(0), V2(N*CellSize));
 v2 BoardP = P;
 v4s CellP = V4S(0);
 for(u32 W=0; W<N; W++){
  for(u32 Z=0; Z<N; Z++){
   if(IsPointInRect(Point, BoardRect+BoardP)){
    v2 CursorP = Point-BoardP;
    CursorP /= CellSize;
    CursorP = V2(Floor(CursorP.X), Floor(CursorP.Y));
    CellP.XY = V2S(CursorP);
    CellP.Z = Z;
    CellP.W = W;
    
    Result.IsValid = true;
    Result.VisualP = TTT4DBoardCellPV2(Board, CellP, P, State->Padding, CellSize);
    Result.I = TTT4DBoardI(Board, CellP);
    Result.XY = CellP.XY;
    
    break;
   }
   
   BoardP.X += Advance;
  }
  
  BoardP.X = P.X;
  BoardP.Y += Advance;
 }
 
 return Result;
}

internal inline ttt_position
TTTBoardPosition(tic_tac_toe_state *State, ttt_board *Board, v2 Point){
 if(Board->Type == TTTBoard_4D) return TTT4DBoardPosition(State, Board, Point);
 
 ttt_position Result = {};
 
 u32 N = Board->N;
 u32 TotalZ = (Board->Type <= TTTBoard_2D) ? 1 : N;
 
 v2 P = State->P;
 f32 CellSize = State->CellSize;
 f32 Advance = CellSize*N;
 
 rect BoardRect = SizeRect(V2(0), V2(N*CellSize));
 v2 BoardP = P;
 v3s CellP = V3S(0);
 for(u32 Z=0; Z<TotalZ; Z++){
  if(IsPointInRect(Point, BoardRect+BoardP)){
   v2 CursorP = Point-BoardP;
   CursorP /= CellSize;
   CursorP = V2(Floor(CursorP.X), Floor(CursorP.Y));
   CellP.XY = V2S(CursorP);
   CellP.Z = Z;
   
   Result.IsValid = true;
   Result.VisualP = TTT3DBoardCellPV2(State, Board, CellP, P);
   Result.I = TTT3DBoardI(Board, CellP);
   Result.XY = CellP.XY;
   
   break;
  }
  
  BoardP.X += Advance;
  BoardP.Y -= State->ZYAdvance;
 }
 
 return Result;
}


//~ Traditional variation
#define TTTBoardCheckWin(Name, Loop, I) { \
ttt_mark LastMark = TTTMark_None; \
u32 WinningIndex = 0; \
Loop{ \
ttt_mark Mark = Board->Board[I]; \
if(!Mark){ \
LastMark = TTTMark_None; \
break; \
}else if(LastMark &&  \
(LastMark != Mark)){ \
LastMark = TTTMark_None; \
break; \
} \
State->WinningIndices[WinningIndex++] = I; \
LastMark = Mark; \
} \
if(LastMark) return LastMark; \
}

#define TTTBoardCountWeights(Name, Loop, I, Factor) { \
u32 Marks = 0; \
ttt_mark MarkType = TTTMark_None; \
Loop{ \
ttt_mark Mark = Board->Board[(I)]; \
if(Mark){ \
if(MarkType &&  \
(Mark != MarkType)){ \
goto continue_##Name; \
} \
MarkType = Mark; \
Marks++; \
} \
} \
\
Marks *= (Factor); \
Marks++; \
u32 Weight = 1; if(MarkType) Weight = (MarkType == TTTMark_Computer) ? TicTacToeState.OffenseWeight : TicTacToeState.DefenseWeight; \
Loop{ \
Weights[(I)] += (s32)(Weight*Marks); \
} \
\
continue_##Name:; \
} 

#define TTTVariation(Name) Name##Traditional
#include "tic_tac_toe_variation_template.h"

//~ Devil's vartiation
#define TTTBoardCheckWin(Name, Loop, I) { \
ttt_mark LastMark = TTTMark_None; \
u32 WinningIndex = 0; \
Loop{ \
ttt_mark Mark = Board->Board[I]; \
if(!Mark){ \
LastMark = TTTMark_None; \
break; \
}else if(LastMark &&  \
(LastMark != Mark)){ \
LastMark = TTTMark_None; \
break; \
} \
State->WinningIndices[WinningIndex++] = I; \
LastMark = Mark; \
} \
if(LastMark) return LastMark; \
}

#define TTTBoardCountWeights(Name, Loop, I, Factor) { \
u32 Marks = 0; \
ttt_mark MarkType = TTTMark_None; \
Loop{ \
ttt_mark Mark = Board->Board[(I)]; \
if(Mark){ \
if(MarkType &&  \
(Mark != MarkType)){ \
goto continue_##Name; \
} \
MarkType = Mark; \
Marks++; \
} \
} \
\
Marks *= (Factor); \
Marks++; \
u32 Weight = 1; if(MarkType) Weight = (MarkType == TTTMark_Computer) ? TicTacToeState.OffenseWeight : TicTacToeState.DefenseWeight; \
Loop{ \
Weights[(I)] += (s32)(Weight*Marks); \
} \
\
continue_##Name:; \
} 

#define TTTVariation(Name) Name##Devil
#include "tic_tac_toe_variation_template.h"

//~ 2D @ttt_2d 
internal inline ttt_board
MakeTTT2DBoard(memory_arena *Arena, u32 N){
 ttt_board Result = {};
 Result.N = N;
 Result.Size = N*N;
 Result.Board = PushArray(Arena, ttt_mark, Result.Size);
 Result.Type = TTTBoard_2D;
 
 return Result;
}

internal inline void
TTT2DBoardRender(tic_tac_toe_state *State, ttt_board *Board, v2 P, f32 Padding, f32 Z){
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
   if(Board->Board[I] == TTTMark_Player){
    v2 CellP = P+CellSize*V2((f32)X, (f32)Y);
    RenderSpriteSheetAnimationFrame(State->Marks, CellP, Z, 0, 1, I);
   }else if(Board->Board[I] == TTTMark_Computer){
    v2 CellP = P+CellSize*V2((f32)X, (f32)Y);
    RenderSpriteSheetAnimationFrame(State->Marks, CellP, Z, 0, 2, I);
   }
   
   X++;
   if(X >= N){
    X = 0;
    Y++;
   }
  }
 }
}

//~ 3D @ttt_3d
internal inline ttt_board
MakeTTT3DBoard(memory_arena *Arena, u32 N){
 ttt_board Result = {};
 Result.N = N;
 Result.Size = N*N*N;
 Result.Board = PushArray(Arena, ttt_mark, Result.Size);
 Result.Type = TTTBoard_3D;
 
 return Result;
}

internal inline void
TTT3DBoardRender(tic_tac_toe_state *State, ttt_board *Board, v2 P, f32 Padding, f32 Z){
 u32 N = Board->N;
 Assert(N > 0);
 
 asset_tilemap *Tilemap = State->Tilemap;
 
 f32 CellSize = State->CellSize;
 f32 Advance = CellSize;
 f32 BoardSize = CellSize*N;
 
 ttt_mark *BoardData = Board->Board;
 for(u32 J=0; J<N; J++){
  RenderTilemap(Tilemap, &State->Data, P, Z+0.1f, 0);
  {
   u32 Extra = J*N*N;
   u32 X = 0;
   u32 Y = 0;
   for(u32 I=0; I<N*N; I++){
    if(BoardData[I] == TTTMark_Player){
     v2 CellP = P+CellSize*V2((f32)X, (f32)Y);
     RenderSpriteSheetAnimationFrame(State->Marks, CellP, Z, 0, 1, Extra+I);
    }else if(BoardData[I] == TTTMark_Computer){
     v2 CellP = P+CellSize*V2((f32)X, (f32)Y);
     RenderSpriteSheetAnimationFrame(State->Marks, CellP, Z, 0, 2, Extra+I);
    }
    
    X++;
    if(X >= N){
     X = 0;
     Y++;
    }
   }
  }
  
  P.X += BoardSize;
  P.Y -= State->ZYAdvance;
  BoardData += N*N;
 }
}

//~ 4D @ttt_4d
internal inline ttt_board
MakeTTT4DBoard(memory_arena *Arena, u32 N){
 ttt_board Result = {};
 Result.N = N;
 Result.Size = N*N*N*N;
 Result.Board = PushArray(Arena, ttt_mark, Result.Size);
 Result.Type = TTTBoard_4D;
 
 return Result;
}

internal inline void
TTT4DBoardRender(tic_tac_toe_state *State, ttt_board *Board, v2 StartP, f32 Padding, f32 Z){
 u32 N = Board->N;
 Assert(N > 0);
 f32 CellSize = 16;
 f32 LineLength = N*CellSize;
 f32 Advance = CellSize;
 f32 BoardSize = CellSize*N + Padding;
 
 asset_tilemap *Tilemap = State->Tilemap;
 
 v2 P = StartP;
 ttt_mark *BoardData = Board->Board;
 for(u32 W=0; W<N; W++){
  for(u32 J=0; J<N; J++){ // Z
   RenderTilemap(Tilemap, &State->Data, P, Z+0.1f, 0);
   {
    u32 Extra = J*N*N + W*N*N*N;
    u32 X = 0;
    u32 Y = 0;
    for(u32 I=0; I<N*N; I++){
     if(BoardData[I] == TTTMark_Player){
      v2 CellP = P+CellSize*V2((f32)X, (f32)Y);
      RenderSpriteSheetAnimationFrame(State->Marks, CellP, Z, 0, 1, Extra+I);
     }else if(BoardData[I] == TTTMark_Computer){
      v2 CellP = P+CellSize*V2((f32)X, (f32)Y);
      RenderSpriteSheetAnimationFrame(State->Marks, CellP, Z, 0, 2, Extra+I);
     }
     
     X++;
     if(X >= N){
      X = 0;
      Y++;
     }
    }
   }
   
   P.X += BoardSize;
   BoardData += N*N;
  }
  
  P.X = StartP.X;
  P.Y += BoardSize;
 }
}

//~ 

internal void
UpdateAndRenderTicTacToe(){
 TIMED_FUNCTION();
 
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
 
 //RenderRect(SizeRect(V2(0), GameRenderer.ScreenToWorld(OSInput.WindowSize, ScaledItem(0))), 
 //5.5f, MakeColor(0.55f, 0.51f, 0.51f), GameItem(0));
 RenderRect(SizeRect(V2(0), GameRenderer.ScreenToWorld(OSInput.WindowSize, ScaledItem(0))), 
            5.5f, MakeColor(0.30f, 0.55f, 0.70f), GameItem(0));
 
 local_persist ttt_board Board = {};
 if(!TicTacToeState.Turn){
  u32 N = 4;
  
  Board = MakeTTT3DBoard(&PermanentStorageArena, N);
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
  if(State->Turn == TTTMark_Player){
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
    CellP = TTTBoardPosition(State, &Board, MouseP);
    if(CellP.IsValid && Board.Board[CellP.I]){
     TTTBoardMaybeShiftPosition(State, Board.Board, Board.N, &CellP, MouseP);
    }
   }
   
   if(CellP.IsValid && !Board.Board[CellP.I]) State->CursorP = CellP;
   ttt_mark *Mark = &Board.Board[State->CursorP.I];
   
   if(!*Mark){
    if(OSInput.MouseJustDown(MouseButton_Left, KeyFlag_Any) ||
       OSInput.KeyJustDown(KeyCode_Space, KeyFlag_Any)){
     *Mark = TTTMark_Player;
     State->Turn = TTTMark_Computer;
     State->Flags &= ~TTTStateFlag_PlayerHasMoved;
     PlayerJustAdded = true;
    }
   }
   
   if(LeaveKeyboardMode) State->Flags &= ~TTTStateFlag_KeyboardMode;
  }else{
   switch(Board.Type){
    case TTTBoard_2D: TTT2DBoardDoComputerMoveTraditional(State, &Board); break;
    case TTTBoard_3D: TTT3DBoardDoComputerMoveTraditional(State, &Board); break;
    case TTTBoard_4D: TTT4DBoardDoComputerMoveTraditional(State, &Board); break;
   }
   //if(OSInput.MouseJustDown(MouseButton_Left)) 
   State->Turn = TTTMark_Player;
  }
 }
 
 ttt_mark Winner = TTTMark_None;
 switch(Board.Type){
  case TTTBoard_2D: {
   TTT2DBoardRender(State, &Board, State->P, Padding, 5.0f);
   Winner = TTT2DBoardTestWinTraditional(State, &Board);
  }break;
  case TTTBoard_3D: {
   TTT3DBoardRender(State, &Board, State->P, Padding, 5.0f);
   Winner = TTT3DBoardTestWinTraditional(State, &Board);
  }break;
  case TTTBoard_4D: {
   TTT4DBoardRender(State, &Board, State->P, Padding, 5.0f);
   Winner = TTT4DBoardTestWinTraditional(State, &Board);
  }break;
 }
 
 if(Winner == TTTMark_Player){
  RenderString(&TitleFont, WHITE, V2(400, 300), -10.0f, "You win!");
 }else if(Winner == TTTMark_Computer){
  RenderString(&TitleFont, WHITE, V2(400, 300), -10.0f, "Computer wins!");
 }else if(Winner == TTTMark_TOTAL){
  RenderString(&TitleFont, WHITE, V2(400, 300), -10.0f, "Cat's game!");
 }
 if(Winner) {
  State->Flags |= TTTStateFlag_HasAWinner;
  if(!PlayerJustAdded && (OSInput.MouseJustDown(MouseButton_Left, KeyFlag_Any) ||
                          OSInput.KeyJustDown(KeyCode_Space, KeyFlag_Any))){
   State->Flags &= ~TTTStateFlag_HasAWinner;
   State->Flags &= ~TTTStateFlag_ComputerHasMoved;
   State->Turn = TTTMark_Player;
   for(u32 I=0; I<Board.Size; I++) Board.Board[I] = TTTMark_None;
  }
  
  if(Winner != TTTMark_TOTAL){
   for(u32 I=0; I<N; I++){
    Assert(Board.Type != TTTBoard_4D);
    u32 Index = State->WinningIndices[I];
    v3s CellP = TTT3DBoardCellP(&Board, Index);
    v2 CellPV2 = TTT3DBoardCellPV2(State, &Board, CellP, P);
    GameRenderer.AddLight(CellPV2+V2(0.5f*CellSize), MakeColor(1.0f, 0.6f, 0.3f), 
                          1.0f, CellSize, GameItem(0));
   }
  }
 }
 
 if(!(State->Flags & TTTStateFlag_HasAWinner)){
  if(!Board.Board[State->CursorP.I] ||
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
}