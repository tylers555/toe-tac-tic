
#include <windows.h>
#include <gl/gl.h>
#include <audioclient.h>
#include <mmdeviceapi.h>
#include <initguid.h>

#ifdef CopyMemory
#undef CopyMemory
#endif

#ifdef ZeroMemory
#undef ZeroMemory
#endif

#ifdef MoveMemory
#undef MoveMemory
#endif

#include "main.cpp"


#include "os_win32.h"
#include "opengl_renderer.cpp"

global b32 Running;
global f32 GlobalPerfCounterFrequency;
global HWND MainWindow;
global WINDOWPLACEMENT GlobalWindowPlacement = {sizeof(GlobalWindowPlacement)};

global IAudioClient *AudioClient;
global IAudioRenderClient *AudioRenderClient;

global u32 Win32SoundCursor;

internal os_key_code
Win32ConvertVKCode(u32 VKCode){
    if(('0' <= VKCode) && (VKCode <= 'Z')) {
        return (os_key_code) VKCode;
    }else{
        switch(VKCode){
            //~ Special keys
            case VK_UP:        return KeyCode_Up;        
            case VK_DOWN:      return KeyCode_Down;      
            case VK_LEFT:      return KeyCode_Left;      
            case VK_RIGHT:     return KeyCode_Right;     
            case VK_SPACE:     return KeyCode_Space;     
            case VK_TAB:       return KeyCode_Tab;       
            case VK_CONTROL:   return KeyCode_Control;   
            case VK_SHIFT:     return KeyCode_Shift;     
            case VK_ESCAPE:    return KeyCode_Escape;    
            case VK_BACK:      return KeyCode_BackSpace; 
            case VK_DELETE:    return KeyCode_Delete;    
            case VK_RETURN:    return KeyCode_Return;    
            case VK_MENU:      return KeyCode_Alt;       
            case VK_F1:        return KeyCode_F1;        
            case VK_F2:        return KeyCode_F2;        
            case VK_F3:        return KeyCode_F3;        
            case VK_F4:        return KeyCode_F4;        
            case VK_F5:        return KeyCode_F5;        
            case VK_F6:        return KeyCode_F6;        
            case VK_F7:        return KeyCode_F7;        
            case VK_F8:        return KeyCode_F8;        
            case VK_F9:        return KeyCode_F9;        
            case VK_F10:       return KeyCode_F10;       
            case VK_F11:       return KeyCode_F11;       
            case VK_F12:       return KeyCode_F12;       
            
            //~ Normal ascii
            
            case VK_OEM_1:      return (os_key_code)';'; 
            case VK_OEM_PLUS:   return (os_key_code)'='; 
            case VK_OEM_COMMA:  return (os_key_code)','; 
            case VK_OEM_MINUS:  return (os_key_code)'-'; 
            case VK_OEM_PERIOD: return (os_key_code)'.'; 
            case VK_OEM_2:      return (os_key_code)'/'; 
            case VK_OEM_3:      return (os_key_code)'`'; 
            case VK_OEM_4:      return (os_key_code)'['; 
            case VK_OEM_5:      return (os_key_code)'\\'; 
            case VK_OEM_6:      return (os_key_code)']'; 
            case VK_OEM_7:      return (os_key_code)'\''; 
        }
    }
}

internal void
ToggleFullscreen(HWND Window){
    // NOTE(Tyler): Raymond Chen fullscreen code:
    // https://devblogs.microsoft.com/oldnewthing/20100412-00/?p=14353
    DWORD Style = GetWindowLong(Window, GWL_STYLE);
    if(Style & WS_OVERLAPPEDWINDOW){
        MONITORINFO MonitorInfo = {sizeof(MonitorInfo)};
        if(GetWindowPlacement(Window, &GlobalWindowPlacement) &&
           GetMonitorInfo(MonitorFromWindow(Window, MONITOR_DEFAULTTOPRIMARY), &MonitorInfo)){
            SetWindowLong(Window, GWL_STYLE, Style & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(Window, HWND_TOP,
                         MonitorInfo.rcMonitor.left, MonitorInfo.rcMonitor.top,
                         MonitorInfo.rcMonitor.right- MonitorInfo.rcMonitor.left,
                         MonitorInfo.rcMonitor.bottom- MonitorInfo.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    }else{
        SetWindowLong(Window, GWL_STYLE, Style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(Window, &GlobalWindowPlacement);
        SetWindowPos(Window, 0, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
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
        case WM_SETCURSOR: {
            HCURSOR Cursor = LoadCursorA(0, IDC_ARROW);
            SetCursor(Cursor);
        }break;
        case WM_SIZE: {
            RECT ClientRect;
            GetClientRect(Window, &ClientRect);
            int Width = ClientRect.right - ClientRect.left;
            int Height = ClientRect.bottom - ClientRect.top;
            OSInput.WindowSize = {(f32)Width, (f32)Height};
        }break;
        case WM_CLOSE: {
            Running = false;
        }break;
        case WM_DESTROY: {
            Running = false;
        }break;
        case WM_KILLFOCUS: case WM_SETFOCUS: case WM_ACTIVATE: {
            u8 KeyStates[256];
            GetKeyboardState(KeyStates);
            
        }break;
        default: {
            Result = DefWindowProcA(Window, Message, WParam, LParam);
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
Win32SecondsElapsed(LARGE_INTEGER Begin, LARGE_INTEGER End){
    f32 Result = ((f32)End.QuadPart - (f32)Begin.QuadPart)/GlobalPerfCounterFrequency;
    return(Result);
}

internal b32
Win32LoadOpenGlFunctions(){
    b32 Result = true;
    s32 CurrentFunction = 0;
#define X(Name) Name = (type_##Name *)wglGetProcAddress(#Name); \
if(!Name) { Assert(0); Result = false; } \
CurrentFunction++;
    
    OPENGL_FUNCTIONS;
    
#undef X
    return(Result);
}

internal void
Win32InitOpenGl(HINSTANCE Instance, HWND *Window){
    HDC DeviceContext = GetDC(*Window);
    
    PIXELFORMATDESCRIPTOR PixelFormatDescriptor = {};
    PixelFormatDescriptor.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    PixelFormatDescriptor.nVersion = 1;
    PixelFormatDescriptor.dwFlags =
        PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER;
    PixelFormatDescriptor.cColorBits = 32;
    PixelFormatDescriptor.cAlphaBits = 8;
    PixelFormatDescriptor.iLayerType = PFD_MAIN_PLANE;
    
    int PixelFormat = ChoosePixelFormat(DeviceContext, &PixelFormatDescriptor);
    PIXELFORMATDESCRIPTOR ActualPixelFormatDescriptor;
    DescribePixelFormat(DeviceContext, PixelFormat, sizeof(ActualPixelFormatDescriptor), &ActualPixelFormatDescriptor);
    
    if(SetPixelFormat(DeviceContext, PixelFormat, &ActualPixelFormatDescriptor)){
        HGLRC OpenGlContext = wglCreateContext(DeviceContext);
        if(wglMakeCurrent(DeviceContext, OpenGlContext)){
            wgl_choose_pixel_format_arb *wglChoosePixelFormatARB = (wgl_choose_pixel_format_arb*)wglGetProcAddress("wglChoosePixelFormatARB");
            wgl_create_context_attribs_arb *wglCreateContextAttribsARB = (wgl_create_context_attribs_arb*)wglGetProcAddress("wglCreateContextAttribsARB");
            if(wglChoosePixelFormatARB && wglCreateContextAttribsARB){
                wglMakeCurrent(DeviceContext, 0);
                Assert(wglDeleteContext(OpenGlContext));
                Assert(ReleaseDC(*Window, DeviceContext));
                Assert(DestroyWindow(*Window));
                
                *Window = CreateWindowEx(0,
                                         "WindowClass",
                                         "Toe Tac Tic",
                                         WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                                         CW_USEDEFAULT, CW_USEDEFAULT,
                                         CW_USEDEFAULT, CW_USEDEFAULT,
                                         0,
                                         0,
                                         Instance,
                                         0);
                
                DeviceContext = GetDC(*Window);
                
                const s32 AttributeList[] =
                {
                    WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
                    WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
                    WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
                    WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
                    WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
                    WGL_COLOR_BITS_ARB, 32,
                    WGL_ALPHA_BITS_ARB, 8,
                    WGL_DEPTH_BITS_ARB, 24,
                    WGL_STENCIL_BITS_ARB, 8,
                    WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
                    WGL_SAMPLES_ARB, 4,
                    0
                };
                
                s32 PixelFormat;
                u32 TotalFormats;
                if(wglChoosePixelFormatARB(DeviceContext, AttributeList, 0, 1, &PixelFormat,
                                           &TotalFormats)){
                    PIXELFORMATDESCRIPTOR PixelFormatDescriptor;
                    DescribePixelFormat(DeviceContext, PixelFormat, 
                                        sizeof(PixelFormatDescriptor), 
                                        &PixelFormatDescriptor);
                    if(SetPixelFormat(DeviceContext, PixelFormat, &PixelFormatDescriptor)){
                        const s32 OpenGLAttributes[] = {
                            WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
                            WGL_CONTEXT_MINOR_VERSION_ARB, 3,
                            WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
                            0,
                        };
                        
                        HGLRC OpenGLContext = wglCreateContextAttribsARB(DeviceContext, 0, OpenGLAttributes);
                        if(OpenGLContext){
                            if(wglMakeCurrent(DeviceContext, OpenGLContext)){
                                if(Win32LoadOpenGlFunctions()){
                                    // NOTE(Tyler): Success!!!
                                    
                                }else{
                                    LogMessage("Win32: Couldn't load OpenGL functions");
                                    Assert(0);
                                }
                            }else{
                                LogMessage("Win32: Couldn't make OpenGL context current 2");
                                Assert(0);
                            }
                        }else{
                            LogMessage("Win32: Couldn't create OpenGL context");
                            Assert(0);
                        }
                    }else{
                        LogMessage("Win32: Couldn't set pixel format");
                        Assert(0);
                    }
                }else{
                    // TODO(Tyler): Logging!!!
                    LogMessage("Win32: Couldn't choose pixel format 2");
                    Assert(0);
                }
            }else{
                LogMessage("Win32: Couldn't choose pixel format 1");
                Assert(0);
            }
        }else{
            LogMessage("Win32: Couldn't make OpenGL context current 1");
            Assert(0);
        }
    }else{
        // TODO(Tyler): Logging!!!
        LogMessage("Win32: Couldn't set pixel format 1");
        Assert(0);
    }
}

internal inline v2
Win32GetMouseP(){
    POINT MouseP;
    GetCursorPos(&MouseP);
    Assert(ScreenToClient(MainWindow, &MouseP));
    v2 Result = V2((f32)MouseP.x, (f32)(OSInput.WindowSize.Height-MouseP.y));
    return(Result);
}

internal BOOL WINAPI
Win32DefaultHandlerRoutine(DWORD ControlSignal){
    switch(ControlSignal)
    {
        case CTRL_CLOSE_EVENT:
        case CTRL_LOGOFF_EVENT:
        case CTRL_SHUTDOWN_EVENT: {
            // TODO(Tyler): I don't know if this is correct, but this is the only way I can
            // get it to close without crashing.
            
#if defined(SNAIL_JUMPY_DO_AUTO_SAVE_ON_EXIT)
            WorldManager.WriteWorldsToFiles();
#endif
            
            ExitProcess(0);
        }break;
        default: {
            return(false); // We didn't handle the control signal
        }break;
    }
}

//~ Audio
DWORD WINAPI
Win32AudioThreadProc(void *Parameter){
    memory_arena Arena;
    {
        umw Size = Kilobytes(512);
        void *Memory = AllocateVirtualMemory(Size);
        Assert(Memory);
        InitializeArena(&Arena, Memory, Size);
    }
    
    HRESULT Error;
    
    HDC DeviceContext = (HDC)Parameter;
    
    UINT DesiredSchedulerMS = 1;
    b8 SleepIsGranular = (timeBeginPeriod(DesiredSchedulerMS) == TIMERR_NOERROR);
    
    u32 AudioSampleCount;
    if(FAILED(Error = AudioClient->GetBufferSize(&AudioSampleCount))) Assert(0);
    
    
    int MonitorRefreshHz = 60;
    int RefreshRate = GetDeviceCaps(DeviceContext, VREFRESH);
    if(RefreshRate > 1)
    {
        MonitorRefreshHz = RefreshRate;
    }
    
    //f32 TargetSecondsPerFrame = 1.0f / MonitorRefreshHz;
    f32 TargetSecondsPerFrame = 10.0f / 1000.0f;
    
    LARGE_INTEGER LastTime = Win32GetWallClock();
    while(Running){
        
        u32 PaddingSamplesCount;
        if(FAILED(Error = AudioClient->GetCurrentPadding(&PaddingSamplesCount))) Assert(0);
        if(PaddingSamplesCount != 0) continue;
        
        //~ Audio
        OSSoundBuffer.SamplesPerFrame = (u32) ((f32)OSSoundBuffer.SampleRate * TargetSecondsPerFrame);
        u32 LatencySampleCount = 2*OSSoundBuffer.SamplesPerFrame;
        u32 SamplesToWrite = LatencySampleCount;
        
        u32 ActualSamplesToWrite = OSSoundBuffer.SamplesPerFrame;
        
        OSSoundBuffer.SamplesToWrite = SamplesToWrite;
        AudioMixer.OutputSamples(&Arena, &OSSoundBuffer);
        
        u8 *BufferData;
        if(SUCCEEDED(Error = AudioRenderClient->GetBuffer(ActualSamplesToWrite, &BufferData))){
            s16 *DestSample = (s16 *)BufferData;
            s16 *InputSample = OSSoundBuffer.Samples;
            for(u32 I=0; I < ActualSamplesToWrite; I++){
                *DestSample++ = *InputSample++;
                *DestSample++ = *InputSample++;
            }
            
            AudioRenderClient->ReleaseBuffer(ActualSamplesToWrite, 0);
        }
        
        f32 SecondsElapsed = Win32SecondsElapsed(LastTime, Win32GetWallClock());
#if 1
        if(SecondsElapsed < TargetSecondsPerFrame)
        {
            if(SleepIsGranular){
                DWORD SleepMS = (DWORD)Maximum((1000.0f*(TargetSecondsPerFrame-SecondsElapsed)-1.1), 0);
                if(SleepMS > 0){
                    Sleep(SleepMS);
                }
            }
            
            SecondsElapsed = Win32SecondsElapsed(LastTime, Win32GetWallClock());
            //Assert(SecondsElapsed < TargetSecondsPerFrame);
            
            while(true)
            {
                if(FAILED(Error = AudioClient->GetCurrentPadding(&PaddingSamplesCount))) Assert(0);
                if(PaddingSamplesCount == 0) break;
                _mm_pause();
            }
            OSInput.dTime = TargetSecondsPerFrame;
        }
        else
        {
            LogMessage("Missed FPS");
            OSInput.dTime = SecondsElapsed;
            if(OSInput.dTime > (MAXIMUM_SECONDS_PER_FRAME)){
                OSInput.dTime = MAXIMUM_SECONDS_PER_FRAME;
            }
        }
#endif
        
        //LogMessage("Audio loop 2: Seconds elapsed: %f, Target: %f", SecondsElapsed, TargetSecondsPerFrame);
        LastTime = Win32GetWallClock();
    }
    
    return 0;
}

internal void
Win32InitAudio(s32 SamplesPerSecond, s32 BufferSizeInSamples){
    HRESULT Error;
    if(FAILED(Error = CoInitializeEx(0, COINIT_SPEED_OVER_MEMORY))) Assert(0);
    
    IMMDeviceEnumerator *Enumerator;
    if(FAILED(Error = CoCreateInstance(__uuidof(MMDeviceEnumerator), 0, CLSCTX_ALL, 
                                       __uuidof(IMMDeviceEnumerator), (void **)&Enumerator))) Assert(0);
    
    IMMDevice *Device;
    if(FAILED(Error = Enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &Device))) Assert(0);
    
    if(FAILED(Error = Device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, 0, (void **)&AudioClient))) Assert(0);
    
    WAVEFORMATEXTENSIBLE WaveFormat;
    
    WaveFormat.Format.cbSize = sizeof(WaveFormat);
    WaveFormat.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
    WaveFormat.Format.wBitsPerSample = 16;
    WaveFormat.Format.nChannels = 2;
    WaveFormat.Format.nSamplesPerSec = (DWORD)SamplesPerSecond;
    WaveFormat.Format.nBlockAlign = (WORD)(WaveFormat.Format.nChannels * WaveFormat.Format.wBitsPerSample / 8);
    WaveFormat.Format.nAvgBytesPerSec = WaveFormat.Format.nSamplesPerSec * WaveFormat.Format.nBlockAlign;
    WaveFormat.Samples.wValidBitsPerSample = 16;
    WaveFormat.dwChannelMask = KSAUDIO_SPEAKER_STEREO;
    WaveFormat.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
    
    REFERENCE_TIME BufferDuration = 10000000ULL * BufferSizeInSamples / SamplesPerSecond;
    if(FAILED(Error = AudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_NOPERSIST, BufferDuration, 0, &WaveFormat.Format, 0))) Assert(0);
    
    if(FAILED(Error = AudioClient->GetService(__uuidof(IAudioRenderClient), (void **)&AudioRenderClient))) Assert(0);
    
    u32 AudioFrameCount;
    if(FAILED(Error = AudioClient->GetBufferSize(&AudioFrameCount))) Assert(0);
    
    Assert(BufferSizeInSamples <= (s32)AudioFrameCount);
}

//~

int CALLBACK
WinMain(HINSTANCE Instance,
        HINSTANCE PrevInstance,
        LPSTR CommandLine,
        int ShowCode){
    //~ Setup console
    Assert(AllocConsole());
    SetConsoleCtrlHandler(Win32DefaultHandlerRoutine, true);
    OSInput.ConsoleOutFile = (os_file *)GetStdHandle(STD_OUTPUT_HANDLE);
    OSInput.ConsoleErrorFile = (os_file *)GetStdHandle(STD_ERROR_HANDLE);
    
    //~ 
    WNDCLASS WindowClass = {};
    
    WindowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
    WindowClass.lpfnWndProc = Win32MainWindowProc;
    WindowClass.hInstance = Instance;
    //WindowClass.hIcon = ...;
    WindowClass.lpszClassName = "WindowClass";
    
    if (RegisterClass(&WindowClass))
    {
        MainWindow = CreateWindowExA(0,
                                     WindowClass.lpszClassName,
                                     "FAKE WINDOW",
                                     WS_OVERLAPPEDWINDOW,
                                     CW_USEDEFAULT, CW_USEDEFAULT,
                                     CW_USEDEFAULT, CW_USEDEFAULT,
                                     0,
                                     0,
                                     Instance,
                                     0);
        if(MainWindow){
            Win32InitOpenGl(Instance, &MainWindow);
            ToggleFullscreen(MainWindow);
            wglSwapIntervalEXT(1);
            
            HDC DeviceContext = GetDC(MainWindow);
            Running = true;
            
            //~ Timing setup
            UINT DesiredSchedulerMS = 1;
            b8 SleepIsGranular = (timeBeginPeriod(DesiredSchedulerMS) == TIMERR_NOERROR);
            
            LARGE_INTEGER PerformanceCounterFrequencyResult;
            QueryPerformanceFrequency(&PerformanceCounterFrequencyResult);
            GlobalPerfCounterFrequency = (f32)PerformanceCounterFrequencyResult.QuadPart;
            
            s32 MonitorRefreshHz = 60;
            s32 RefreshRate = GetDeviceCaps(DeviceContext, VREFRESH);
            if(RefreshRate > 1)
            {
                MonitorRefreshHz = RefreshRate;
            }
            f32 GameUpdateHz = (f32)(MonitorRefreshHz);
            
            f32 TargetSecondsPerFrame = 1.0f / GameUpdateHz;
            if(TargetSecondsPerFrame < MINIMUM_SECONDS_PER_FRAME){
                TargetSecondsPerFrame = MINIMUM_SECONDS_PER_FRAME;
            }else if(TargetSecondsPerFrame > MAXIMUM_SECONDS_PER_FRAME){
                TargetSecondsPerFrame = MAXIMUM_SECONDS_PER_FRAME;
            }
            LARGE_INTEGER LastTime = Win32GetWallClock();
            
            //~ Audio
            s32 SamplesPerSecond = 48000;
            u32 SamplesPerAudioFrame = (u32)((f32)SamplesPerSecond / (f32)MonitorRefreshHz);
            Win32InitAudio(SamplesPerSecond, SamplesPerSecond);
            AudioClient->Start();
            
            u32 AudioSampleCount;
            HRESULT Error;
            if(FAILED(Error = AudioClient->GetBufferSize(&AudioSampleCount))) Assert(0);
            
            u32 BufferSize = AudioSampleCount*2*sizeof(s16);
            OSSoundBuffer.SampleRate = SamplesPerSecond;
            OSSoundBuffer.Samples = (s16 *)AllocateVirtualMemory(BufferSize);
            
            CreateThread(0, 0, Win32AudioThreadProc, DeviceContext, 0, 0);
            
            //~ 
            InitializeGame();
            
            //~ Main loop
            while(Running){
                RECT ClientRect;
                GetClientRect(MainWindow, &ClientRect);
                OSInput.WindowSize = {
                    (f32)(ClientRect.right - ClientRect.left),
                    (f32)(ClientRect.bottom - ClientRect.top),
                };
                OSInput.LastMouseP = OSInput.MouseP;
                OSInput.MouseP = Win32GetMouseP();
                OSInput.dTime = TargetSecondsPerFrame;
                
                GameUpdateAndRender();
                
                //~ Timing
#if 0
                f32 SecondsElapsed = Win32SecondsElapsed(LastTime, Win32GetWallClock());
                if(SecondsElapsed < TargetSecondsPerFrame)
                {
                    if(SleepIsGranular){
                        DWORD SleepMS = (DWORD)(1000.0f * (TargetSecondsPerFrame-SecondsElapsed));
                        if(SleepMS > 0){
                            Sleep(SleepMS);
                        }
                    }
                    
                    SecondsElapsed = Win32SecondsElapsed(LastTime, Win32GetWallClock());
                    
                    while(SecondsElapsed < TargetSecondsPerFrame)
                    {
                        SecondsElapsed = Win32SecondsElapsed(LastTime, Win32GetWallClock());
                    }
                    OSInput.dTime = TargetSecondsPerFrame;
                }
                else
                {
                    LogMessage("Missed FPS");
                    OSInput.dTime = SecondsElapsed;
                    if(OSInput.dTime > (MAXIMUM_SECONDS_PER_FRAME)){
                        OSInput.dTime = MAXIMUM_SECONDS_PER_FRAME;
                    }
                }
#endif
                SwapBuffers(DeviceContext);
                
                LARGE_INTEGER EndTime = Win32GetWallClock();
                f32 TimeElapsedForFrame = Win32SecondsElapsed(LastTime, EndTime);
                
                TargetSecondsPerFrame = TimeElapsedForFrame;
                
                LastTime = EndTime;
            }
        }
        else
        {
            // TODO(Tyler): Error logging!
            OutputDebugString("Failed to create window!");
            LogMessage("Win32: Failed to create window!");
        }
        
    }
    else
    {
        // TODO(Tyler): Error logging!
        OutputDebugString("Failed to register window class!");
        LogMessage("Win32: Failed to register window class!!");
    }
    
#if defined(SNAIL_JUMPY_DO_AUTO_SAVE_ON_EXIT)
    WorldManager.WriteWorldsToFiles();
#endif
    FreeConsole();
    
    return(0);
}

//~ OS API
internal os_file *
OpenFile(const char *Path, open_file_flags Flags){
    DWORD Access = 0;
    DWORD Creation = OPEN_ALWAYS;
    if(Flags & OpenFile_Read){
        Access |= GENERIC_READ;
        Creation = OPEN_EXISTING;
    }
    if(Flags & OpenFile_Write){
        Access |= GENERIC_WRITE;
    }
    if(Flags & OpenFile_Clear){
        DeleteFileA(Path);
    }
    
    os_file *Result;
    HANDLE File = CreateFileA(Path, Access, FILE_SHARE_READ, 0, Creation, 0, 0);
    if(File != INVALID_HANDLE_VALUE){
        Result = (os_file *)File;
    }else{
        Result = 0;
    }
    
    return(Result);
}

internal u64 
GetFileSize(os_file *File){
    u64 Result = 0;
    LARGE_INTEGER FileSize = {};
    if(GetFileSizeEx(File, &FileSize)){
        Result = FileSize.QuadPart;
    }else{
        // TODO(Tyler): Logging
        Result = {};
    }
    return(Result);
}

internal b32 
ReadFile(os_file *File, u64 FileOffset, void *Buffer, umw BufferSize){
    b32 Result = false;
    LARGE_INTEGER DistanceToMove;
    DistanceToMove.QuadPart = FileOffset;
    SetFilePointerEx((HANDLE)File, DistanceToMove, 0, FILE_BEGIN);
    DWORD BytesRead;
    //Assert(BufferSize <= File->Size);
    if(ReadFile((HANDLE)File,
                Buffer,
                (DWORD)(BufferSize),
                &BytesRead, 0)){
        // NOTE(Tyler): Success!!!
        Result = true;
    }else{
        DWORD Error = GetLastError();
        Assert(0);
    }
    
    return(Result);
}

internal void 
CloseFile(os_file *File){
    CloseHandle((HANDLE)File);
}

// TODO(Tyler): Proper WriteFile for 64-bits
internal u64 
WriteToFile(os_file *File, u64 FileOffset, const void *Buffer, umw BufferSize){
    DWORD BytesWritten;
    LARGE_INTEGER DistanceToMove;
    DistanceToMove.QuadPart = FileOffset;
    SetFilePointerEx((HANDLE)File, DistanceToMove, 0, FILE_BEGIN);
    WriteFile((HANDLE)File, Buffer, (DWORD)BufferSize, &BytesWritten, 0);
    return(BytesWritten);
}

internal u64
GetLastFileWriteTime(os_file *File){
    FILETIME LastWriteTime = {};
    GetFileTime(File, 0, 0, &LastWriteTime);
    ULARGE_INTEGER Result;
    Result.HighPart = LastWriteTime.dwHighDateTime;
    Result.LowPart  = LastWriteTime.dwLowDateTime;
    return(Result.QuadPart);
}

internal b8
DeleteFileAtPath(const char *Path){
    b8 Result = (b8)DeleteFileA(Path);
    return(Result);
}

internal void *
AllocateVirtualMemory(umw Size){
    void *Memory = VirtualAlloc(0, Size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    DWORD Error = GetLastError();
    return(Memory);
}

internal void 
FreeVirtualMemory(void *Pointer){
    VirtualFree(Pointer, 0, MEM_RELEASE);
}

internal void *
DefaultAlloc(umw Size){
    void *Result = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, Size);
    return(Result);
}

internal void *
DefaultRealloc(void *Memory, umw Size){
    void *Result = HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, Memory, Size);
    return(Result);
}

internal void
DefaultFree(void *Pointer){
    Assert(HeapFree(GetProcessHeap(), 0, Pointer));
}

internal void
VWriteToDebugConsole(os_file *Output, const char *Format, va_list VarArgs){
    char Buffer[DEFAULT_BUFFER_SIZE];
    stbsp_vsnprintf(Buffer, sizeof(Buffer), Format, VarArgs);
    WriteConsole(Output, Buffer, (DWORD)CStringLength(Buffer), 0, 0);
}

internal void
WriteToDebugConsole(os_file *Output, const char *Format, ...){
    va_list VarArgs;
    va_start(VarArgs, Format);
    VWriteToDebugConsole(Output, Format, VarArgs);
    va_end(VarArgs);
}

internal b8
PollEvents(os_event *Event){
    *Event = {};
    
    MSG Message;
    b8 Result = false;
    while(true){
        Result = (b8)PeekMessage(&Message, 0, 0, 0, PM_REMOVE);
        if(!Result) break;
        
        // TODO(Tyler): This may not actually be needed here
        if(Message.message == WM_QUIT){
            Running = false;
        }
        TranslateMessage(&Message);
        
        switch(Message.message){
            case WM_SETCURSOR: {
                HCURSOR Cursor = LoadCursorA(0, IDC_ARROW);
                SetCursor(Cursor);
            }break;
            case WM_SIZE: {
                RECT ClientRect;
                GetClientRect(MainWindow, &ClientRect);
                int Width = ClientRect.right - ClientRect.left;
                int Height = ClientRect.bottom - ClientRect.top;
                OSInput.WindowSize = {(f32)Width, (f32)Height};
            }break;
            case WM_CLOSE: {
                Running = false;
            }break;
            case WM_DESTROY: {
                Running = false;
            }break;
            case WM_SYSKEYDOWN: case WM_SYSKEYUP: 
            case WM_KEYDOWN: case WM_KEYUP: {
                u32 VKCode = (u32)Message.wParam;
                
                b8 WasDown = ((Message.lParam & (1 << 30)) != 0);
                b8 IsDown = ((Message.lParam & (1UL << 31)) == 0);
                if(WasDown != IsDown){
                    Event->JustDown = true;
                    if(IsDown){
                        if(VKCode == VK_F11){
                            ToggleFullscreen(MainWindow);
                        }else if((VKCode == VK_F4) && (Message.lParam & (1<<29))){
                            Running = false;
                        }
                    }
                }
                
                if(('0' <= VKCode) && (VKCode <= 'Z')) {
                    Event->Key = (os_key_code)VKCode; 
                }else{
                    switch(VKCode){
                        //~ Special keys
                        case VK_UP: Event->Key = KeyCode_Up;               break;
                        case VK_DOWN:      Event->Key = KeyCode_Down;      break;
                        case VK_LEFT:      Event->Key = KeyCode_Left;      break;
                        case VK_RIGHT:     Event->Key = KeyCode_Right;     break;
                        case VK_SPACE:     Event->Key = KeyCode_Space;     break;
                        case VK_TAB:       Event->Key = KeyCode_Tab;       break;
                        case VK_CONTROL:   Event->Key = KeyCode_Control;   break;
                        case VK_SHIFT:     Event->Key = KeyCode_Shift;     break;
                        case VK_ESCAPE:    Event->Key = KeyCode_Escape;    break;
                        case VK_BACK:      Event->Key = KeyCode_BackSpace; break;
                        case VK_DELETE:    Event->Key = KeyCode_Delete;    break;
                        case VK_RETURN:    Event->Key = KeyCode_Return;    break;
                        case VK_MENU:      Event->Key = KeyCode_Alt;       break;
                        case VK_F1:        Event->Key = KeyCode_F1;        break;
                        case VK_F2:        Event->Key = KeyCode_F2;        break;
                        case VK_F3:        Event->Key = KeyCode_F3;        break;
                        case VK_F4:        Event->Key = KeyCode_F4;        break;
                        case VK_F5:        Event->Key = KeyCode_F5;        break;
                        case VK_F6:        Event->Key = KeyCode_F6;        break;
                        case VK_F7:        Event->Key = KeyCode_F7;        break;
                        case VK_F8:        Event->Key = KeyCode_F8;        break;
                        case VK_F9:        Event->Key = KeyCode_F9;        break;
                        case VK_F10:       Event->Key = KeyCode_F10;       break;
                        case VK_F11:       Event->Key = KeyCode_F11;       break;
                        case VK_F12:       Event->Key = KeyCode_F12;       break;
                        
                        //~ Normal ascii
                        
                        case VK_OEM_1:      Event->Key = (os_key_code)';'; break;
                        case VK_OEM_PLUS:   Event->Key = (os_key_code)'='; break; 
                        case VK_OEM_COMMA:  Event->Key = (os_key_code)','; break;
                        case VK_OEM_MINUS:  Event->Key = (os_key_code)'-'; break;
                        case VK_OEM_PERIOD: Event->Key = (os_key_code)'.'; break;
                        case VK_OEM_2:      Event->Key = (os_key_code)'/'; break;
                        case VK_OEM_3:      Event->Key = (os_key_code)'`'; break;
                        case VK_OEM_4:      Event->Key = (os_key_code)'['; break;
                        case VK_OEM_5:      Event->Key = (os_key_code)'\\'; break;
                        case VK_OEM_6:      Event->Key = (os_key_code)']'; break;
                        case VK_OEM_7:      Event->Key = (os_key_code)'\''; break;
                    }
                }
                
                
                if(IsDown){
                    Event->Kind = OSEventKind_KeyDown;
                }else{
                    Event->Kind = OSEventKind_KeyUp;
                }
            }break;
            {
                case WM_LBUTTONDOWN: {
                    Event->Button = MouseButton_Left;
                }goto process_mouse_down;
                case WM_MBUTTONDOWN: {
                    Event->Button = MouseButton_Middle;
                }goto process_mouse_down;
                case WM_RBUTTONDOWN: {
                    Event->Button = MouseButton_Right;
                }goto process_mouse_down;
                
                process_mouse_down:;
                Event->Kind = OSEventKind_MouseDown;
                
                Event->MouseP = Win32GetMouseP();
            }break;
            {
                case WM_LBUTTONUP: {
                    Event->Button = MouseButton_Left;
                }goto process_mouse_up;
                case WM_MBUTTONUP: {
                    Event->Button = MouseButton_Middle;
                }goto process_mouse_up;
                case WM_RBUTTONUP: {
                    Event->Button = MouseButton_Right;
                }goto process_mouse_up;
                
                process_mouse_up:;
                Event->Kind = OSEventKind_MouseUp;
                
                Event->MouseP = Win32GetMouseP();
            }break;
            case WM_MOUSEMOVE: {
                Event->Kind = OSEventKind_MouseMove;
                Event->MouseP = Win32GetMouseP();
            }break;
            case WM_MOUSEWHEEL: {
                Event->Kind = OSEventKind_MouseWheelMove;
                Event->WheelMovement = GET_WHEEL_DELTA_WPARAM(Message.wParam);
            }break;
            default: {
                DefWindowProcA(Message.hwnd, Message.message, 
                               Message.wParam, Message.lParam);
            }break;
        }
        
        break;
    }
    return(Result);
}

//~ Miscellaneous
internal void
OSSleep(u32 Milliseconds){
    Sleep(Milliseconds);
}

internal void
OSEndGame(){
    Running = false;
}