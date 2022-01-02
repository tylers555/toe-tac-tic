#ifndef SNAIL_JUMPY_WORLD_H
#define SNAIL_JUMPY_WORLD_H

//~ World entities

typedef u32 world_entity_flags;
enum world_entity_flags_ {
 WorldEntityFlag_None = (0 << 0),
 WorldEntityEditFlag_Hide = (1 << 0),
 
 WorldEntityEditFlags_All = (WorldEntityEditFlag_Hide),
 
 WorldEntityTilemapFlag_TreatEdgesAsTiles = (1 << 15),
};

struct world_entity_group {
 char *Name;
 f32 Z;
 u32 Layer;
 world_entity_flags Flags;
 b8 FlaggedForDeletion;
};

//~ World data
typedef u32 world_flags;
enum world_flags_ {
 WorldFlag_None,
 WorldFlag_IsCompleted = (1 << 0),
 WorldFlag_IsTopDown = (1 << 1)
};

global_constant u32 MAX_WORLD_ENTITIES = 256;
global_constant u32 MAX_WORLD_ENTITY_GROUPS = 16;
struct world_data {
 string Name;
 
 u32 Width;
 u32 Height;
 
 entity_manager Manager;
 
 collision_boundary *TeleporterBoundary;
 
 array<world_entity_group> EntityGroups;
 
 world_flags Flags;
 
 hsb_color AmbientColor;
 f32       Exposure;
};

//~ World manager

global_constant u32 CURRENT_WORLD_FILE_VERSION = 1;

struct world_manager {
 memory_arena Memory;
 memory_arena TransientMemory;
 hash_table<string, world_data> WorldTable;
 
 void        Initialize(memory_arena *Arena);
 world_data *GetOrCreateWorld(string Name);
 world_data *GetWorld(string Name);
 world_data *CreateNewWorld(string Name);
 
 entity     *LoadBasicEntity(world_data *World, entity_manager *Manager, stream *Stream);
 world_data *LoadWorldFromFile(const char *Name);
 
 void WriteBasicEntity();
 void WriteWorldsToFiles();
 
 void LoadWorld(const char *LevelName);
 b8 IsLevelCompleted(string LevelName);
 void RemoveWorld(string Name);
};

//~ File loading
#pragma pack(push, 1)
struct world_file_header {
 char Header[3];
 u32 Version;
 u32 WidthInTiles;
 u32 HeightInTiles;
 
 u32 EntityGroupCount;
 u32 EntityCount;
 
 hsb_color AmbientColor;
 f32       Exposure;
};
#pragma pack(pop)

#endif //SNAIL_JUMPY_WORLD_H
