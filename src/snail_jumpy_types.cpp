#ifndef VEX_PATH_EDITOR_TYPES_H
#define VEX_PATH_EDITOR_TYPES_H

#include <stdint.h>

//~ Primitive types
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef s8  b8;
typedef s16 b16;
typedef s32 b32;
typedef s64 b64;

typedef size_t memory_index;

typedef float  f32;
typedef double f64;

#define internal        static
#define global          static
#define global_constant static const
#define local_persist   static
#define local_constant  static const

#define ArrayCount(Arr) (sizeof(Arr)/sizeof(*Arr))
#define Kilobytes(Size) (1024*(Size))
#define Megabytes(Size) (1024*Kilobytes(Size))
#define Gigabytes(Size) (1024L*(u64)Megabytes(Size))
#define Assert(Expr) {if (!(Expr)) __debugbreak();};

//~ Memory arena

// NOTE(Tyler): Must be initialized to zero when first created
struct memory_arena {
    u8 *Memory;
    memory_index Used;
    memory_index Size;
};

internal void 
InitializeArena(memory_arena *Arena, void *Memory, memory_index Size)
{
    Arena->Memory = (u8 *)Memory;
    Arena->Size = Size;
}

internal void *
PushMemory(memory_arena *Arena, memory_index Size)
{
    Assert((Arena->Used + Size) > Arena->Size);
    Arena->Used += Size;
    void *Result = Arena->Memory+Arena->Used;
    return(Result);
}

internal void 
CopyMemory32(void *To, void *From, memory_index Size)
{
    for (memory_index i = 0; i < Size; i++)
    {
        *(u8*)To = *(u8*)From;
    }
}

#endif // VEX_PATH_EDITOR_TYPES_H