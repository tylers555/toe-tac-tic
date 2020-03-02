
//#include "snail_jumpy_platform.h"
#include "snail_jumpy_math.h"
#include "snail_jumpy_intrinsics.h"

#pragma pack(push, 1)
struct bitmap_header
{
    // 14-byte header
    u16 FileType; // 2
    u32 FileSize; // 6
    u16 Reserved1; // 8
    u16 Reserved2; // 10
    u32 BitmapOffset; // 14
    
    // Bitmap header
    u32 HeaderSize; // 18 // Size of bitmap header, not entire header!
    s32 Width; // 22
    s32 Height; // 26
    u16 Planes; // 28
    u16 BitsPerPixel; // 30 
    u32 Compression;
    u32 SizeOfBitmap;
    s32 HorzResolution;
    s32 VertResolution;
    u32 ColorsUsed;
    u32 ColorsImportant;
    
    // Color bitmasks
    u32 RedMask;
    u32 GreenMask;
    u32 BlueMask;
};
#pragma pack(pop)

struct loaded_bitmap
{
    u32 *Pixels;
    s32 Width, Height;
};

typedef u32 tile_id;

struct world 
{
    u32 *TileMap;
    u32 XTiles, YTiles;
    u32 ScreenXTiles, ScreenYTiles;
    f32 TileSideInPixels, TileSideInMeters;
    v2 CameraPos;
};

struct entity
{
    v2 P, dP;
};

struct game_state
{
    entity Player;
    loaded_bitmap TestBitmap;
    loaded_bitmap PlayerBitmap;
};
