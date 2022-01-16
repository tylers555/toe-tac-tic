
//~ Helpers
internal inline void
OpenPauseMenu(){
    MenuState.LastGameMode = GameMode;
    MenuState.Page = MenuPage_Pause;
    ChangeState(GameMode_Menu, MakeString(0));
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

internal inline b8
MenuPageDoText(menu_page *Page, const char *Name){
    b8 Result = false;
    
    color TargetColor;
    if(IsPointInRect(OSInput.MouseP, Page->R)){
        if(OSInput.MouseJustDown(MouseButton_Left, KeyFlag_Any)){
            Result = true;
        }
        if(MenuState.Hovered != Name) MenuState.HoverT = 0.0f;
        MenuState.HoverT += MENU_FLASH_FACTOR*OSInput.dTime;
        MenuState.Hovered = Name;
        TargetColor = MENU_HOVER_COLOR;
    }else{
        TargetColor = MENU_BASE_COLOR;
    }
    
    //0.4f*Sin(MenuState.HoverT)+0.6f
    MenuState.HoverT = Clamp(MenuState.HoverT, 0.0f, 1.0f);
    color Color = MixColor(TargetColor, MENU_BASE_COLOR, MenuState.HoverT);
    RenderCenteredString(Page->Font, Color, Page->P, 0.0f, Name);
    
    Page->P.Y -= Page->YAdvance;
    Page->R -= V2(0, Page->YAdvance);
    
    return Result;
}

internal inline f32
MenuPageDoSlider(menu_page *Page, const char *Name, f32 CurrentValue){
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
    RenderRoundedRect(LineRect, 0.0f, 0.5f*Size, WHITE, UIItem(0));
    //RenderLineFrom(P, V2(Width, 0), 0.0f, 30.0f, WHITE, UIItem(0));
    
    color Color = PINK;
    if(MenuState.UsingSlider == Name){
        Result = (OSInput.MouseP.X - CursorBaseP.X) / EffectiveWidth;
        Result = Clamp(Result, 0.0f, 1.0f);
        Color = RED;
    }else if(!MenuState.UsingSlider && IsPointInRect(OSInput.MouseP, LineRect)){
        Color = BLUE;
        if(OSInput.MouseJustDown(MouseButton_Left, KeyFlag_Any)){
            MenuState.UsingSlider = Name;
            Color = RED;
        }
    }
    
    RenderRoundedRect(CursorRect, -1.0f, 0.5f*Size, Color, UIItem(0));
    
    Page->P.Y -= Page->YAdvance;
    Page->R -= V2(0, Page->YAdvance);
    
    return Result;
}

//~ 
internal void 
DoMainMenu(font *ItemFont, v2 P, f32 YAdvance){
    menu_page Page = MakeMenuPage(ItemFont, P, YAdvance);
    
    if(MenuPageDoText(&Page, "Start")){
        ChangeState(GameMode_MainGame, String(STARTUP_LEVEL));
    }
    if(MenuPageDoText(&Page, "Settings")){
        MenuState.LastPage = MenuState.Page;
        MenuState.Page = MenuPage_Settings;
    }
    if(MenuPageDoText(&Page, "Quit")){
        OSEndGame();
    }
}

internal void 
DoPauseMenu(font *ItemFont, v2 P, f32 YAdvance){
    menu_page Page = MakeMenuPage(ItemFont, P, YAdvance);
    
    if(MenuPageDoText(&Page, "Resume") ||
       OSInput.KeyJustDown(KeyCode_Escape, KeyFlag_Any)){
        ChangeState(MenuState.LastGameMode, String(0));
    }
    if(MenuPageDoText(&Page, "Settings")){
        MenuState.LastPage = MenuState.Page;
        MenuState.Page = MenuPage_Settings;
    }
    if(MenuPageDoText(&Page, "Quit")){
        OSEndGame();
    }
}

internal void 
DoSettingsMenu(font *ItemFont, v2 P, f32 YAdvance){
    menu_page Page = MakeMenuPage(ItemFont, P, YAdvance);
    
    AudioMixer.MasterVolume = V2(MenuPageDoSlider(&Page, "Volume", AudioMixer.MasterVolume.E[0]));
    if(MenuPageDoText(&Page, "Back") || 
       OSInput.KeyJustDown(KeyCode_Escape, KeyFlag_Any)){
        MenuState.Page = MenuState.LastPage;
    }
}

internal void
UpdateAndRenderMenu(){
    os_event Event;
    while(PollEvents(&Event)){
        ProcessDefaultEvent(&Event);
    }
    
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
            DoMainMenu(ItemFont, P, YAdvance);
        }break;
        case MenuPage_Pause: {
            DoPauseMenu(ItemFont, P, YAdvance);
        }break;
        case MenuPage_Settings: {
            DoSettingsMenu(ItemFont, P, YAdvance);
        }break;
    }
    
    if(OSInput.MouseUp(MouseButton_Left, KeyFlag_Any)){
        MenuState.UsingSlider = 0;
    }
}