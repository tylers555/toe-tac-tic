
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

//~ 2D
internal inline ttt_mark
TTT2DBoardTestWinTraditional(tic_tac_toe_state *State, ttt_board *Board){
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
TTT2DBoardDoComputerMoveTraditional(tic_tac_toe_state *State, ttt_board *Board){
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
TTT3DBoardTestWinTraditional(tic_tac_toe_state *State, ttt_board *Board){
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
TTT3DBoardDoComputerMoveTraditional(tic_tac_toe_state *State, ttt_board *Board){
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
TTT4DBoardTestWinTraditional(tic_tac_toe_state *State, ttt_board *Board){
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
TTT4DBoardDoComputerMoveTraditional(tic_tac_toe_state *State, ttt_board *Board){
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
    
#if 0
    UpdateAndRenderBoardGame();
#else
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
                    //State->Turn = TTTMark_Computer;
                    State->Flags &= ~TTTStateFlag_PlayerHasMoved;
                    PlayerJustAdded = true;
                    
                    asset_sound_effect *Asset = AssetSystem.GetSoundEffect(String("ttt_place_mark"));
                    AudioMixer.PlaySound(Asset);
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
        if(!(State->Flags & TTTStateFlag_HasAWinner)){
            asset_sound_effect *Asset = AssetSystem.GetSoundEffect(String("ttt_win"));
            AudioMixer.PlaySound(Asset);
        }
        
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
#endif
}