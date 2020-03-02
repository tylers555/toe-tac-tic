
#include "snail_jumpy.h"

internal void DrawRectangle(platform_backbuffer *Backbuffer, v2 Pos, v2 Size, u32 Color);

#include "snail_jumpy_world.cpp"

internal void
DrawRectangle(platform_backbuffer *Backbuffer, v2 Pos, v2 Size, u32 Color) {
    s32 MinX = (s32)Pos.X;
    s32 MinY = (s32)Pos.Y;
    s32 MaxX = (s32)(MinX+Size.X);
    s32 MaxY = (s32)(MinY+Size.Y);
    
    if(MinX < 0) MinX = 0;
    if(MinY < 0) MinY = 0;
    if(MaxX > Backbuffer->Width) MaxX = Backbuffer->Width;
    if(MaxY > Backbuffer->Height) MaxY = Backbuffer->Height;
    
    u8 *Row = (u8 *)Backbuffer->Memory + (u32)(MinX*BYTES_PER_PIXEL) + (u32)(MinY*Backbuffer->Pitch);
    for(s32 Y = MinY; Y < MaxY; Y++)
    {
        u32 *Pixel = (u32 *)Row;
        for(s32 X = MinX; X < MaxX; X++)
        {
            *Pixel++ = Color;
        }
        Row += Backbuffer->Pitch;
    }
}

internal loaded_bitmap
DEBUGLoadBitmapFromFile(thread_context *Thread, platform_api *PlatformAPI, const char *FilePath) {
    loaded_bitmap Result = {0};
    
    DEBUG_read_file_result BmpFile = PlatformAPI->ReadFile(Thread, FilePath);
    Assert(BmpFile.Data && BmpFile.Size);
    bitmap_header *BmpHeader = (bitmap_header *)BmpFile.Data;
    Result.Pixels = (u32 *)((u8 *)BmpFile.Data + BmpHeader->BitmapOffset);
    Result.Width = BmpHeader->Width;
    Result.Height = BmpHeader->Height;
    Assert((Result.Height > 0) && (Result.Width > 0));
    
    u32 RedMask = BmpHeader->RedMask;
    u32 GreenMask = BmpHeader->GreenMask;
    u32 BlueMask = BmpHeader->BlueMask;
    u32 AlphaMask = ~(RedMask | GreenMask | BlueMask);
    
    bit_scan_result RedShift = ScanForLeastSignificantSetBit(RedMask);
    bit_scan_result GreenShift = ScanForLeastSignificantSetBit(GreenMask);
    bit_scan_result BlueShift = ScanForLeastSignificantSetBit(BlueMask);
    bit_scan_result AlphaShift = ScanForLeastSignificantSetBit(AlphaMask);
    
    u32 *Dest = Result.Pixels;
    for(s32 Y = 0; Y < Result.Height; Y++)
    {
        for(s32 X = 0; X < Result.Width; X++)
        {
            u32 Color = *Dest;
            *Dest++ = ((((Color >> AlphaShift.Index) & 0xFF) << 24) |
                       (((Color >> RedShift.Index) & 0xFF) << 16) |
                       (((Color >> GreenShift.Index) & 0xFF) << 8) |
                       (((Color >> BlueShift.Index) & 0xFF) << 0));
        }
    }
    
    return(Result);
}

internal void
DrawBitmap(platform_backbuffer *Backbuffer, loaded_bitmap *Bitmap, v2 Pos) {
    s32 MinX = RoundF32ToS32(Pos.X);
    s32 MinY = RoundF32ToS32(Pos.Y);
    s32 MaxX = RoundF32ToS32(Pos.X + (f32)Bitmap->Width);
    s32 MaxY = RoundF32ToS32(Pos.Y + (f32)Bitmap->Height);
    
    s32 SourceOffsetX = 0;
    if(MinX < 0)
    {
        SourceOffsetX = -MinX;
        MinX = 0;
    }
    
    s32 SourceOffsetY = 0;
    if(MinY < 0)
    {
        SourceOffsetY = -MinY;
        MinY = 0;
    }
    
    if(MaxX > Backbuffer->Width)
    {
        MaxX = Backbuffer->Width;
    }
    
    if(MaxY > Backbuffer->Height)
    {
        MaxY = Backbuffer->Height;
    }
    
    u32 *SourceRow = Bitmap->Pixels + Bitmap->Width*(Bitmap->Height - 1);
    SourceRow += -SourceOffsetY*Bitmap->Width + SourceOffsetX;
    u8 *DestRow = ((u8 *)Backbuffer->Memory +
                   MinX*BYTES_PER_PIXEL +
                   MinY*Backbuffer->Pitch);
    for(int Y = MinY;
        Y < MaxY;
        ++Y)
    {
        u32 *Dest = (u32 *)DestRow;
        u32 *Source = SourceRow;
        for(int X = MinX;
            X < MaxX;
            ++X)
        {
            f32 A = (f32)((*Source >> 24) & 0xFF) / 255.0f;
            f32 SR = (f32)((*Source >> 16) & 0xFF);
            f32 SG = (f32)((*Source >> 8) & 0xFF);
            f32 SB = (f32)((*Source >> 0) & 0xFF);
            
            f32 DR = (f32)((*Dest >> 16) & 0xFF);
            f32 DG = (f32)((*Dest >> 8) & 0xFF);
            f32 DB = (f32)((*Dest >> 0) & 0xFF);
            
            f32 R = (1.0f-A)*DR + A*SR;
            f32 G = (1.0f-A)*DG + A*SG;
            f32 B = (1.0f-A)*DB + A*SB;
            
            *Dest = (((u32)(R + 0.5f) << 16) |
                     ((u32)(G + 0.5f) << 8) |
                     ((u32)(B + 0.5f) << 0));
            
            ++Dest;
            ++Source;
        }
        
        DestRow += Backbuffer->Pitch;
        SourceRow -= Bitmap->Width;
    }
}

// NOTE(Tyler): Y coordinates are also used in place for X coordinates
internal void
TestWall(f32 WallX, f32 PlayerX, f32 dPlayerX, f32 *CollisionTime) {
    f32 Epsilon = 0.001f;
    if(dPlayerX != 0.0f) {
        f32 CollisionTimeResult = (WallX - PlayerX) / dPlayerX;
        if((CollisionTimeResult >= 0.0f) && (*CollisionTime > CollisionTimeResult)) {
            *CollisionTime = Maximum(0.0f, CollisionTimeResult-Epsilon);
        }
    }
}

internal void
MovePlayer(world *World, entity *Player, v2 PlayerSize, v2 ddP, f32 dTimeForFrame) {
    ddP += -1.5f * Player->dP;
    v2 PlayerDelta = (ddP*Square(dTimeForFrame) + 
                      Player->dP*dTimeForFrame);
    Player->dP = ddP*dTimeForFrame + Player->dP;
    
    v2 NewPlayerP = Player->P;
    NewPlayerP += PlayerDelta;
    
    v2s OldTileCoords = GetTileCoordsFromPoint(Player->P);
    v2s NewTileCoords = GetTileCoordsFromPoint(NewPlayerP);
    
    u32 MinTileX = Minimum(OldTileCoords.X, NewTileCoords.X);
    u32 MinTileY = Minimum(OldTileCoords.Y, NewTileCoords.Y);
    u32 OnePastMaxTileX = Maximum(OldTileCoords.X, NewTileCoords.X) + 1;
    u32 OnePastMaxTileY = Maximum(OldTileCoords.Y, NewTileCoords.Y) + 1;
    
    f32 CollisionTime = 1.0f;
    if(PlayerDelta.X != 0.0f) {
        for(u32 TileY = MinTileY;
            TileY < OnePastMaxTileY;
            TileY++)
        {
            for(u32 TileX = MinTileX;
                TileX < OnePastMaxTileX;
                TileX++)
            {
                if(IsTileIDWall(GetTileIDAtPoint(World, {(f32)TileX, (f32)TileY}))) {
                    TestWall((f32)TileX, Player->P.X, PlayerDelta.X, &CollisionTime);
                    TestWall((f32)TileX+1.0f, Player->P.X, PlayerDelta.X, &CollisionTime);
                    TestWall((f32)TileY, Player->P.Y, PlayerDelta.Y, &CollisionTime);
                    TestWall((f32)TileY+1.0f, Player->P.Y, PlayerDelta.Y, &CollisionTime);
                }
            }
        }
    }
    
    Player->P += PlayerDelta*CollisionTime;
}

internal void 
GameUpdateAndRender(thread_context *Thread, game_memory *Memory, 
                    platform_api *PlatformAPI,
                    platform_user_input *Input, 
                    platform_backbuffer *Backbuffer) {
    Assert(Memory->PermanentStorageSize >= sizeof(game_state));
    if(!Memory->IsInitialized)
    {
        Memory->State = (game_state *)Memory->PermanentStorage;
        Memory->State->Player.P.X = 10.0f;
        Memory->State->Player.P.Y = Backbuffer->Height*(1.0f/60.0f);
        
        Memory->State->TestBitmap = DEBUGLoadBitmapFromFile(Thread, PlatformAPI, "test_background.bmp");
        Memory->State->PlayerBitmap = DEBUGLoadBitmapFromFile(Thread, PlatformAPI, "test_hero_front_head.bmp");
        
        Memory->IsInitialized = true;
    }
    game_state *GameState = Memory->State;
    
    v2 ScreenSizeInPixels = V2((f32)Backbuffer->Width, (f32)Backbuffer->Height);
    
    DrawRectangle(Backbuffer, {0, 0}, 
                  {(f32)Backbuffer->Width, (f32)Backbuffer->Height}, 0x00333333);
    
    u32 Map[13][21] = {
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1},
    };
    
    world World;
    World.TileMap = (u32 *)Map;
    World.XTiles = 21;
    World.YTiles = 13;
    World.ScreenXTiles = 16;
    World.ScreenYTiles = 9;
    World.TileSideInPixels = 60.0f;
    World.TileSideInMeters = 1.0f;
    
    f32 MetersToPixels = World.TileSideInPixels / World.TileSideInMeters;
    
    UpdateCameraPos(&World, GameState->Player.P, ScreenSizeInPixels); 
    
    DrawBitmap(Backbuffer, &GameState->TestBitmap, V2(World.TileSideInPixels, World.TileSideInPixels)-(World.CameraPos*MetersToPixels));
    
    DrawWorld(Backbuffer, &World);
    
    v2 PlayerAcc = {0};
    if (Input->UpButton.EndedDown)
    {
        PlayerAcc.Y--;
    }
    if (Input->DownButton.EndedDown)
    {
        PlayerAcc.Y++;
    }
    if(Input->RightButton.EndedDown)
    {
        PlayerAcc.X++;
    }
    if(Input->LeftButton.EndedDown)
    {
        PlayerAcc.X--;
    }
    
    f32 PlayerAccLength = LengthSquared(PlayerAcc);
    if (PlayerAccLength > 1.0f)
    {
        PlayerAcc *= 1.0f / SquareRoot(PlayerAccLength);
    }
    
    f32 PlayerSpeed = 10.0f;
    PlayerAcc *= PlayerSpeed;
    //PlayerAcc.Y -= -5.0f;
    
    f32 PlayerWidth = 0.7f;
    f32 PlayerHeight = 1.0f;
    
    MovePlayer(&World, &GameState->Player, V2(PlayerWidth, PlayerHeight), PlayerAcc, Input->dTimeForFrame);
    
    // NOTE(Tyler): The bottom center of the character is the tracked position
    v2 PlayerTopLeft = V2(GameState->Player.P.X - 0.5f*PlayerWidth,
                          GameState->Player.P.Y - PlayerHeight);
    DrawRectangleInWorld(Backbuffer, &World, PlayerTopLeft,
                         V2(PlayerWidth, PlayerHeight), 0x00FFFF00);
}
