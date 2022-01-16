#ifndef PUZZLE_BOARD_H
#define PUZZLE_BOARD_H

struct game_board_cell {
    u8 Mark;
};

struct game_board {
    u32 Size;
    u32 N;
    game_board_cell *Board;
};

internal void UpdateAndRenderBoardGame();

#endif //PUZZLE_BOARD_H
