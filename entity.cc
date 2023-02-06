// "ecs" strcutrure

struct Entity
{
    int guid;
    Component components[];
    EntitySystemUpdate systems;

    // dynamically add/remove components

    bool is_spatial_entity() const { return root != nullptr; }
    SpatialComponent* root;

    void activate()
    {
        // thread safe?:
        //   register each componet with all local systems
        //   create per-stage local system update lists
        //   creates entity attachment (if required)
        // not thread safe:
        //   registers each componet with all global systems
    }
};

enum class EntityState
{
    // all components are unloaded
    unloaded,
    
    // all components are loaded
    loaded,
    
    // entity has been "turned on" in the world
    // entity components have been registered with all systems
    activated
};

void attach(Entity* parent, Entity* child)
{
    assert(parent->is_spatial_entity() && child->is_spatial_entity());
    // ...
}

enum class ComponentState
{
    // initial state
    unloaded,

    // resource load is in progress
    loading,

    // all resources loaded successfully
    loaded,

    // some or all resources failed to load
    load_failed,

    // component has been initialized (automatically called after loading successfull)
    initialized
};

struct ComponentType
{
    int id;
    const char* name;
    bool max_one_per_entity; // can the enttity have amny components of this type
};

struct Component
{
    int guid;
    string name; // debug and tools

    // settings that are serialized
    // resources that are loaded

    // no access to other compoennts
    // no acess to the entity
    // no "default" update
    // can be inherited

    /*
    digraph G
    {
        unloaded -> loading [label="load()"];
        loading -> unloaded [label="unload()"];

        loading_failed -> unloaded [label="unload()"];
        loaded -> unloaded [label="unload()"];

        loading -> loading_failed;
        loading -> loaded;

        loaded -> initialized[label="initialize()"];
        initialized -> loaded[label="shutdown()"];
    }
    */
    void on_load(); void on_unload();
    void on_initialize(); void on_shutdown();
};

struct SpatialComponent : Component
{
    mat4 local_transform;
    mat4 global_transform; // private set

    void local_transform_has_changed()
    {
        // calculate world transform based on world
        // update world bounds
        // update world transforms on children
    }
`
    private:
    // non-inclusive
    Obb local_bounds;
    Obb world_bounds;

    // parent spatial components
    // only components that can reference other components
    // all components in graph must belong to the same entity
};

struct ComponentFactory
{
    void add(ComponentType* name, CreatorFunc);
    ComponentType* from_name(string name);
    Component* create(ComponentType* name);
};

enum class UpdateStage
{
    start_frame,
    before_physics,
    physics,
    after_physics,
    end_frame
};

struct EntitySystemUpdate
{
    void update(UpdateStage);
    void add(EntitySystem*, UpdateStage, int prio);
    void remove(EntitySystem*, UpdateStage, int prio);
};

// can't reference other entities
struct EntitySystem
{
    // manually added to a entity
    // can have entity specific data

    // compoennt requirements: requires and optionals for this system to work
    // tooling: user can see if the system is missing components 
};

// only one system of a specified ype is allowed per owner (entity/world)

// entity/local systems: animation, movement, targeting
// world/global systems: skeletal mesh, physics, static mesh, navmesh

struct WorldSystem
{
    // primary role: handle global world state
    // secondary role: data transfer between entitites

    // callback for when component is added to the world
};

struct World
{
    EntityList entities;
    EntitySystemUpdate systems;

    void update(UpdateStage s)
    {
        entities.update(s); // parallelized, spatial parent is updated before child (worker threads: nuber of cores - 1)
        systems.update(s); // sequential, can use worker threads if needed
    }
};