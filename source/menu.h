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
    u32 IDCounter;
};

global_constant u32 MAX_MENU_ITEMS = 16;

struct menu_item_state {
    f32 HoverT;
    f32 ActiveT;
};

struct menu_state {
    menu_item_state ItemStates[MAX_MENU_ITEMS];
    
    game_mode LastGameMode;
    
    menu_page_type LastPage;
    menu_page_type Page;
    
    const char *UsingSlider;
    const char *Hovered;
    b8 SetHovered;
};

global_constant color MENU_BACKGROUND_COLOR = MakeColor(0x0d2440ff);
global_constant f32 MENU_HOVER_FACTOR = 5.0f;
global_constant f32 MENU_ACTIVE_FACTOR = 5.0f;

global_constant color MENU_BASE_COLOR = WHITE;
global_constant color MENU_BASE_COLOR2 = RED;
global_constant color MENU_HOVER_COLOR = GREEN;
global_constant color MENU_ACTIVE_COLOR = BLUE;

#endif //MENU_H
