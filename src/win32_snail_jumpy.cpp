#include <windows.h>

#ifdef CopyMemory
#undef CopyMemory
#endif

#include "snail_jumpy_types.cpp"
#include "win32_snail_jumpy.h"

#include "snail_jumpy_platform.h" // NOTE(Tyler): This is the OS platform layer

#include "snail_jumpy.cpp"

global b32 Running;
global f32 GlobalPerfCounterFrequency;
global Win32_Backbuffer GlobalBackbuffer;

// TODO(Tyler): Possibly do this differently
global platform_user_input UserInput;

// TODO(Tyler): Implement actual file I/O
DEBUG_READ_FILE_SIG(Win32ReadFile)
{
    DEBUG_read_file_result Result = {0};
    HANDLE File = CreateFileA(Path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if(File != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER FileSize = {0};
        if(GetFileSizeEx(File, &FileSize))
        {
            Result.Size = FileSize.QuadPart;
            
            Assert(Result.Size <= 0xffffffff);
            Result.Data = VirtualAlloc(0, Result.Size, MEM_COMMIT, PAGE_READWRITE);
            if (Result.Data)
            {
                DWORD BytesRead;
                if(ReadFile(File, Result.Data, (DWORD)Result.Size, &BytesRead, 0))
                {
                    // NOTE(Tyler): File read successful
                }
                else
                {
                    DWORD Error = GetLastError();
                    //VirtualFree(Result.Data, 0, MEM_RELEASE);
                    Result = {0};
                }
            }
            else
            {
                // TODO(Tyler): Error logging
            }
        }
        else
        {
            // TODO(Tyler): Error logging
        }
        CloseHandle(File);
    }
    else
    {
        // TODO(Tyler): Error logging
    }
    return(Result);
}

DEBUG_FREE_FILE_DATA_SIG(Win32FreeFileData)
{
    VirtualFree(FileData, 0, MEM_RELEASE);
}

DEBUG_WRITE_DATA_TO_FILE_SIG(Win32WriteDataToFile)
{
    Assert(Size <= 0xffffffff);
    HANDLE File = CreateFileA(Path, 
                              GENERIC_READ, 
                              FILE_SHARE_READ, 
                              0, 
                              CREATE_ALWAYS, 
                              FILE_ATTRIBUTE_NORMAL, 
                              0);
    DWORD BytesWritten;
    b32 Result = WriteFile(File, Data, (DWORD)Size, &BytesWritten, 0);
    
    CloseHandle(File);
    return(Result);
}

internal void
Win32ResizeDIBSection(Win32_Backbuffer *Buffer, int Width, int Height)
{
    if (Buffer->Memory)
    {
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }
    // NOTE(Tyler): We are locking the width and height
    Width = 960;
    Height = 540;
    
    Buffer->Width = Width;
    Buffer->Height = Height;
    Buffer->BytesPerPixel = 4;
    Buffer->Pitch = Width*Buffer->BytesPerPixel;
    
    Buffer->Info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    Buffer->Info.bmiHeader.biWidth = Buffer->Width;
    Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32;
    Buffer->Info.bmiHeader.biCompression = BI_RGB;
    
    s32 BitmapMemorySize = (Buffer->Width*Buffer->Height)*Buffer->BytesPerPixel;
    // NOTE(Tyler): VirtualAlloc initializes the memory to 0 so it doesn't need to be done
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
}

internal void
Win32CopyBackbufferToWindow(HDC DeviceContext, RECT *WindowRect, Win32_Backbuffer *Buffer)
{
    int WindowWidth = WindowRect->right - WindowRect->left;
    int WindowHeight = WindowRect->bottom - WindowRect->top;
    
    PatBlt(DeviceContext, 0, 0, WindowWidth, 20, BLACKNESS);
    PatBlt(DeviceContext, 0, 0, 20, WindowHeight, BLACKNESS);
    PatBlt(DeviceContext, 980, 0, WindowWidth-980, WindowHeight, BLACKNESS);
    PatBlt(DeviceContext, 0, 560, WindowWidth, WindowHeight-560, BLACKNESS);
    
    // NOTE(Tyler): We will only render 960x540 for now
    WindowWidth = 960;
    WindowHeight = 540;
    
    StretchDIBits(DeviceContext, 
#if 0
                  X, Y, Width, Height,
                  X, Y, Width, Height,
#endif
                  20, 20, WindowWidth, WindowHeight,
                  0, 0, Buffer->Width, Buffer->Height,
                  Buffer->Memory,
                  &Buffer->Info,
                  DIB_RGB_COLORS, SRCCOPY);
}

internal void
Win32ProcessKeyboardInput(platform_button_state *Button, b32 IsDown)
{
    if (Button->EndedDown != IsDown)
    {
        Button->EndedDown = IsDown;
        Button->HalfTransitionCount++;
    }
}

LRESULT CALLBACK 
Win32MainWindowProc(HWND Window,
                    UINT Message,
                    WPARAM WParam,
                    LPARAM LParam)
{
    LRESULT Result = 0;
    switch (Message)
    {
        case WM_ACTIVATEAPP:
        {
            
        }break;
        
        case WM_SIZE:
        {
            RECT ClientRect;
            GetClientRect(Window, &ClientRect);
            int Width = ClientRect.right - ClientRect.left;
            int Height = ClientRect.bottom - ClientRect.top;
            Win32ResizeDIBSection(&GlobalBackbuffer, Width, Height);
        }break;
        
        case WM_CLOSE:
        {
            Running = false;
        }break;
        
        case WM_DESTROY:
        {
            Running = false;
        }break;
        
        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            
            RECT WindowRect;
            GetClientRect(Window, &WindowRect);
            
            Win32CopyBackbufferToWindow(DeviceContext, &WindowRect, &GlobalBackbuffer);
            EndPaint(Window, &Paint);
        }break;
        
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            u32 VKCode = (u32)WParam;
            
            b32 WasDown = ((LParam & (1 << 30)) != 0);
            b32 IsDown = ((LParam & (1UL << 31)) == 0);
            if (WasDown != IsDown)
            {
                if (VKCode == 'W')
                {
                    Win32ProcessKeyboardInput(&UserInput.UpButton, IsDown);
                }
                else if (VKCode == 'S')
                {
                    Win32ProcessKeyboardInput(&UserInput.DownButton, IsDown);
                }
                else if (VKCode == 'A')
                {
                    Win32ProcessKeyboardInput(&UserInput.LeftButton, IsDown);
                }
                else if (VKCode == 'D')
                {
                    Win32ProcessKeyboardInput(&UserInput.RightButton, IsDown);
                }
                else if (VKCode == ' ')
                {
                    Win32ProcessKeyboardInput(&UserInput.JumpButton, IsDown);
                }
            }
        }break;
        
        default:
        {
            Result = DefWindowProc(Window, Message, WParam, LParam);
        }break;
    }
    return(Result);
}

internal LARGE_INTEGER
Win32GetWallClock()
{
    LARGE_INTEGER Result;
    QueryPerformanceCounter(&Result);
    return(Result);
}

internal f32
Win32SecondsElapsed(LARGE_INTEGER Begin, LARGE_INTEGER End)
{
    f32 Result = ((f32)End.QuadPart - (f32)Begin.QuadPart)/GlobalPerfCounterFrequency;
    return(Result);
}

int CALLBACK
WinMain(HINSTANCE Instance,
        HINSTANCE PrevInstance,
        LPSTR CommandLine,
        int ShowCode)
{
    WNDCLASS WindowClass = {0};
    
    WindowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
    WindowClass.lpfnWndProc = Win32MainWindowProc;
    WindowClass.hInstance = Instance;
    //WindowClass.hIcon = ...;
    WindowClass.lpszClassName = "SnailJumpyWindowClass";
    
    if (RegisterClass(&WindowClass))
    {
        // NOTE(Tyler): The window is made at whatever size but we only render 960 x 540
        HWND Window = CreateWindowEx(0,
                                     WindowClass.lpszClassName,
                                     "Snail Jumpy",
                                     WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                                     CW_USEDEFAULT, CW_USEDEFAULT,
                                     CW_USEDEFAULT, CW_USEDEFAULT,
                                     0, 
                                     0,
                                     Instance,
                                     0);
        if (Window)
        {
            LARGE_INTEGER PerformanceCounterFrequencyResult;
            QueryPerformanceFrequency(&PerformanceCounterFrequencyResult);
            GlobalPerfCounterFrequency = (f32)PerformanceCounterFrequencyResult.QuadPart;
            
            LARGE_INTEGER LastCounter = Win32GetWallClock();
            f32 TargetSecondsPerFrame = 1.0f/30.0f;
            
            game_memory GameMemory = {0};
            GameMemory.PermanentStorageSize = Megabytes(4);
            GameMemory.PermanentStorage = VirtualAlloc(0, GameMemory.PermanentStorageSize, MEM_COMMIT, PAGE_READWRITE);
            GameMemory.TransientStorageSize = Gigabytes(4);
            GameMemory.TransientStorage = VirtualAlloc(0, GameMemory.TransientStorageSize
                                                       , MEM_COMMIT, PAGE_READWRITE);
            
            Running = true;
            while (Running)
            {
                MSG Message;
                while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
                {
                    if (Message.message == WM_QUIT)
                    {
                        Running = false;
                    }
                    
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }
                
                UserInput.dTimeForFrame = TargetSecondsPerFrame;
                
                platform_backbuffer GameBackbuffer;
                GameBackbuffer.Memory = GlobalBackbuffer.Memory;
                GameBackbuffer.Width = GlobalBackbuffer.Width;
                GameBackbuffer.Height = GlobalBackbuffer.Height;
                GameBackbuffer.Pitch = GlobalBackbuffer.Pitch;
                
                platform_api PlatformAPI;
                PlatformAPI.ReadFile = Win32ReadFile;
                PlatformAPI.FreeFileData = Win32FreeFileData;
                PlatformAPI.WriteDataToFile = Win32WriteDataToFile;
                
                // TODO(Tyler): Multithreading
                thread_context Thread = {0};
                GameUpdateAndRender(&Thread, &GameMemory, &PlatformAPI, &UserInput, &GameBackbuffer);
                
                
                f32 SecondsElapsed = Win32SecondsElapsed(LastCounter, Win32GetWallClock());
                if (SecondsElapsed < TargetSecondsPerFrame)
                {
                    while (SecondsElapsed < TargetSecondsPerFrame)
                    {
                        DWORD SleepMS = (DWORD)(1000.0f * (TargetSecondsPerFrame-SecondsElapsed));
                        Sleep(SleepMS);
                        SecondsElapsed = Win32SecondsElapsed(LastCounter, Win32GetWallClock());
                    }
                }
                else
                {
                    // TODO(Tyler): Error logging
                }
                LastCounter = Win32GetWallClock();
                
                HDC DeviceContext = GetDC(Window);
                RECT WindowRect;
                GetClientRect(Window, &WindowRect);
                Win32CopyBackbufferToWindow(DeviceContext, &WindowRect, &GlobalBackbuffer);
                ReleaseDC(Window, DeviceContext);
                
            }
        }
        else
        {
            // TODO(Tyler): Error logging!
            OutputDebugString("Failed to create window!");
        }
        
    }
    else
    {
        // TODO(Tyler): Error logging!
        OutputDebugString("Failed to register window class!");
    }
    
    return(0);
}