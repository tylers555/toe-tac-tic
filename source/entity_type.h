#ifndef ENTITY_TYPE_H
#define ENTITY_TYPE_H

typedef u32 entity_type_flags;
enum entity_type_flags_ {
 EntityTypeFlag_None    = (0 << 0),
 EntityTypeFlag_Dynamic = (1 << 0),
 EntityTypeFlag_Static  = (1 << 1),
 EntityTypeFlag_Trigger = (1 << 2),
};

#define AllocEntity(Manager, TypeName) \
(TypeName *)(Manager)->AllocBasicEntity(ENTITY_TYPE(TypeName))

#define FOR_ENTITY_TYPE(Manager, TypeName) FOR_BUCKET_ARRAY(It, &(Manager)->EntityArray_##TypeName)
#define FOR_EACH_ENTITY(Manager) for(auto It = EntityManagerBeginIteration(Manager); \
EntityManagerContinueIteration(Manager, &It);   \
EntityManagerNextIteration(Manager, &It))

#define ENTITY_ARRAY(TypeName) EntityArray_##TypeName

#define ENTITY_TYPES \
ENTITY_TYPE_(entity_tilemap,    EntityTypeFlag_None,    PhysicsLayerFlag_Static,                            "tilemap") \
ENTITY_TYPE_(entity_teleporter, EntityTypeFlag_Trigger, PhysicsLayerFlag_PlayerTrigger,                     "teleporter") \
ENTITY_TYPE_(entity_door,       EntityTypeFlag_Static,  PhysicsLayerFlag_Static,                            "door") \
ENTITY_TYPE_(entity_art,        EntityTypeFlag_None,    PhysicsLayerFlag_None,                              "art")  \
ENTITY_TYPE_(entity_match,      EntityTypeFlag_Trigger, PhysicsLayerFlag_PlayerTrigger,                     "teleporter") \

#define SPECIAL_ENTITY_TYPES

#define PLAYER_ENTITY_TYPE \
ENTITY_TYPE_(entity_player, EntityTypeFlag_Dynamic, PhysicsLayerFlag_Basic|PhysicsLayerFlag_PlayerTrigger, "player")

#define ENTITY_TYPE_(TypeName, ...) EntityArrayType_##TypeName,
enum entity_array_type {
 EntityArrayType_None,
 PLAYER_ENTITY_TYPE
  
  ENTITY_TYPES
  EntityArrayType_TOTAL
};
#undef ENTITY_TYPE_

#define ENTITY_TYPE_(TypeName, TypeFlags, LayerFlags, ...) (TypeFlags),
entity_type_flags ENTITY_TYPE_TYPE_FLAGS[EntityArrayType_TOTAL]  = {
 EntityTypeFlag_None,
 PLAYER_ENTITY_TYPE
  
  ENTITY_TYPES
};
#undef ENTITY_TYPE_

#define ENTITY_TYPE_(TypeName, TypeFlags, LayerFlags, ...) (LayerFlags),
physics_layer_flags ENTITY_TYPE_LAYER_FLAGS[EntityArrayType_TOTAL]  = {
 PhysicsLayerFlag_None,
 PLAYER_ENTITY_TYPE
  
  ENTITY_TYPES
};
#undef ENTITY_TYPE_

#define ENTITY_TYPE_(TypeName, TypeFlags, LayerFlags, NameString, ...) NameString,
const char *ENTITY_TYPE_NAME_TABLE[EntityArrayType_TOTAL]  = {
 PLAYER_ENTITY_TYPE
  
  ENTITY_TYPES
};
#undef ENTITY_TYPE_

#define ENTITY_TYPE(TypeName) EntityArrayType_##TypeName

struct entity_iterator {
 entity_array_type CurrentArray;
 
 entity *Item;
 bucket_index Index;
 u32 I;
};

#endif //ENTITY_TYPE_H
