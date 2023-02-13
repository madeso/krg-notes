// "ecs" strcutrure

struct Guid
{
    uint8_t data[16];
};

/** A generic thing that you can't derive from.
*/
struct Entity
{
    Guid guid;
    std::vector<Component> components;
    EntitySystemUpdate systems;

    // dynamically add/remove components

    SpatialComponent* root_component = nullptr;
    bool is_spatial_entity() const { return root_component != nullptr; }

    void activate()
    {
        // thread safe?:
        //   register each componet with all local systems
        //   create per-stage local system update lists
        //   creates entity attachment (if required)
        // not thread safe:
        //   registers each componet with all global systems
    }

    void update(UpdateStage stage)
    {
        systems.update(stage);
    }
};

enum class EntityState
{
    /// all components are unloaded
    unloaded,
    
    /// all components are loaded
    loaded,
    
    /// entity has been "turned on" in the world and entity components have been registered with all systems
    activated
};

void attach(Entity* parent, Entity* child)
{
    assert(parent->is_spatial_entity() && child->is_spatial_entity());
    // ...
}

enum class ComponentState
{
    /// initial state
    unloaded,

    /// resource load is in progress
    loading,

    /// all resources loaded successfully
    loaded,

    /// some or all resources failed to load
    load_failed,

    /// component has been initialized (automatically called after loading successfull)
    initialized
};

/** Data storage.
 * 
 * Properties that define settings and resource.
 * 
 * * Components have no access to other components.
 * * Components have no access to the entity.
 * * Components have no default update.
 * * Inheritance of components is allowed.
 * * Can contain logic if needed (example: animation graph component contains everything related to the animation graph) and be viewed as a black box.
*/
struct ComponentType
{
    HashedStringView name;
    
    /// can the enttity have many components of this type
    bool max_one_per_entity;

    virtual Component* create() = 0;
};

/** Creates a new componet type for built-in components.
 * 
 * ```
 * CUSTOM_COMPONENT(CustomComponent, custom, "custom-name");
 * ```
*/
#define CUSTOM_COMPONENT(TYPE, NAME, HASH)\
struct TYPE : ComponentType\
{\
    constexpr TYPE() : ComponentType{HASH} {}\
    Component* create() const override;\
};\
constexpr TYPE NAME;

/**
 * * no access to other compoennts
 * * no acess to the entity
 * * no "default" update
 * * can be inherited
 *
 * Loading state switching between @ref ComponentState
    ```graphviz
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
    ```
*/
struct Component
{
    virtual ~Component() = default;

    Guid guid;

    /// for debug and tools
    std::string name;

    // settings that are serialized
    // resources that are loaded

    virtual void on_load(); virtual void on_unload();
    virtual void on_initialize(); virtual void on_shutdown();
};

struct SkeletalMeshComponent : Component
{
    // property: MeshResource(defines skeleton, bindpose, inverse bindpose)
    // init/shutdown: allocate pose storage based on mesh
};

struct SpatialComponent : Component
{
private:
    mat4f _local_transform;
    mat4f _global_transform;

public:
    void set_local_transform(const mat4f& m) { _local_transform = m; update_world_transform(); }
    

    const mat4f& get_local_transform() { return _local_transform; };
    const mat4f& get_global_transform() { return _global_transform; };

    void update_world_transform()
    {
        // calculate world transform based on world
        // update world bounds

        // update world transforms on children
        for(SpatialComponent* child: children)
        { child->update_world_transform(); }
    }

private:
    /// non-inclusive bounds in local space
    Obb local_bounds;

    /// non-inclusive bounds in world space
    Obb world_bounds;

    // parent spatial components + socket attachment
    Entity* parent;

    std::vector<SpatialComponent*> children;

    // only components that can reference other components
    // all components in graph must belong to the same entity (so same thread)
};

struct ComponentFactory
{
    void add(ComponentType* name);
    ComponentType* from_name(HashedStringView name);
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
constexpr int UpdateStageCount = UpdateStage::end_frame + 1;

struct EntitySystemWithPrio
{
    EntitySystem* system;
    int prio;
};

struct UpdateStageList
{
    std::vector<EntitySystemWithPrio> systems;

    void update(UpdateStage stage)
    {
        for(auto& es: systems)
        {
            es.system->update(stage);
        }
    }

    void add(EntitySystem* sys, int prio)
    {
        systems.emplace_back(sys, prio);
    }

    void remove(EntitySystem* sys)
    {
        swap_back_and_erase(&systems, [sys](const EntitySytemWithPrio& es) { return es.system == sys;});
    }
};

struct EntitySystemUpdate
{
    std::array<UpdateStageList, UpdateStageCount> systems;

    void update(UpdateStage stage)
    {
        systems[stage].update(stage);
    }

    void add(EntitySystem* sys, UpdateStage stage, int prio)
    {
        systems[stage].add(sys, prio);
    }

    void remove(EntitySystem* sys, UpdateStage stage)
    {
        systems[stage].remove(sys);
    }
};

/** A local system for a entity.
 * 
 * only one system of a specified type is allowed per entity
 * 
 * * It's manually added to a entity
 * * It can have entity specific data
 * * It can't reference other entities
 * 
 * Examples
 *  * animation (uses animation and mesh components)
 *  * movement
 *  * targeting
 */
struct EntitySystem
{
    // compoent requirements: requires and optionals for this system to work
    // tooling: user can see if the system is missing components 
};


/*
Design thoughts: should we have a single instance or a per-entity instance.
Currently it's written as a per-intity instance.

Pro:
* Simpler
* We can be sure it's thread safe as it won't integrate with other streads

Cons:
* Ugly
* Wastes memory as we need a instance of the system per entity


*/

/** A global system for the world.
 * 
 * Only one system of a specified type is allowed per world
 * 
 * Primary role is to handle global world state.
 * Secondary role is to do data transfer between entitites.
 * 
 * Examples:
 *  * skeletal mesh
 *  * physics
 *  * static mesh
 *  * navmesh
*/
struct WorldSystem
{
    // callback for when component is added to the world
};

struct WorldSystemUpdate
{
    void update(UpdateStage);
    void add(WorldSystem*, UpdateStage, int prio);
    void remove(WorldSystem*, UpdateStage stage);
};

struct World
{
    EntityList entities;
    WorldSystemUpdate systems;

    void update(UpdateStage s)
    {
        entities.update(s); // parallelized, spatial parent is updated before child (worker threads: nuber of cores - 1)
        systems.update(s); // sequential, can use worker threads if needed
    }
};