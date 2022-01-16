#ifndef MENU_H
#define MENU_H

enum menu_page_type {
    MenuPage_Main,
    MenuPage_Pause,
    MenuPage_Settings,
};

struct menu_page {
    font *Font;
    rect R;
    v2 P;
    f32 YAdvance;
    
};

struct menu_state {
    game_mode LastGameMode;
    
    menu_page_type LastPage;
    menu_page_type Page;
    
    const char *UsingSlider;
    const char *Hovered;
    
    f32 HoverT;
};

global_constant color MENU_BACKGROUND_COLOR = MakeColor(0x0d2440ff);
global_constant color MENU_BASE_COLOR = WHITE;
global_constant color MENU_HOVER_COLOR = YELLOW;
global_constant f32 MENU_FLASH_FACTOR = 3.0f;

#endif //MENU_H
