#ifndef SNAIL_JUMPY_MATH_H
#define SNAIL_JUMPY_MATH_H

// TODO(Tyler): Remove this dependency!
#include <math.h>

//~ Mathy stuff

global_constant f32 PI = 3.141592653589f;
global_constant f32 TAU = 2.0f*PI;

internal inline s32
RoundF32ToS32(f32 A)
{
 s32 Result;
 if(A < 0)
 {
  Result = (s32)(A - 0.5f);
 }
 else
 {
  Result = (s32)(A + 0.5f);
 }
 return(Result);
}

internal inline s32
Truncate(f32 A)
{
 return((s32)A);
}

internal inline f32
Floor(f32 A){
 return(floorf(A));
}

internal inline f32
Clamp(f32 Value, f32 Min, f32 Max){
 f32 Result = Value;
 if(Result < Min){
  Result = Min;
 }else if(Result > Max){
  Result = Max;
 }
 return(Result);
}

internal inline f32
Ceil(f32 A){
 return(ceilf(A));
}

internal inline f32
Round(f32 A)
{
 f32 Result;
 if(A < 0)
 {
  Result = Floor((A - 0.5f));
 }
 else
 {
  Result = Floor((A + 0.5f));
 }
 return(Result);
}

internal inline u32
CeilToS32(f32 A)
{
 u32 Result = (u32)ceilf(A);
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
Sin(f32 A)
{
 return(sinf(A));
}

internal inline f32
Cos(f32 A)
{
 return(cosf(A));
}

internal inline f32
Tan(f32 A)
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

internal inline s32
AbsoluteValue(s32 A)
{
 s32 Result = (A < 0) ? -A : A;
 return(Result);
}

internal inline f32
SignOf(f32 A){
 f32 Result = (A < 0) ? -1.0f : 1.0f;
 return(Result);
}

internal inline f32
ToPowerOf(f32 Base, f32 Exponent){
 f32 Result = powf(Base, Exponent);
 return(Result);
}

internal inline f32
SafeRatioN(f32 Numerator, f32 Denominator, f32 N){
 f32 Result = N;
 
 if(Denominator != 0.0f){
  Result = Numerator / Denominator;
 }
 
 return(Result);
}

internal inline f32
SafeRatio0(f32 Numerator, f32 Denominator){
 f32 Result = SafeRatioN(Numerator, Denominator, 0.0f);
 return(Result);
}

internal inline f32
SafeRatio1(f32 Numerator, f32 Denominator){
 f32 Result = SafeRatioN(Numerator, Denominator, 1.0f);
 return(Result);
}

internal inline u64
SafeRatioN(u64 Numerator, u64 Denominator, u64 N){
 u64 Result = N;
 
 if(Denominator != 0.0f){
  Result = Numerator / Denominator;
 }
 
 return(Result);
}

internal inline u64
SafeRatio0(u64 Numerator, u64 Denominator){
 u64 Result = SafeRatioN(Numerator, Denominator, 0);
 return(Result);
}

internal inline s32
Clamp(s32 Value, s32 Min, s32 Max){
 s32 Result = Value;
 if(Result < Min){
  Result = Min;
 }else if(Result > Max){
  Result = Max;
 }
 return(Result);
}

internal inline f32
Lerp(f32 A, f32 B, f32 T){
 T = Clamp(T, 0.0f, 1.0f);
 f32 Result = T*A + (1.0f-T)*B;
 return(Result);
}

internal inline b8
IsEven(s32 A){
 b8 Result = (A % 2) == 0;
 return(Result);
}

internal inline b8
IsOdd(s32 A){
 b8 Result = (A % 2) == 1;
 return(Result);
}

internal inline s32
NormalizeDegrees(s32 D){
 s32 Result;
 if(D < 0){
  Result = D;
  while(Result < 0) Result += 360; 
 }else{
  Result = D % 360;
 }
 return(Result);
}

//~ V2

union v2 {
 struct {
  f32 X;
  f32 Y;
 };
 struct {
  f32 Width;
  f32 Height;
 };
};

internal inline v2
V2(f32 X, f32 Y){ 
 v2 Result = v2{X, Y}; 
 return(Result);
}

internal inline v2
V2(f32 XY){ 
 v2 Result = V2(XY, XY); 
 return(Result);
}

// TODO(Tyler): Possibly implement operations for this?
union v2s {
 struct {
  s32 X;
  s32 Y;
 };
 struct {
  s32 Width;
  s32 Height;
 };
};

internal inline v2s
V2S(s32 X, s32 Y){ 
 v2s Result = v2s{X, Y}; 
 return(Result);
}

internal inline v2s
V2S(s32 XY){ 
 v2s Result = v2s{XY, XY}; 
 return(Result);
}

internal inline v2s
V2S(v2 A){ 
 v2s Result; 
 Result.X = (s32)A.X;
 Result.Y = (s32)A.Y;
 return(Result);
}

internal inline v2
V2(v2s A){ 
 v2 Result = v2{(f32)A.X, (f32)A.Y}; 
 return(Result);
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
V2Invert(v2 A)
{
 v2 Result;
 Result.X = -A.X;
 Result.Y = -A.Y;
 return(Result);
}

internal inline v2
operator*(v2 A, f32 B)
{
 v2 Result;
 Result.X = A.X * B;
 Result.Y = A.Y * B;
 return(Result);
}

internal inline v2
operator*(f32 B, v2 A)
{
 v2 Result;
 Result.X = A.X * B;
 Result.Y = A.Y * B;
 return(Result);
}

internal inline v2
operator/(v2 A, f32 B)
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
operator*=(v2 &A, f32 B)
{
 A = B * A;
 return(A);
}

internal inline v2
operator/=(v2 &A, f32 B)
{
 A = A / B;
 return(A);
}

internal inline f32
Dot(v2 A, v2 B) {
 f32 Result = (A.X*B.X)+(A.Y*B.Y);
 return(Result);
}

internal inline v2
Hadamard(v2 A, v2 B){
 v2 Result;
 Result.X = A.X*B.X;
 Result.Y = A.Y*B.Y;
 return(Result);
}

internal inline v2
Clockwise90(v2 A, v2 Origin=V2(0,0)){
 A -= Origin;
 v2 Result = V2(A.Y, -A.X);
 Result += Origin;
 return(Result);
}

internal inline v2
CounterClockwise90(v2 A, v2 Origin=V2(0,0)){
 A -= Origin;
 v2 Result = V2(-A.Y, A.X);
 Result += Origin;
 return(Result);
}

internal inline f32
LengthSquared(v2 V){
 f32 Result = Dot(V, V);
 return(Result);
}

internal inline f32
Length(v2 V){
 f32 Result = SquareRoot(LengthSquared(V));
 return(Result);
}

internal inline v2
Normalize(v2 V){
 f32 Length = SquareRoot(LengthSquared(V));
 v2 Result = {};
 if(Length > 0.0f){
  Result = V/Length;
 }
 return(Result);
}

// Perpendicular to A in the direction of B
internal inline v2 
TripleProduct(v2 A, v2 B){
 // A cross B cross A = (A cross B) cross A
 f32 Z = (A.X*B.Y)-(A.Y*B.X);
 v2 Result = V2(-Z*A.Y, Z*A.X);
 return(Result);
}

internal inline v2
MinimumV2(v2 A, v2 B){
 v2 Result;
 Result.X = Minimum(A.X, B.X);
 Result.Y = Minimum(A.Y, B.Y);
 
 return(Result);
}

internal inline v2
MaximumV2(v2 A, v2 B){
 v2 Result;
 Result.X = Maximum(A.X, B.X);
 Result.Y = Maximum(A.Y, B.Y);
 
 return(Result);
}

internal inline v2s
MinimumV2S(v2s A, v2s B){
 v2s Result;
 Result.X = Minimum(A.X, B.X);
 Result.Y = Minimum(A.Y, B.Y);
 
 return(Result);
}

internal inline v2s
MaximumV2S(v2s A, v2s B){
 v2s Result;
 Result.X = Maximum(A.X, B.X);
 Result.Y = Maximum(A.Y, B.Y);
 
 return(Result);
}

internal inline v2
SquareV2(v2 V){
 v2 Result;
 Result.X = V.X*V.X;
 Result.Y = V.Y*V.Y;
 return(Result);
}

internal inline v2
FloorV2(v2 V){
 v2 Result;
 Result.X = Floor(V.X);
 Result.Y = Floor(V.Y);
 return(Result);
}

internal inline v2
RoundV2(v2 V){
 v2 Result;
 Result.X = Round(V.X);
 Result.Y = Round(V.Y);
 return(Result);
}

internal inline v2
CeilV2(v2 V){
 v2 Result;
 Result.X = Ceil(V.X);
 Result.Y = Ceil(V.Y);
 return(Result);
}

internal inline v2
Lerp(v2 A, v2 B, f32 T){
 T = Clamp(T, 0.0f, 1.0f);
 v2 Result = T*A + (1.0f-T)*B;
 return(Result);
}

//~
union v3s {
 struct {
  s32 X;
  s32 Y;
  s32 Z;
 };
 struct {
  v2s XY;
  s32 Z;
 };
 struct {
  s32 Width;
  s32 Height;
  s32 Depth;
 };
};

internal inline v3s 
V3S(s32 X, s32 Y, s32 Z){
 v3s Result = {X, Y, Z};
 return Result;
}

internal inline v3s 
V3S(s32 XYZ){
 v3s Result = {XYZ, XYZ, XYZ};
 return Result;
}

internal inline v3s 
V3S(v2s XY, s32 Z){
 v3s Result = {XY.X, XY.Y, Z};
 return Result;
}

//~
union v4s {
 struct {
  s32 X;
  s32 Y;
  s32 Z;
  s32 W;
 };
 struct {
  v2s XY;
  v2s ZW;
 };
 struct {
  v3s XYZ;
  s32 W;
 };
};

internal inline v4s 
V4S(s32 X, s32 Y, s32 Z, s32 W){
 v4s Result = {X, Y, Z, W};
 return Result;
}

internal inline v4s 
V4S(s32 XYZW){
 v4s Result = {XYZW, XYZW, XYZW, XYZW};
 return Result;
}

internal inline v4s 
V4S(v2s XY, v2s ZW){
 v4s Result = {XY.X, XY.Y, ZW.X, ZW.Y};
 return Result;
}

//~ Colors
struct color {
 f32 R, G, B, A;
};

internal inline color
MakeColor(f32 R, f32 G, f32 B, f32 A=1.0f){
 color Result = color{R, G, B, A};
 return(Result);
}

internal inline color
MixColor(color A, color B, f32 Value){
 Value = Clamp(Value, 0.0f, 1.0f);
 color Result;
 Result.R = Value*A.R + (1.0f-Value)*B.R;
 Result.G = Value*A.G + (1.0f-Value)*B.G;
 Result.B = Value*A.B + (1.0f-Value)*B.B;
 Result.A = Value*A.A + (1.0f-Value)*B.A;
 return(Result);
}

internal inline color
Alphiphy(color Color, f32 Alpha){
 Alpha = Clamp(Alpha, 0.0f, 1.0f);
 color Result = Color;
 Result.A *= Alpha;
 return(Result);
}

internal inline color
operator*(f32 X, color Color)
{
 color Result;
 Result.R = X*Color.R;
 Result.G = X*Color.G;
 Result.B = X*Color.B;
 Result.A = X*Color.A;
 return(Result);
}

internal inline color
operator*(color Color, f32 X)
{
 color Result = X*Color;
 return(Result);
}

//~ HSB color
struct hsb_color {
 f32 Hue, Saturation, Brightness;
};

internal hsb_color 
HSBColor(f32 Hue, f32 Saturation, f32 Brightness){
 hsb_color Result = {Hue, Saturation, Brightness};
 return(Result);
}

internal color
HSBToRGB(hsb_color HSBColor){
 f32 Hue = Clamp(HSBColor.Hue, 0.0f, 360.0f);
 Hue /= 60.0f;
 f32 Saturation = Clamp(HSBColor.Saturation, 0.0f, 1.0f);
 f32 Brightness = Clamp(HSBColor.Brightness, 0.0f, 1.0f);
 
 
 f32 Chroma = Brightness*Saturation;
 f32 X      = Chroma * (1.0f - AbsoluteValue(ModF32(Hue, 2.0f) - 1.0f)); 
 
 color Result = {};
 u32 HueU32 = (u32)Hue;
 switch(HueU32){
  case 0: { Result = MakeColor(Chroma,      X,   0.0f); }break;
  case 1: { Result = MakeColor(     X, Chroma,   0.0f); }break;
  case 2: { Result = MakeColor(  0.0f, Chroma,      X); }break;
  case 3: { Result = MakeColor(  0.0f,      X, Chroma); }break;
  case 4: { Result = MakeColor(     X,   0.0f, Chroma); }break;
  case 5: 
  case 6: { Result = MakeColor(Chroma,   0.0f,      X); }break;
  default: { INVALID_CODE_PATH; }break;
 }
 
 f32 M = Brightness-Chroma;
 Result.R += M;
 Result.G += M;
 Result.B += M;
 
 return(Result);
}

//~ Rectangles
union rect {
 struct {
  v2 Min;
  v2 Max;
 };
 struct {
  v2 E[2];
 };
 struct {
  f32 X0, Y0;
  f32 X1, Y1;
 };
};

struct rect_s32 {
 v2s Min;
 v2s Max;
};

internal inline rect
operator+(rect A, v2 B){
 rect Result;
 Result.Min = A.Min + B;
 Result.Max = A.Max + B;
 return(Result);
}

internal inline rect
operator-(rect A, v2 B){
 rect Result;
 Result.Min = A.Min - B;
 Result.Max = A.Max - B;
 return(Result);
}

internal inline rect
operator*(rect A, f32 B){
 rect Result;
 Result.Min = A.Min * B;
 Result.Max = A.Max * B;
 return(Result);
}

internal inline rect
operator*(f32 B, rect A){
 rect Result;
 Result.Min = A.Min * B;
 Result.Max = A.Max * B;
 return(Result);
}

internal inline rect
operator/(rect A, f32 B){
 rect Result;
 Result.Min = A.Min / B;
 Result.Max = A.Max / B;
 return(Result);
}

internal inline rect
operator+=(rect &A, v2 B){
 A.Min += B;
 A.Max += B;
 return(A);
}

internal inline rect
operator-=(rect &A, v2 B){
 A.Min -= B;
 A.Max -= B;
 return(A);
}

internal inline rect
operator*=(rect &A, f32 B){
 A.Min *= B;
 A.Max *= B;
 return(A);
}

internal inline rect
operator/=(rect &A, f32 B){
 A.Min /= B;
 A.Max /= B;
 return(A);
}

internal inline rect
MakeRect(v2 Min, v2 Max){
 rect Result;
 Result.Min = Min;
 Result.Max = Max;
 return(Result);
}

internal inline rect_s32
RectS32(v2s Min, v2s Max){
 rect_s32 Result;
 Result.Min = Min;
 Result.Max = Max;
 return(Result);
}

internal inline rect_s32 
RectS32(rect Rect){
 rect_s32 Result;
 Result.Min.X = Truncate(Rect.Min.X);
 Result.Min.Y = Truncate(Rect.Min.Y);
 Result.Max.X = (s32)Ceil(Rect.Max.X);
 Result.Max.X = (s32)Ceil(Rect.Max.X);
 Result.Max.Y = (s32)Ceil(Rect.Max.Y);
 return(Result);
}

internal inline rect
CenterRect(v2 P, v2 Size){
 rect Result;
 Result.Min = P - 0.5f*Size;
 Result.Max = P + 0.5f*Size;
 return(Result);
}

internal inline rect
RectFix(rect Rect){
 rect Result = {};
 Result.Min.X = Minimum(Rect.Min.X, Rect.Max.X);
 Result.Min.Y = Minimum(Rect.Min.Y, Rect.Max.Y);
 Result.Max.X = Maximum(Rect.Min.X, Rect.Max.X);
 Result.Max.Y = Maximum(Rect.Min.Y, Rect.Max.Y);
 
 return(Result);
}

internal inline rect
TopLeftRect(v2 TopLeft, v2 Size){
 rect Result;
 Result.Min = V2(TopLeft.X,        TopLeft.Y-Size.Y);
 Result.Max = V2(TopLeft.X+Size.X, TopLeft.Y);
 return(Result);
}

internal inline rect
SizeRect(v2 Min, v2 Size){
 rect Result;
 Result.Min = Min;
 Result.Max = Min+Size;
 return(Result);
}

internal inline v2
RectSize(rect Rect){
 v2 Result = Rect.Max - Rect.Min;
 return(Result);
}

internal inline b8
IsPointInRect(v2 Point, rect Rect){
 b8 Result = ((Rect.Min.X < Point.X) && (Point.X < Rect.Max.X) &&
              (Rect.Min.Y < Point.Y) && (Point.Y < Rect.Max.Y));
 return(Result);
}

internal inline b8
DoRectsOverlap(rect A, rect B){
 b8 Result = ((A.Min.X <= B.Max.X) &&
              (B.Min.X <= A.Max.X) &&
              (A.Min.Y <= B.Max.Y) &&
              (B.Min.Y <= A.Max.Y));
 return(Result);
}

internal inline rect
RectGrow(rect Rect, f32 G){
 rect Result = Rect;
 Result.Min -= V2(G, G);
 Result.Max += V2(G, G);
 return(Result);
}

internal inline rect
RectGrow(rect Rect, v2 G){
 rect Result = Rect;
 Result.Min -= G;
 Result.Max += G;
 return(Result);
}

internal inline v2
RectCenter(rect Rect){
 v2 Result = {};
 v2 Size = RectSize(Rect);
 Result = Rect.Min + 0.5f*Size;
 
 return(Result);
}

internal inline rect
RectSweep(rect RectA, v2 Delta){
 rect RectB = RectA + Delta;
 rect Result;
 Result.Min = MinimumV2(RectA.Min, RectB.Min);
 Result.Max = MaximumV2(RectA.Max, RectB.Max);
 
 return(Result);
}

internal inline rect
RectLerp(rect A, rect B, f32 T){
 rect Result;
 Result.Min = Lerp(A.Min, B.Min, T);
 Result.Max = Lerp(A.Max, B.Max, T);
 return(Result);
}

#endif // SNAIL_JUMPY_MATH_H
