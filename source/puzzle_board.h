#ifndef PUZZLE_BOARD_H
#define PUZZLE_BOARD_H

enum puzzle_cell_type {
    PuzzleCell_None        = 0,
    PuzzleCell_SlideUp     = 1,
    PuzzleCell_SlideDown   = 2,
    PuzzleCell_SlideLeft   = 3,
    PuzzleCell_SlideRight  = 4,
    PuzzleCell_NeededToWinVertical   = 5,
    PuzzleCell_NeededToWinHorizontal = 6,
    PuzzleCell_NeededToWinAny        = 7,
};

enum puzzle_mark {
    PuzzleMark_None = 0, 
    PuzzleMark_X    = 1, 
    PuzzleMark_O    = 2, 
    PuzzleMark_E    = 3, 
};

struct puzzle_cell {
    puzzle_cell_type Type; 
    union {
        u8 NeededInARow;
    };
    puzzle_mark Mark;
};

struct puzzle_board {
    u32 Size;
    u32 N;
    puzzle_cell *Board;
};

struct puzzle_update_node {
    ttt_position CellP;
    puzzle_mark Mark;
    puzzle_update_node *Next;
};

struct puzzle_state {
    memory_arena Memory;
    
    puzzle_mark Turn;
    
    asset_tilemap *Tilemap;
    asset_sprite_sheet *Marks;
    tilemap_data Data;
    
    f32 CellSize;
    
    puzzle_update_node *FirstUpdate;
    puzzle_update_node *LastUpdate;
    puzzle_update_node *FirstFreeUpdate;
    
    // Settings
    v2 P = V2(10, 50);
    f32 Thickness = 1;
    f32 Padding = 1;
};


#endif //PUZZLE_BOARD_H
