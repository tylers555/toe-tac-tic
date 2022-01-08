
//~ 2D
internal inline ttt_mark
TTTVariation(TTT2DBoardTestWin)(tic_tac_toe_state *State, ttt_board *Board){
 TIMED_FUNCTION();
 u32 N = Board->N;
 
 YLoop TTTBoardCheckWin(Horizontals, XLoop, X+Y);
 XLoop TTTBoardCheckWin(Verticals, YLoop, X+Y);
 
 TTTBoardCheckWin(ForwardDiagonal, for(u32 I=0; I<N*N; I+=N+1), I);
 TTTBoardCheckWin(BackwardDiagonal, for(u32 I=N-1; I<N*N-1; I+=N-1), I);
 
 /* Cat's game */
 for(u32 I=0; I<N*N; I++){
  if(!Board->Board[I]) return TTTMark_None;
 }
 return TTTMark_TOTAL; 
}

internal void
TTTVariation(TTT2DBoardDoComputerMove)(tic_tac_toe_state *State, ttt_board *Board){
 TIMED_FUNCTION();
 u32 N = Board->N;
 s32 *Weights = PushArray(&TransientStorageArena, s32, N*N);
 
 YLoop TTTBoardCountWeights(Horizontals, XLoop, X+Y, Marks);
 XLoop TTTBoardCountWeights(Verticals, YLoop, X+Y, Marks);
 
 TTTBoardCountWeights(ForwardDiagonal, for(u32 I=0; I<N*N; I+=N+1), I, Marks);
 TTTBoardCountWeights(BackwardDiagonal, for(u32 I=N-1; I<N*N-1; I+=N-1), I,Marks);
 
 s32 HighestWeight = -1;
 array<u32> HighestWeightIndicies = MakeArray<u32>(&TransientStorageArena, N*N);
 for(u32 I=0; I<N*N; I++){
  if(Board->Board[I]) continue;
  if(Weights[I] > HighestWeight){
   ArrayClear(&HighestWeightIndicies);
   HighestWeight = Weights[I];
   ArrayAdd(&HighestWeightIndicies, I);
  }else if(Weights[I] == HighestWeight){
   ArrayAdd(&HighestWeightIndicies, I);
  }
 }
 Assert(HighestWeightIndicies.Count);
 
 /* TODO(Tyler): BETTER RANDOM NUMBER GENERATION */
 u32 IndexIndex = GetRandomNumber((u32)(1234.0f*Counter)) % HighestWeightIndicies.Count;
 u32 Index = HighestWeightIndicies[IndexIndex];
 Board->Board[Index] = TTTMark_Computer;
 
 v3s ComputerCellP = TTT3DBoardCellP(Board, Index);
 f32 CellSize = State->Tilemap->TileSize.X;
 State->TargetComputerP = TTT3DBoardCellPV2(State, Board, ComputerCellP, State->P);
 if(!(State->Flags & TTTStateFlag_ComputerHasMoved)){
  State->ComputerP = State->TargetComputerP;
  State->Flags |= TTTStateFlag_ComputerHasMoved;
 }
}

//~ 3D 
internal inline ttt_mark
TTTVariation(TTT3DBoardTestWin)(tic_tac_toe_state *State, ttt_board *Board){
 TIMED_FUNCTION();
 
 u32 N = Board->N;
 
 ZLoop{
  YLoop TTTBoardCheckWin(XWins, XLoop, Z+Y+X);
  XLoop TTTBoardCheckWin(YWins, YLoop, Z+Y+X);
 }
 YLoop XLoop TTTBoardCheckWin(ZWins, ZLoop, Z+Y+X);
 
#define TTT3DBoardPlanarDiagonalCheck(Name, ForwardAdvance, BackwardOffset, BackwardAdvance, JAdvance) \
for(u32 J=0; J<N*(JAdvance); J+=(JAdvance)){ \
TTTBoardCheckWin(Name##ForwardDiagonal, for(u32 I=J; I<J+N*(ForwardAdvance); I+=(ForwardAdvance)), I); \
TTTBoardCheckWin(Name##BackwardDiagonal, for(u32 I=J+(BackwardOffset); I<J+(BackwardOffset)+N*(BackwardAdvance); I+=(BackwardAdvance)), I); \
}
 
 TTT3DBoardPlanarDiagonalCheck(XYWins,   N+1,   N-1,   N-1, N*N);
 TTT3DBoardPlanarDiagonalCheck(XZWins, N*N+1,   N-1, N*N-1,   N);
 TTT3DBoardPlanarDiagonalCheck(YZWins, N*N+N, N*N-N, N*N-N,   1);
 
#undef TTT3DBoardPlanarDiagonalCheck
 
#define TTT3DBoardDiagonalCheck(Name, Offset, Advance) \
TTTBoardCheckWin(Name, for(u32 I=(Offset); I<(Offset)+N*(Advance); I+=(Advance)), I)
 
 TTT3DBoardDiagonalCheck(A,     0, N*N + N + 1);
 TTT3DBoardDiagonalCheck(B,   N-1, N*N + N - 1);
 TTT3DBoardDiagonalCheck(C, N*N-N, N*N - N + 1);
 TTT3DBoardDiagonalCheck(D, N*N-1, N*N - N - 1);
 
#undef TTT3DBoardDiagonalCheck
 
 // Cat's game
 for(u32 I=0; I<N*N*N; I++){
  if(!Board->Board[I]) return TTTMark_None;
 }
 
 return TTTMark_TOTAL;
}

internal void
TTTVariation(TTT3DBoardDoComputerMove)(tic_tac_toe_state *State, ttt_board *Board){
 TIMED_FUNCTION();
 
 u32 N = Board->N;
 s32 *Weights = PushArray(&TransientStorageArena, s32, N*N*N);
 
 ZLoop{
  YLoop TTTBoardCountWeights(XWeights, XLoop, Z+Y+X, Marks*Marks);
  XLoop TTTBoardCountWeights(YWeights, YLoop, Z+Y+X, Marks*Marks);
 }
 YLoop XLoop TTTBoardCountWeights(ZWeights, ZLoop, Z+Y+X, Marks*Marks);
 
 
#define TTT3DBoardPlanarDiagonalWeights(Name, ForwardAdvance, BackwardOffset, BackwardAdvance, JAdvance) \
for(u32 J=0; J<N*(JAdvance); J+=(JAdvance)){ \
TTTBoardCountWeights(Name##Forward, for(u32 I=J; I<J+N*(ForwardAdvance); I+=(ForwardAdvance)), I, Marks*Marks); \
TTTBoardCountWeights(Name##Backward, for(u32 I=J+(BackwardOffset); I<J+(BackwardOffset)+N*(BackwardAdvance); I+=(BackwardAdvance)), I, Marks*Marks); \
}
 
 TTT3DBoardPlanarDiagonalWeights(XYWeights, N+1,     N-1,   N-1, N*N);
 TTT3DBoardPlanarDiagonalWeights(XZWeights, N*N+1,   N-1, N*N-1,   N);
 TTT3DBoardPlanarDiagonalWeights(YZWeights, N*N+N, N*N-N, N*N-N,   1);
 
#undef TTT3DBoardPlanarDiagonalWeights
 
#define TTT3DBoardDiagonalWeights(Name, Offset, Advance) \
TTTBoardCountWeights(Name, for(u32 I=(Offset); I<(Offset)+N*(Advance); I+=(Advance)), I, Marks*Marks); 
 
 TTT3DBoardDiagonalWeights(A,     0, N*N + N + 1); // A
 TTT3DBoardDiagonalWeights(B,   N-1, N*N + N - 1); // B
 TTT3DBoardDiagonalWeights(C, N*N-N, N*N - N + 1); // C
 TTT3DBoardDiagonalWeights(D, N*N-1, N*N - N - 1); // D
 
#undef TTT3DBoardDiagonalWeights
 
 s32 HighestWeight = -1;
 array<u32> HighestWeightIndicies = MakeArray<u32>(&TransientStorageArena, N*N*N);
 for(u32 I=0; I<N*N*N; I++){
  if(Board->Board[I]) continue;
  if(Weights[I] > HighestWeight){
   ArrayClear(&HighestWeightIndicies);
   HighestWeight = Weights[I];
   ArrayAdd(&HighestWeightIndicies, I);
  }else if(Weights[I] == HighestWeight){
   ArrayAdd(&HighestWeightIndicies, I);
  }
 }
 Assert(HighestWeightIndicies.Count);
 // TODO(Tyler): BETTER RANDOM NUMBER GENERATION
 u32 IndexIndex = GetRandomNumber((u32)(1234.0f*Counter)) % HighestWeightIndicies.Count;
 u32 Index = HighestWeightIndicies[IndexIndex];
 
 //if(OSInput.MouseJustDown(MouseButton_Left)){
 Board->Board[Index] = TTTMark_Computer;
 //}
 
 v3s ComputerCellP = TTT3DBoardCellP(Board, Index);
 f32 CellSize = State->Tilemap->TileSize.X;
 State->TargetComputerP = TTT3DBoardCellPV2(State, Board, ComputerCellP, State->P);
 if(!(State->Flags & TTTStateFlag_ComputerHasMoved)){
  State->ComputerP = State->TargetComputerP;
  State->Flags |= TTTStateFlag_ComputerHasMoved;
 }
}

//~ 4D
internal inline ttt_mark
TTTVariation(TTT4DBoardTestWin)(tic_tac_toe_state *State, ttt_board *Board){
 TIMED_FUNCTION();
 
 u32 N = Board->N;
 u32 N2 = N*N;
 u32 N3 = N*N*N;
 
 // Straight wins
 WLoop {
  ZLoop {
   YLoop TTTBoardCheckWin(XWins, XLoop, W+Z+Y+X);
   XLoop TTTBoardCheckWin(YWins, YLoop, W+Z+Y+X);
  }
  YLoop XLoop TTTBoardCheckWin(ZWins, ZLoop, W+Z+Y+X);
 }
 ZLoop YLoop XLoop TTTBoardCheckWin(WWins, WLoop, W+Z+Y+X);
 
 // Planar diagonal wins
#define TTT4DBoardPlanarDiagonalCheck(Name, ForwardAdvance, BackwardOffset, BackwardAdvance, JAdvance, KAdvance) \
for(u32 K=0; K<N*(KAdvance); K+=(KAdvance)){ \
for(u32 J=K; J<K+N*(JAdvance); J+=(JAdvance)){ \
TTTBoardCheckWin(Name##ForwardDiagonal, for(u32 I=J; I<J+N*(ForwardAdvance); I+=(ForwardAdvance)), I); \
TTTBoardCheckWin(Name##BackwardDiagonal, for(u32 I=J+(BackwardOffset); I<J+(BackwardOffset)+N*(BackwardAdvance); I+=(BackwardAdvance)), I); \
} \
}
 
 TTT4DBoardPlanarDiagonalCheck(XYWins,   N+1,   N-1,   N-1, N2, N3);
 TTT4DBoardPlanarDiagonalCheck(XZWins,  N2+1,   N-1,  N2-1,  N, N3);
 TTT4DBoardPlanarDiagonalCheck(XWWins,  N3+1,   N-1,  N3-1,  N, N2);
 TTT4DBoardPlanarDiagonalCheck(YZins,   N2+N,  N2-N,  N2-N,  1, N3);
 TTT4DBoardPlanarDiagonalCheck(YWWins,  N3+N,  N2-N,  N3-N,  1, N2);
 TTT4DBoardPlanarDiagonalCheck(ZWWins, N3+N2, N3-N2, N3-N2,  1, N);
 
#undef TTT4DBoardPlanarDiagonalCheck
 
 // Cubic diagonal wins
#define TTT4DBoardCubicDiagonalCheck(Name, Offset, Advance, J) \
TTTBoardCheckWin(Name, for(u32 I=(J)+(Offset); I<(J)+(Offset)+N*(Advance); I+=(Advance)), I); \
 
 // XYZ
 WLoop{
  TTT4DBoardCubicDiagonalCheck(XYZA,    0, N2+N+1, W);
  TTT4DBoardCubicDiagonalCheck(XYZB,  N-1, N2+N-1, W);
  TTT4DBoardCubicDiagonalCheck(XYZC, N2-N, N2-N+1, W);
  TTT4DBoardCubicDiagonalCheck(XYZD, N2-1, N2-N-1, W);
 }
 
 // XYW
 ZLoop{
  TTT4DBoardCubicDiagonalCheck(XYWA,    0, N3+N+1, Z);
  TTT4DBoardCubicDiagonalCheck(XYWB,  N-1, N3+N-1, Z);
  TTT4DBoardCubicDiagonalCheck(XYWC, N2-N, N3-N+1, Z);
  TTT4DBoardCubicDiagonalCheck(XYWD, N2-1, N3-N-1, Z);
 }
 
 // XZW
 YLoop{
  TTT4DBoardCubicDiagonalCheck(XZWA,         0, N3+N2+1, Y);
  TTT4DBoardCubicDiagonalCheck(XZWB,       N-1, N3+N2-1, Y);
  TTT4DBoardCubicDiagonalCheck(XZWC,     N3-N2, N3-N2+1, Y);
  TTT4DBoardCubicDiagonalCheck(XZWD, N3-N2+N-1, N3-N2-1, Y);
 }
 
 // YZW
 XLoop{
  TTT4DBoardCubicDiagonalCheck(YZWA,     0, N3+N2+N, X);
  TTT4DBoardCubicDiagonalCheck(YZWB,   N-1, N3+N2-N, X);
  TTT4DBoardCubicDiagonalCheck(YZWC, N3-N2, N3-N2+N, X);
  TTT4DBoardCubicDiagonalCheck(YZWD,  N3-N, N3-N2-N, X);
 }
 
#undef TTT4DBoardCubicDiagonalCheck
 
#define TTT4DBoardDiagonalCheck(Name, Offset, Advance) \
TTTBoardCheckWin(Name, for(u32 I=(Offset); I<(Offset)+N*(Advance); I+=(Advance)), I)
 
 TTT4DBoardDiagonalCheck(XYZWA,          0, N3+N2+N+1);
 TTT4DBoardDiagonalCheck(XYZWB,        N-1, N3+N2+N-1);
 TTT4DBoardDiagonalCheck(XYZWC,       N2-N, N3+N2-N+1);
 TTT4DBoardDiagonalCheck(XYZWD,       N2-1, N3+N2-N-1);
 TTT4DBoardDiagonalCheck(XYZWE,      N3-N2, N3-N2+N+1);
 TTT4DBoardDiagonalCheck(XYZWF,  N3-N2+N-1, N3-N2+N-1);
 TTT4DBoardDiagonalCheck(XYZWG,       N3-N, N3-N2-N+1);
 TTT4DBoardDiagonalCheck(XYZWH,       N3-1, N3-N2-N-1);
 
#undef TTT4DBoardDiagonalCheck
 
 
 // Cat's game
 for(u32 I=0; I<N*N*N*N; I++){
  if(!Board->Board[I]) return TTTMark_None;
 }
 
 return TTTMark_TOTAL;
}

internal void
TTTVariation(TTT4DBoardDoComputerMove)(tic_tac_toe_state *State, ttt_board *Board){
 TIMED_FUNCTION();
 
 u32 N = Board->N;
 u32 N2 = N*N;
 u32 N3 = N*N*N;
 
 s32 *Weights = PushArray(&TransientStorageArena, s32, N*N3);
 
 // Straight wins
 WLoop {
  ZLoop {
   YLoop TTTBoardCountWeights(XWins, XLoop, W+Z+Y+X, Marks*Marks*Marks);
   XLoop TTTBoardCountWeights(YWins, YLoop, W+Z+Y+X, Marks*Marks*Marks);
  }
  
  YLoop XLoop TTTBoardCountWeights(ZWins, ZLoop, W+Z+Y+X, Marks*Marks*Marks);
 }
 ZLoop YLoop XLoop TTTBoardCountWeights(WWins, WLoop, W+Z+Y+X, Marks*Marks*Marks);
 
 // Planar diagonal wins
#define TTT4DBoardPlanarDiagonalWeights(Name, ForwardAdvance, BackwardOffset, BackwardAdvance, JAdvance, KAdvance) \
for(u32 K=0; K<N*(KAdvance); K+=(KAdvance)){ \
for(u32 J=K; J<K+N*(JAdvance); J+=(JAdvance)){ \
TTTBoardCountWeights(Name##ForwardDiagonal, for(u32 I=J; I<J+N*(ForwardAdvance); I+=(ForwardAdvance)), I, Marks*Marks*Marks); \
TTTBoardCountWeights(Name##BackwardDiagonal, for(u32 I=J+(BackwardOffset); I<J+(BackwardOffset)+N*(BackwardAdvance); I+=(BackwardAdvance)), I, Marks*Marks*Marks); \
} \
}
 
 TTT4DBoardPlanarDiagonalWeights(XYWins,   N+1,   N-1,   N-1, N2, N3);
 TTT4DBoardPlanarDiagonalWeights(XZWins,  N2+1,   N-1,  N2-1,  N, N3);
 TTT4DBoardPlanarDiagonalWeights(XWWins,  N3+1,   N-1,  N3-1,  N, N2);
 TTT4DBoardPlanarDiagonalWeights(YZins,   N2+N,  N2-N,  N2-N,  1, N3);
 TTT4DBoardPlanarDiagonalWeights(YWWins,  N3+N,  N2-N,  N3-N,  1, N2);
 TTT4DBoardPlanarDiagonalWeights(ZWWins, N3+N2, N3-N2, N3-N2,  1, N);
 
#undef TTT4DBoardPlanarDiagonalWeights
 
 // Cubic diagonal wins
#define TTT4DBoardCubicDiagonalWeights(Name, Offset, Advance, J) \
TTTBoardCountWeights(Name, for(u32 I=(J)+(Offset); I<(J)+(Offset)+N*(Advance); I+=(Advance)), I, Marks*Marks*Marks); \
 
 // XYZ
 WLoop{
  TTT4DBoardCubicDiagonalWeights(XYZA,    0, N2+N+1, W);
  TTT4DBoardCubicDiagonalWeights(XYZB,  N-1, N2+N-1, W);
  TTT4DBoardCubicDiagonalWeights(XYZC, N2-N, N2-N+1, W);
  TTT4DBoardCubicDiagonalWeights(XYZD, N2-1, N2-N-1, W);
 }
 
 // XYW
 ZLoop{
  TTT4DBoardCubicDiagonalWeights(XYWA,    0, N3+N+1, Z);
  TTT4DBoardCubicDiagonalWeights(XYWB,  N-1, N3+N-1, Z);
  TTT4DBoardCubicDiagonalWeights(XYWC, N2-N, N3-N+1, Z);
  TTT4DBoardCubicDiagonalWeights(XYWD, N2-1, N3-N-1, Z);
 }
 
 // XZW
 YLoop{
  TTT4DBoardCubicDiagonalWeights(XZWA,         0, N3+N2+1, Y);
  TTT4DBoardCubicDiagonalWeights(XZWB,       N-1, N3+N2-1, Y);
  TTT4DBoardCubicDiagonalWeights(XZWC,     N3-N2, N3-N2+1, Y);
  TTT4DBoardCubicDiagonalWeights(XZWD, N3-N2+N-1, N3-N2-1, Y);
 }
 
 // YZW
 XLoop{
  TTT4DBoardCubicDiagonalWeights(YZWA,     0, N3+N2+N, X);
  TTT4DBoardCubicDiagonalWeights(YZWB,   N-1, N3+N2-N, X);
  TTT4DBoardCubicDiagonalWeights(YZWC, N3-N2, N3-N2+N, X);
  TTT4DBoardCubicDiagonalWeights(YZWD,  N3-N, N3-N2-N, X);
 }
 
#undef TTT4DBoardCubicDiagonalWeights
 
#define TTT4DBoardDiagonalWeights(Name, Offset, Advance) \
TTTBoardCountWeights(Name, for(u32 I=(Offset); I<(Offset)+N*(Advance); I+=(Advance)), I, Marks*Marks*Marks)
 
 TTT4DBoardDiagonalWeights(XYZWA,          0, N3+N2+N+1);
 TTT4DBoardDiagonalWeights(XYZWB,        N-1, N3+N2+N-1);
 TTT4DBoardDiagonalWeights(XYZWC,       N2-N, N3+N2-N+1);
 TTT4DBoardDiagonalWeights(XYZWD,       N2-1, N3+N2-N-1);
 TTT4DBoardDiagonalWeights(XYZWE,      N3-N2, N3-N2+N+1);
 TTT4DBoardDiagonalWeights(XYZWF,  N3-N2+N-1, N3-N2+N-1);
 TTT4DBoardDiagonalWeights(XYZWG,       N3-N, N3-N2-N+1);
 TTT4DBoardDiagonalWeights(XYZWH,       N3-1, N3-N2-N-1);
 
#undef TTT4DBoardDiagonalWeights
 
 s32 HighestWeight = -1;
 array<u32> HighestWeightIndicies = MakeArray<u32>(&TransientStorageArena, N*N*N);
 for(u32 I=0; I<N*N3; I++){
  if(Board->Board[I]) continue;
  if(Weights[I] > HighestWeight){
   ArrayClear(&HighestWeightIndicies);
   HighestWeight = Weights[I];
   ArrayAdd(&HighestWeightIndicies, I);
  }else if(Weights[I] == HighestWeight){
   ArrayAdd(&HighestWeightIndicies, I);
  }
 }
 Assert(HighestWeightIndicies.Count);
 // TODO(Tyler): BETTER RANDOM NUMBER GENERATION
 u32 IndexIndex = GetRandomNumber((u32)(1234.0f*Counter)) % HighestWeightIndicies.Count;
 u32 Index = HighestWeightIndicies[IndexIndex];
 
 //if(OSInput.MouseJustDown(MouseButton_Left)){
 Board->Board[Index] = TTTMark_Computer;
 //}
 
 v4s ComputerCellP = TTT4DBoardCellP(Board, Index);
 f32 CellSize = State->Tilemap->TileSize.X;
 State->TargetComputerP = TTT4DBoardCellPV2(Board, ComputerCellP, State->P, State->Padding, CellSize);
 if(!(State->Flags & TTTStateFlag_ComputerHasMoved)){
  State->ComputerP = State->TargetComputerP;
  State->Flags |= TTTStateFlag_ComputerHasMoved;
 }
}

#undef TTTBoardCheckWin
#undef TTTBoardCountWeights
#undef TTTVariation