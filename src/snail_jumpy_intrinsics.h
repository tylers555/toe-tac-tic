struct bit_scan_result {
    b32 Found;
    u32 Index;
};

#if defined(_MSC_VER)
#include <intrin.h>
#pragma intrinsic(_BitScanForward)

internal inline bit_scan_result
ScanForLeastSignificantSetBit(u32 Mask)
{
    bit_scan_result Result;
    Result.Found = _BitScanForward64(&(DWORD)Result.Index, Mask);
    return(Result);
}
#else
#error Please implement ScanForLeastSignificantSetBit for this compiler!
#endif