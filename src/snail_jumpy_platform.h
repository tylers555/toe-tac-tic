
// NOTE(Tyler): This is the platform layer, not the platforms that the entities interact with

struct platform_button_state {
    u32 HalfTransitionCount;
    b32 EndedDown;
};

struct platform_user_input {
    f32 dTimeForFrame;
    
    platform_button_state UpButton;    // 'W'
        platform_button_state DownButton;  // 'S'
        platform_button_state LeftButton;  // 'A'
            platform_button_state RightButton; // 'D'
            platform_button_state JumpButton;  // Spacebar
};

#define BYTES_PER_PIXEL 4
struct platform_backbuffer {
    // Memory is laid out: AA RR GG BB
    void *Memory;
    s32 Width, Height;
    s32 Pitch;
};

// TODO(Tyler): Implement actual file I/O
struct DEBUG_read_file_result
{
     u64 Size;
    void *Data;
};

struct thread_context
{
    int PlaceHolder;
};

#define DEBUG_READ_FILE_SIG(name) DEBUG_read_file_result name(thread_context *Thread, const char *Path)
typedef DEBUG_READ_FILE_SIG(DEBUG_read_file_proc);

#define DEBUG_FREE_FILE_DATA_SIG(name) void name(thread_context *Thread, void *FileData)
typedef DEBUG_FREE_FILE_DATA_SIG(DEBUG_free_file_data_proc);

#define DEBUG_WRITE_DATA_TO_FILE_SIG(name) b32 name(thread_context *Thread, const char *Path, void *Data, u64 Size)
typedef DEBUG_WRITE_DATA_TO_FILE_SIG(DEBUG_write_data_to_file_proc);

struct platform_api {
    DEBUG_read_file_proc *ReadFile;
    DEBUG_free_file_data_proc *FreeFileData;
    DEBUG_write_data_to_file_proc *WriteDataToFile;
};

struct game_state;

struct game_memory {
    b32 IsInitialized;
    game_state *State;
    
    void *PermanentStorage; // NOTE(Tyler): Should be initialized to zero
    memory_index PermanentStorageSize;
    
    void *TransientStorage; // NOTE(Tyler): Should be initialized to zero
    memory_index TransientStorageSize;
};
    