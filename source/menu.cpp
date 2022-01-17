
//~ Helpers
internal inline void
OpenPauseMenu(){
    MenuState.LastGameMode = GameMode;
    MenuState.Page = MenuPage_Pause;
    ChangeState(GameMode_Menu, MakeString(0));
}

internal inline void
MenuChangePage(menu_state *State, menu_page_type NewPage){
    State->Page = NewPage;
    for(u32 I=0; I<MAX_MENU_ITEMS; I++){
        State->ItemStates[I] = {};
    }
}

//~ Menu page
internal inline menu_page
MakeMenuPage(font *Font, v2 P, f32 YAdvance){
    menu_page Result = {};
    Result.Font = Font;
    Result.P = P;
    Result.YAdvance = YAdvance;
    Result.R = CenterRect(V2(P.X, P.Y-Font->Descent), V2(500, Font->Size));
    
    return Result;
}

internal inline menu_item_state *
MenuPageBeginItem(menu_state *State, menu_page *Page){
    Assert(Page->IDCounter < MAX_MENU_ITEMS);
    menu_item_state *Result = &State->ItemStates[Page->IDCounter++];
    return Result;
}

internal inline color
MenuItemGetColor(menu_item_state *Item, color BaseColor, b8 Hovered, b8 Active){
    color Color = MixColor(MENU_HOVER_COLOR, BaseColor, Item->HoverT);
    if(Hovered) Item->HoverT += MENU_HOVER_FACTOR*OSInput.dTime;
    else Item->HoverT -= MENU_HOVER_FACTOR*OSInput.dTime;
    Item->HoverT = Clamp(Item->HoverT, 0.0f, 1.0f);
    
    Color = MixColor(MENU_ACTIVE_COLOR, Color, Item->ActiveT);
    if(Active) Item->ActiveT += MENU_ACTIVE_FACTOR*OSInput.dTime; 
    else Item->ActiveT -= MENU_ACTIVE_FACTOR*OSInput.dTime;
    Item->ActiveT = Clamp(Item->ActiveT, 0.0f, 1.0f);
    
    return Color;
}

internal inline b8
MenuPageDoText(menu_state *State, menu_page *Page, const char *Name){
    b8 Result = false;
    
    menu_item_state *Item = MenuPageBeginItem(State, Page);
    if(IsPointInRect(OSInput.MouseP, Page->R)){
        if(OSInput.MouseJustDown(MouseButton_Left, KeyFlag_Any)){
            Result = true;
        }
        
        State->Hovered = Name;
        State->SetHovered = true;
    }
    
    color Color = MenuItemGetColor(Item, MENU_BASE_COLOR, (State->Hovered==Name), false);
    RenderCenteredString(Page->Font, Color, Page->P, 0.0f, Name);
    
    Page->P.Y -= Page->YAdvance;
    Page->R -= V2(0, Page->YAdvance);
    
    return Result;
}

internal inline f32
MenuPageDoSlider(menu_state *State, menu_page *Page, const char *Name, f32 CurrentValue){
    f32 Result = CurrentValue;
    
    f32 StringAdvance = GetStringAdvance(Page->Font, Name);
    
    f32 Size = 20;
    v2 CursorSize = V2(Size);
    f32 Width = 500;
    f32 EffectiveWidth = Width-CursorSize.X;
    f32 Padding = 20;
    
    v2 P = Page->P;
    P.X -= 0.5f*(StringAdvance+Padding+Width);
    RenderString(Page->Font, WHITE, P, 0.0f, Name);
    P.X += StringAdvance + Padding + 0.5f*CursorSize.Width;
    P.Y += -Page->Font->Descent;
    
    v2 CursorBaseP = P;
    CursorBaseP.X += 0.5f*CursorSize.Width;
    
    v2 CursorP = CursorBaseP;
    CursorP.X += CurrentValue*EffectiveWidth;
    
    rect CursorRect = CenterRect(CursorP, CursorSize);
    
    rect LineRect = SizeRect(V2(P.X, P.Y-0.5f*Size), V2(Width, Size));
    RenderRoundedRect(LineRect, 0.0f, 0.5f*Size, MENU_BASE_COLOR, UIItem(0));
    
    menu_item_state *Item = MenuPageBeginItem(State, Page);
    if(State->UsingSlider == Name){
        Result = (OSInput.MouseP.X - CursorBaseP.X) / EffectiveWidth;
        Result = Clamp(Result, 0.0f, 1.0f);
        
        State->SetHovered = true;
        
    }else if(!State->UsingSlider && IsPointInRect(OSInput.MouseP, LineRect)){
        State->Hovered = Name;
        State->SetHovered = true;
        
        if(OSInput.MouseDown(MouseButton_Left, KeyFlag_Any)){
            State->UsingSlider = Name;
        }
    }
    
    color Color = MenuItemGetColor(Item, MENU_BASE_COLOR2, 
                                   (State->Hovered==Name), (State->UsingSlider==Name));
    RenderRoundedRect(CursorRect, -1.0f, 0.5f*Size, Color, UIItem(0));
    
    Page->P.Y -= Page->YAdvance;
    Page->R -= V2(0, Page->YAdvance);
    
    return Result;
}

//~ 
internal void 
DoMainMenu(menu_state *State, font *ItemFont, v2 P, f32 YAdvance){
    menu_page Page = MakeMenuPage(ItemFont, P, YAdvance);
    
    if(MenuPageDoText(State, &Page, "Start")){
        ChangeState(GameMode_MainGame, String(STARTUP_LEVEL));
    }
    if(MenuPageDoText(State, &Page, "Settings")){
        State->LastPage = State->Page;
        State->Page = MenuPage_Settings;
    }
    if(MenuPageDoText(State, &Page, "Quit")){
        OSEndGame();
    }
}

internal void 
DoPauseMenu(menu_state *State, font *ItemFont, v2 P, f32 YAdvance){
    menu_page Page = MakeMenuPage(ItemFont, P, YAdvance);
    
    if(MenuPageDoText(State, &Page, "Resume") ||
       OSInput.KeyJustDown(KeyCode_Escape, KeyFlag_Any)){
        ChangeState(State->LastGameMode, String(0));
    }
    if(MenuPageDoText(State, &Page, "Settings")){
        State->LastPage = State->Page;
        State->Page = MenuPage_Settings;
    }
    if(MenuPageDoText(State, &Page, "Quit")){
        OSEndGame();
    }
}

internal void 
DoSettingsMenu(menu_state *State, font *ItemFont, v2 P, f32 YAdvance){
    menu_page Page = MakeMenuPage(ItemFont, P, YAdvance);
    
    AudioMixer.MasterVolume = V2(MenuPageDoSlider(State, &Page, "Volume", AudioMixer.MasterVolume.E[0]));
    if(MenuPageDoText(State, &Page, "Back") || 
       OSInput.KeyJustDown(KeyCode_Escape, KeyFlag_Any)){
        MenuChangePage(State, State->LastPage);
    }
}

internal void
UpdateAndRenderMenu(){
    GameRenderer.NewFrame(&TransientStorageArena, OSInput.WindowSize, MENU_BACKGROUND_COLOR);
    GameRenderer.SetLightingConditions(WHITE, 1.0f);
    GameRenderer.SetCameraSettings(0.5f);
    
    font *MenuTitleFont = FontSystem.GetFont(String("menu_title_font"), "asset_fonts/Merriweather/Merriweather-Black.ttf", 200);
    font *ItemFont = FontSystem.GetFont(String("menu_item_font"), "asset_fonts/Merriweather/Merriweather-Black.ttf", 75);
    
    v2 P = V2(OSInput.WindowSize.Width/2, OSInput.WindowSize.Height-200);
    f32 Padding = 10.0f;
    f32 FontSize = ItemFont->Ascent - ItemFont->Descent;
    f32 YAdvance = FontSize+Padding;
    
    RenderCenteredString(MenuTitleFont, WHITE, P, 0.0f, "Toe Tac Tic");
    P.Y -= MenuTitleFont->Size;
    
    switch(MenuState.Page){
        case MenuPage_Main: {
            DoMainMenu(&MenuState, ItemFont, P, YAdvance);
        }break;
        case MenuPage_Pause: {
            DoPauseMenu(&MenuState, ItemFont, P, YAdvance);
        }break;
        case MenuPage_Settings: {
            DoSettingsMenu(&MenuState, ItemFont, P, YAdvance);
        }break;
    }
    
    if(!MenuState.SetHovered) MenuState.Hovered = 0;
    MenuState.SetHovered = false;
    
    if(OSInput.MouseUp(MouseButton_Left, KeyFlag_Any)){
        MenuState.UsingSlider = 0;
    }
}