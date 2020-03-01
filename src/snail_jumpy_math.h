#ifndef SNAIL_JUMPY_MATH_H
#define SNAIL_JUMPY_MATH_H

// TODO(Tyler): Remove this dependency!
#include <math.h>

//~ Helper functions

// TODO(Tyler): Check the correctness of this
internal inline s32
RoundF32ToS32(f32 A)
{
    s32 Result = (s32)(A + 0.5f);
    return(Result);
}

internal inline s32
TruncateF32ToS32(f32 A)
{
    return((s32)A);
}

internal inline u32
TruncateF32ToU32(f32 A)
{
    return((u32)A);
}

template <typename Type> 
internal inline Type 
Minimum(Type A, Type B)
{
    Type Result = (A > B) ? B : A;
    return(Result);
}

template <typename Type> 
internal inline Type 
Maximum(Type A, Type B)
{
    Type Result = (A > B) ? A : B;
    return(Result);
}

internal inline f32
Square(f32 A) 
{
    f32 Result = A*A;
    return(Result);
}

internal inline f32
SquareRoot(f32 A)
{
    f32 Result = sqrtf(A);
    return(Result);
}

internal inline f32
Sine(f32 A)
{
    return(sinf(A));
}

internal inline f32
Cosine(f32 A)
{
    return(cosf(A));
}

internal inline f32
Tangent(f32 A)
{
    return(tanf(A));
}

internal inline f32
ModF32(f32 A, f32 B)
{
    f32 Result = (f32)fmod(A, B);
    return(Result);
}

internal inline f32
AbsoluteValue(f32 A)
{
    f32 Result = (A < 0) ? -A : A;
    return(Result);
}

//~ Vectors

union v2 {
    struct 
    {
        f32 X, Y;
    };
    struct
    {
        f32 Width, Height;
    };
};

// TODO(Tyler): Possibly implement operations for this?
union v2s {
    struct 
    {
        s32 X, Y;
    };
    struct
    {
        s32 Width, Height;
    };
};

internal inline v2
V2(f32 X, f32 Y)
{
    v2 Result;
    Result.X = X;
    Result.Y = Y;
    return Result;
}

internal inline v2
V2(v2s V)
{
    v2 Result;
    Result.X = (f32)V.X;
    Result.Y = (f32)V.Y;
    return Result;
}

internal inline v2 
operator+(v2 A, v2 B)
{
    v2 Result;
    Result.X = A.X + B.X;
    Result.Y = A.Y + B.Y;
    return(Result);
}

internal inline v2 
operator-(v2 A, v2 B)
{
    v2 Result;
    Result.X = A.X - B.X;
    Result.Y = A.Y - B.Y;
    return(Result);
}

internal inline v2 
operator-(v2 A)
{
    v2 Result;
    Result.X = -A.X;
    Result.Y = -A.Y;
    return(Result);
}

internal inline v2 
operator*(v2 A, float B)
{
    v2 Result;
    Result.X = A.X * B;
    Result.Y = A.Y * B;
    return(Result);
}

internal inline v2 
operator*(float B, v2 A)
{
    v2 Result;
    Result.X = A.X * B;
    Result.Y = A.Y * B;
    return(Result);
}

internal inline v2 
operator/(v2 A, float B)
{
    v2 Result;
    Result.X = A.X / B;
    Result.Y = A.Y / B;
    return(Result);
}

internal inline v2 
operator/(float B, v2 A)
{
    v2 Result;
    Result.X = A.X / B;
    Result.Y = A.Y / B;
    return(Result);
}

internal inline v2 
operator+=(v2 &A, v2 B)
{
    A = A + B;
    return(A);
}

internal inline v2 
operator-=(v2 &A, v2 B)
{
    A = A - B;
    return(A);
}

internal inline v2 
operator*=(v2 &A, float B)
{
    A = A * B;
    return(A);
}

internal inline float
Inner(v2 A, v2 B)
{
    float Result = (A.X*B.X)+(A.Y*B.Y);
    return(Result);
}

internal inline f32
LengthSquared(v2 Vec)
{
    f32 Result = Inner(Vec, Vec);
    return(Result);
}



#endif // SNAIL_JUMPY_MATH_H
