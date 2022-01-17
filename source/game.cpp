
internal void
GameProcessHotKeys(){
    if(OSInput.KeyJustDown('E', KeyFlag_Control)) ToggleWorldEditor(); 
}

internal void
UpdateAndRenderMainGame(){
    TIMED_FUNCTION();
    
    GameProcessHotKeys();
    EntityManager.HandleInput();
    
    GameRenderer.NewFrame(&TransientStorageArena, OSInput.WindowSize, MakeColor(0.30f, 0.55f, 0.70f));
    GameRenderer.CalculateCameraBounds(CurrentWorld); 
    GameRenderer.SetCameraSettings(0.5f);
    GameRenderer.SetLightingConditions(HSBToRGB(CurrentWorld->AmbientColor), CurrentWorld->Exposure);
    
    EntityManager.UpdateEntities();
    EntityManager.RenderEntities();
    
    entity_player *Player = EntityManager.Player;
    
    if(CompletionCooldown > 0.0f){
        f32 Advance =
            GetFormatStringAdvance(&MainFont, 
                                   "Level completed");
        v2 TopCenter = v2{
            OSInput.WindowSize.Width/2, OSInput.WindowSize.Height/2
        };
        color Color = GREEN;
        if(CompletionCooldown > 0.5f*3.0f){
            Color.A = 2.0f*(1.0f - CompletionCooldown/3.0f);
        }else if(CompletionCooldown < 0.3f*3.0f){
            Color.A = 2.0f * CompletionCooldown/3.0f;
        }
        RenderFormatString(&MainFont, Color, 
                           V2(TopCenter.X-(0.5f*Advance), TopCenter.Y), -0.9f,
                           "Level completed!");
        
        CompletionCooldown -= OSInput.dTime;
        if(CompletionCooldown < 0.00001f){
            CurrentWorld->Flags |= WorldFlag_IsCompleted;
            CompletionCooldown = 0.0f;
            //ChangeState(GameMode_MainGame, "Overworld");
        }
    }
    
    //~ Health display
    {
        v2 P = V2(10.0f, 10.0f);
        f32 XAdvance = 10.0f;
        
        u32 FullHearts = Player->Health / 3;
        u32 Remainder = Player->Health % 3;
        
        asset_sprite_sheet *Asset = AssetSystem.GetSpriteSheet(Strings.GetString("heart"));
        Assert(FullHearts <= 3);
        u32 I;
        for(I = 0; I < FullHearts; I++){
            RenderSpriteSheetAnimationFrame(Asset, P, -0.9f, 0, 1, 0);
            P.X += XAdvance;
        }
        
        if(Remainder > 0){
            Remainder = 3 - Remainder;
            RenderSpriteSheetAnimationFrame(Asset, P, -0.9f, 0, 1, Remainder);
            P.X += XAdvance;
            I++;
        }
        
        if(I < 3){
            for(u32 J = 0; J < 3-I; J++){
                RenderSpriteSheetAnimationFrame(Asset, P, -0.9f, 0, 1, 3);
                P.X += XAdvance;
            }
        }
    }
    
#if 0
    //~ Rope/vine thing
    {
        v2 BaseP = V2(100, 110);
        
        f32 FinalT = (0.5f*Sin(2*Counter))+0.5f;
        f32 MinAngle = 0.4*PI;
        f32 MaxAngle = 0.6f*PI;
        f32 Angle = Lerp(MinAngle, MaxAngle, FinalT);
        v2 Delta = 50.0f*V2(Cos(Angle), -Sin(Angle));
        
        RenderLineFrom(BaseP, Delta, 0.0f, 1.0f, GREEN, GameItem(1));
        GameRenderer.AddLight(BaseP+Delta, MakeColor(0.0f, 1.0f, 0.0f), 0.3f, 5.0f, GameItem(1));
    }
#endif
    
    
    RenderFormatString(&MainFont, GREEN, V2(100, OSInput.WindowSize.Height-100),
                       -0.9f, "Score: %u", Score);
}