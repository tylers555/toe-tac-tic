//TODO(Tyler): Implement an allocator for the stb libraries
#define STB_NO_STDIO
#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb_image.h"
#define STB_RECT_PACK_IMPLEMENTATION
#include "third_party/stb_rect_pack.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "third_party/stb_truetype.h"
#define STB_SPRINTF_IMPLEMENTATION
#include "third_party/stb_sprintf.h"

#include "main.h"

//~ Engine variables
global debug_config DebugConfig;

global game_renderer GameRenderer;

global entity_manager EntityManager;
global ui_manager UIManager;

global state_change_data StateChangeData;

global world_editor WorldEditor;
global tic_tac_toe_state TicTacToeState;

global string_manager Strings;

global world_manager WorldManager;

global asset_system AssetSystem;

global font_system FontSystem;

global audio_mixer AudioMixer;

//~ TODO(Tyler): Refactor these!
global font MainFont; // TODO(Tyler): Remove this one
global font TitleFont;
global font DebugFont;

//~ Gameplay variables
global s32 Score;
global f32 CompletionCooldown;
global world_data *CurrentWorld;

//~ Hotloaded variables file!
// TODO(Tyler): Load this from a variables file at startup
global game_mode GameMode = GameMode_TicTacToe;

//~ Helpers
internal inline string
String(const char *S){
    return Strings.GetString(S);
}

//~ Includes
#include "logging.cpp"
#include "render.cpp"
#include "stream.cpp"
#include "file_processing.cpp"
#include "wav.cpp"
#include "ui.cpp"
#include "ui_window.cpp"
#include "physics.cpp"
#include "asset.cpp"
#include "asset_loading.cpp"
#include "entity.cpp"
#include "debug_ui.cpp"
#include "world.cpp"
#include "audio_mixer.cpp"

#include "debug_game_mode.cpp"
#include "world_editor.cpp"
#include "game.cpp"
#include "tic_tac_toe.cpp"

//~ 

internal void
InitializeGame(){
    stbi_set_flip_vertically_on_load(true);
    
    {
        umw Size = Megabytes(500);
        void *Memory = AllocateVirtualMemory(Size);
        Assert(Memory);
        InitializeArena(&PermanentStorageArena, Memory, Size);
    }{
        umw Size = Gigabytes(1);
        void *Memory = AllocateVirtualMemory(Size);
        Assert(Memory);
        InitializeArena(&TransientStorageArena, Memory, Size);
    }
    
    LogFile = OpenFile("log.txt", OpenFile_Write | OpenFile_Clear);
    
    InitializeRendererBackend();
    GameRenderer.Initialize(&PermanentStorageArena, OSInput.WindowSize);
    
    Strings.Initialize(&PermanentStorageArena);
    FontSystem.Initialize(&PermanentStorageArena);
    
    FontSystem.LoadFont(String("debug_font"), "asset_fonts/Roboto-Regular.ttf", 22);
    FontSystem.LoadFont(String("title_font"), "asset_fonts/Roboto-Regular.ttf", 35);
    FontSystem.LoadFont(String("main_font"),  "asset_fonts/Press-Start-2P.ttf", 26);
    MainFont  = *FontSystem.FindFont(String("main_font"));
    TitleFont = *FontSystem.FindFont(String("title_font"));
    DebugFont = *FontSystem.FindFont(String("debug_font"));
    
    EntityManager.Initialize(&PermanentStorageArena);
    WorldManager.Initialize(&PermanentStorageArena);
    UIManager.Initialize(&PermanentStorageArena);
    
    //~ Load things
    LoadedImageTable = PushHashTable<const char *, image>(&PermanentStorageArena, 256);
    AssetSystem.Initialize(&PermanentStorageArena);
    WorldManager.LoadWorld(STARTUP_LEVEL);
    
    WorldEditor.Initialize();
    
    AudioMixer.Initialize(&PermanentStorageArena);
    
    AssetSystem.LoadAssetFile(ASSET_FILE_PATH);
    AudioMixer.PlaySound(AssetSystem.GetSoundEffect(String("test_music")));
}

internal void
GameUpdateAndRender(){
    //~ Prepare for next frame
    ProfileData.CurrentBlockIndex = 0;
    ArenaClear(&TransientStorageArena);
    UIManager.BeginFrame();
    
    //~ Do next frame
    TIMED_FUNCTION();
    AssetSystem.LoadAssetFile(ASSET_FILE_PATH);
    
    switch(GameMode){
        case GameMode_Debug: {
            UpdateAndRenderDebug();
        }break;
        case GameMode_WorldEditor: {
            WorldEditor.UpdateAndRender();
        }break;
        case GameMode_MainGame: {
            UpdateAndRenderMainGame();
        }break;
        case GameMode_TicTacToe: {
            UpdateAndRenderTicTacToe();
        }break;
    }
    
    UIManager.EndFrame();
    DEBUGRenderOverlay();
    RendererRenderAll(&GameRenderer);
    
    Counter += OSInput.dTime;
    
    //~ Reset OS input
    {
        for(u32 I=0; I<KeyCode_TOTAL; I++){
            key_state State = OSInput.KeyboardState[I];
            OSInput.KeyboardState[I] &= ~KeyState_JustDown;
            OSInput.KeyboardState[I] &= ~KeyState_RepeatDown;
            OSInput.KeyboardState[I] &= ~KeyState_JustUp;
        }
        
        for(u32 I=0; I<MouseButton_TOTAL; I++){
            key_state State = OSInput.MouseState[I];
            OSInput.MouseState[I] &= ~KeyState_JustDown;
            OSInput.MouseState[I] &= ~KeyState_JustUp;
        }
        
        OSInput.ScrollMovement = 0;
    }
    
    //~ Other
    if(StateChangeData.DidChange){
        switch(StateChangeData.NewMode){
            case GameMode_None: WorldManager.LoadWorld(StateChangeData.NewLevel); break;
            case GameMode_WorldEditor:{
                GameMode = GameMode_WorldEditor;
            }break;
            case GameMode_MainGame: {
                GameMode = GameMode_MainGame; 
                WorldManager.LoadWorld(StateChangeData.NewLevel); 
            }break;
            case GameMode_TicTacToe: {
                GameMode = GameMode_TicTacToe;
            }break;
        }
        
        StateChangeData = {};
    }
}

internal inline void
ChangeState(game_mode NewMode, string NewLevel){
    StateChangeData.DidChange = true;
    StateChangeData.NewMode = NewMode;
    StateChangeData.NewLevel = Strings.GetString(NewLevel);
}

internal inline void
ToggleOverlay(_debug_overlay_flags Overlay){
    if(!(DebugConfig.Overlay & Overlay)) DebugConfig.Overlay |= Overlay;
    else DebugConfig.Overlay &= ~Overlay;
}

internal void
ProcessDefaultEvent(os_event *Event){
    switch(Event->Kind){
        case OSEventKind_KeyDown: {
            switch((u32)Event->Key){
                case KeyCode_Shift:   OSInput.KeyFlags |= KeyFlag_Shift;   break;
                case KeyCode_Control: OSInput.KeyFlags |= KeyFlag_Control; break;
                case KeyCode_Alt:     OSInput.KeyFlags |= KeyFlag_Alt;     break;
                
#if defined(SNAIL_JUMPY_DEBUG_BUILD)
                case KeyCode_F1: ToggleOverlay(DebugOverlay_Miscellaneous); break;
                case KeyCode_F2: ToggleOverlay(DebugOverlay_Profiler); break;
                case KeyCode_F3: ToggleOverlay(DebugOverlay_Boundaries); break;
                case KeyCode_F8: {
                    if(PhysicsDebugger.Flags & PhysicsDebuggerFlags_StepPhysics){
                        PhysicsDebugger.Flags &= ~PhysicsDebuggerFlags_StepPhysics;
                    }else{
                        PhysicsDebugger.Flags |= PhysicsDebuggerFlags_StepPhysics;
                    } 
                }break;
                case KeyCode_F9: if(PhysicsDebugger.Paused > 0) {
                    PhysicsDebugger.Paused--; 
                } break;
                case KeyCode_F10: PhysicsDebugger.Paused++; break;
                
                case 'J': if(PhysicsDebugger.Flags & PhysicsDebuggerFlags_StepPhysics){
                    if(PhysicsDebugger.Scale > 0.1f){
                        PhysicsDebugger.Scale /= 1.01f;
                    }
                } break;
                case 'K': if(PhysicsDebugger.Flags & PhysicsDebuggerFlags_StepPhysics){
                    PhysicsDebugger.Scale *= 1.01f;
                } break;
#endif
                
            }
            
            OSInput.KeyboardState[Event->Key] |= KeyState_RepeatDown;
            OSInput.KeyboardState[Event->Key] |= KeyState_IsDown;
            if(Event->JustDown){
                OSInput.KeyboardState[Event->Key] |= KeyState_JustDown;
            }
        }break;
        case OSEventKind_KeyUp: {
            switch((u32)Event->Key){
                case KeyCode_Shift:   OSInput.KeyFlags &= ~KeyFlag_Shift;   break;
                case KeyCode_Control: OSInput.KeyFlags &= ~KeyFlag_Control; break;
                case KeyCode_Alt:     OSInput.KeyFlags &= ~KeyFlag_Alt;     break;
            }
            
            OSInput.KeyboardState[Event->Key] = KeyState_JustUp;
        }break;
        case OSEventKind_MouseDown: {
            OSInput.MouseState[Event->Button] |= KeyState_IsDown;
            OSInput.MouseState[Event->Button] |= KeyState_JustDown;
        }break;
        case OSEventKind_MouseUp: {
            OSInput.MouseState[Event->Button] = KeyState_JustUp;
        }break;
        case OSEventKind_MouseWheelMove: {
            OSInput.ScrollMovement = Event->WheelMovement;
        }break;
    }
}