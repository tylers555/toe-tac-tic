#ifndef TIC_TAC_TOE_H
#define TIC_TAC_TOE_H

typedef u8 ttt_mark;
enum ttt_mark_ {
    TTTMark_None,
    
    TTTMark_Player,
    TTTMark_Computer,
    
    TTTMark_TOTAL
};

enum ttt_board_type {
    TTTBoard_2D = 2,
    TTTBoard_3D = 3,
    TTTBoard_4D = 4,
};

struct ttt_board {
    ttt_board_type Type;
    u32 N;
    u32 Size;
    ttt_mark *Board;
};

struct ttt_win_state {
    ttt_mark Winner;
};

struct ttt_position {
    v2 VisualP;
    
    union{
        struct {
            s32 X;
            s32 Y;
        };
        v2s XY;
    };
    
    u32 I;
    b8 IsValid;
};

typedef u32 tic_tac_toe_state_flags;
enum tic_tac_toe_state_flags_ {
    TTTStateFlag_None             = (0 << 0),
    TTTStateFlag_HasAWinner       = (1 << 0),
    TTTStateFlag_ComputerHasMoved = (1 << 1),
    TTTStateFlag_KeyboardMode     = (1 << 2),
    TTTStateFlag_PlayerHasMoved   = (1 << 3),
};

struct tic_tac_toe_state {
    ttt_mark Turn;
    tic_tac_toe_state_flags Flags;
    
    const f32 Cursor1MoveFactor = 0.8f;
    const f32 Cursor2MoveFactor = 0.3f;
    const f32 ComputerMoveFactor = 0.8f;
    
    ttt_position CursorP;
    v2 Cursor1P;
    v2 Cursor2P;
    
    v2 ComputerP;
    v2 TargetComputerP;
    
    u32 *WinningIndices;
    
    asset_tilemap *Tilemap;
    asset_sprite_sheet *Marks;
    tilemap_data Data;
    
    // Settings
    v2 P = V2(10, 100);
    f32 Thickness = 1;
    f32 Padding = 1;
    
    f32 CellSize;
    f32 ZYAdvance;
    
    u32 OffenseWeight = 1;
    u32 DefenseWeight = 1;
    
};

#endif //TIC_TAC_TOE_H
