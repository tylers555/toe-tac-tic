#ifndef SNAIL_JUMPY_ENTITY_H
#define SNAIL_JUMPY_ENTITY_H

//~ Other

struct player_input {
 b8 Up;
 b8 Down;
 b8 Left;
 b8 Right;
 b8 Jump;
 
 b8 Select;
};

//~ Entities

struct entity {
 entity_array_type Type;
 entity_type_flags TypeFlags;
 entity_flags Flags;
 
 animation_state Animation;
 
 f32 Z;
 u32 Layer;
 
 entity *Parent;
 physics_update *Update;
 
 physics_state_flags PhysicsFlags;
 physics_layer_flags PhysicsLayer;
 
 v2 P, dP, TargetdP;
 v2 FloorNormal;
 
 rect Bounds;
 collision_boundary *Boundaries;
 u8 BoundaryCount;
 u8 BoundarySet;
 
 union{
  collision_response_function *Response;
  trigger_response_function   *TriggerResponse;
 };
};

struct entity_tilemap : public entity {
 string Asset;
 tilemap_data TilemapData;
 
 // TODO(Tyler): TEMPORARY
 tilemap_tile *Tiles;
 u32 Width;
 u32 Height;
 
 u8 *PhysicsMap;
 v2 TileSize;
};

struct entity_teleporter : public entity {
 char *Level;
 char *RequiredLevel;
 b8 IsLocked;
 b8 IsSelected;
};

struct entity_door : public entity {
 b8 IsOpen;
 f32 Cooldown;
 char *RequiredLevel;
};

// TODO(Tyler): This has a lot of unnecessary stuff in it
struct entity_art : public entity {
 string Asset;
};

struct entity_match : public entity {
 b8 IsSelected;
};

struct entity_player : public entity {
 asset_entity   *EntityInfo;
 
 s32 Health;
 f32 JumpTime;
 v2 StartP;
};


//~
struct entity_manager {
 memory_arena Memory;
 
 player_input PlayerInput;
 u32 EntityCount;
 
#define ENTITY_TYPE_(TypeName, ...) bucket_array<TypeName, 32> EntityArray_##TypeName;
 entity_player *Player;
 ENTITY_TYPES;
 SPECIAL_ENTITY_TYPES;
#undef ENTITY_TYPE_
 
 void Initialize(memory_arena *Arena);
 void Reset();
 void ProcessEvent(os_event *Event);
 void UpdateEntities();
 void RenderEntities();
 inline void DamagePlayer(u32 Damage);
 inline void LoadTo(entity_manager *ToManager, memory_arena *Arena);
 
 entity *AllocBasicEntity(entity_array_type Type);
 void RemoveEntityByIndex(entity_array_type Type, bucket_index Index);
 
 //~ Physics
 bucket_array<physics_particle_system, 64> ParticleSystems;
 
 memory_arena ParticleMemory;
 memory_arena PermanentBoundaryMemory;
 memory_arena BoundaryMemory;
 
 void DoFloorRaycast(physics_update_context *Context, entity *Entity, physics_layer_flags Layer, f32 Depth);
 void DoPhysics(physics_update_context *Context);
 
 void DoStaticCollisions(physics_collision *OutCollision, collision_boundary *Boundary, v2 P, v2 Delta);
 void DoTriggerCollisions(physics_trigger_collision *OutTrigger, collision_boundary *Boundary, v2 P, v2 Delta);
 void DoCollisionsRelative(physics_update_context *Context, physics_collision *OutCollision, collision_boundary *Boundary, v2 P, v2 Delta, entity *EntityA, physics_layer_flags Layer, u32 StartIndex=0);
 void DoCollisionsNotRelative(physics_update_context *Context, physics_collision *OutCollision, collision_boundary *Boundary, v2 P, v2 Delta, entity *EntityA, physics_layer_flags Layer);
 
 physics_particle_system *AddParticleSystem(v2 P, collision_boundary *Boundary, u32 ParticleCount, f32 COR);
 
 collision_boundary *AllocPermanentBoundaries(u32 Count);
 collision_boundary *AllocBoundaries(u32 Count);
};

internal inline entity_iterator
EntityManagerBeginIteration(entity_manager *Manager);
internal inline b8
EntityManagerContinueIteration(entity_manager *Manager, entity_iterator *Iterator);
internal inline void
EntityManagerNextIteration(entity_manager *Manager, entity_iterator *Iterator);


#endif //SNAIL_JUMPY_ENTITY_H
#ifndef SNAIL_JUMPY_ENTITY_H
#define SNAIL_JUMPY_ENTITY_H

#endif //SNAIL_JUMPY_ENTITY_H
