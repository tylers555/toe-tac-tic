
internal void 
UpdateCameraPos(world *World, v2 Pos, v2 ScreenSizeInPixels) {
    f32 PixelToMeters = World->TileSideInMeters / World->TileSideInPixels;
    f32 ScreenCenterXInMeters = (ScreenSizeInPixels.X/2) * PixelToMeters;
    f32 ScreenCenterYInMeters = (ScreenSizeInPixels.Y/2) * PixelToMeters;
    
    World->CameraPos = Pos-V2(ScreenCenterXInMeters, ScreenCenterYInMeters);
    
    u32 MaximumXTile = World->XTiles-World->ScreenXTiles;
    f32 MaximumWorldXPos = MaximumXTile*World->TileSideInMeters;
    if(World->CameraPos.X > MaximumWorldXPos)
    {
        World->CameraPos.X = MaximumWorldXPos;
    }
    else if(World->CameraPos.X < 0)
    {
        World->CameraPos.X = 0;
    }
    
    u32 MaximumYTile = World->YTiles-World->ScreenYTiles;
    f32 MaximumWorldYPos = MaximumYTile*World->TileSideInMeters;
    if(World->CameraPos.Y > MaximumWorldYPos)
    {
        World->CameraPos.Y = MaximumWorldYPos;
    }
    else if(World->CameraPos.Y < 0)
    {
        World->CameraPos.Y = 0;
    }
}

internal void
DrawWorld(platform_backbuffer *Backbuffer, world *World) {
    f32 MetersToPixels = World->TileSideInPixels / World->TileSideInMeters;
    
    f32 DrawRow = -World->CameraPos.Y*MetersToPixels;
    u32 *Tile = World->TileMap;
    for (u32 Y = 0; Y < World->YTiles; Y++)
    {
        f32 DrawTile = -World->CameraPos.X*MetersToPixels;
        for (u32 X = 0; X < World->XTiles; X++)
        {
            u32 TileID = *Tile;
            if(TileID == 1)
            {
                DrawRectangle(Backbuffer, {DrawTile, DrawRow}, 
                              {World->TileSideInPixels, World->TileSideInPixels}, 0x00FFFFFF);
            }
            Tile++;
            DrawTile += World->TileSideInPixels;
        }
        DrawRow += World->TileSideInPixels;
    }
}

internal void
DrawRectangleInWorld(platform_backbuffer *Backbuffer, world *World, v2 Pos, v2 Size, u32 Color) {
    Pos -= World->CameraPos;
    f32 MetersToPixels = World->TileSideInPixels / World->TileSideInMeters;
    DrawRectangle(Backbuffer, Pos*MetersToPixels, Size*MetersToPixels, Color);
}

internal inline v2s
GetTileCoordsFromPoint(v2 Point) {
    v2s Result;
    Result.X = TruncateF32ToS32(Point.X);
    Result.Y = TruncateF32ToS32(Point.Y);
    return(Result);
}

internal inline tile_id
GetTileIDAtPoint(world *World, v2 Point) {
    f32 MetersToPixels = World->TileSideInPixels / World->TileSideInMeters;
    v2s Tile = GetTileCoordsFromPoint(Point);
    s32 TileIndex = (Tile.Y*World->XTiles) + Tile.X;
    u32 Result = World->TileMap[TileIndex];
    return(Result);
}

internal inline b32
IsTileIDWall(tile_id TileID) {
    b32 Result = (TileID == 1);
    return(Result);
}
