#ifndef BOARD_GAME_H
#define BOARD_GAME_H

struct game_board_cell {
    u8 Mark;
};

struct game_board {
    u32 Size;
    u32 N;
    game_board_cell *Board;
};

internal void UpdateAndRenderBoardGame();

#endif //BOARD_GAME_H
