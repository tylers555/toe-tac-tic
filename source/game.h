#ifndef GAME_H
#define GAME_H

struct game_settings {
    os_key_code Jump        = KeyCode_Space;
    os_key_code Select      = (os_key_code)'X';
    os_key_code PlayerLeft  = KeyCode_Left;
    os_key_code PlayerRight = KeyCode_Right;
    
    os_key_code BoardUp    = KeyCode_Up;
    os_key_code BoardDown  = KeyCode_Down;
    os_key_code BoardLeft  = KeyCode_Left;
    os_key_code BoardRight = KeyCode_Right;
    os_key_code BoardPlace = KeyCode_Space;
};

global_constant os_key_code PAUSE_KEY = KeyCode_Escape;

#endif //GAME_H
