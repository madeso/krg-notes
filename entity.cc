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

    bool is_spatial_entity() const { return root != nullptr; }
    SpatialComponent* root = nullptr;

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

struct SpatialComponent : Component
{
private:
    mat4f _local_transform;
    mat4f _global_transform;

    void set_global_transform(const mat4f& m) { _global_transform = m; }

public:
    void set_local_transform(const mat4f& m) { _local_transform = m; local_transform_has_changed(); }
    

    const mat4f& get_local_transform() { return _local_transform; };
    const mat4f& get_global_transform() { return _global_transform; };

    void local_transform_has_changed()
    {
        // calculate world transform based on world
        // update world bounds
        // update world transforms on children
    }

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

struct EntitySystemUpdate
{
    void update(UpdateStage);
    void add(EntitySystem*, UpdateStage, int prio);
    void remove(EntitySystem*, UpdateStage, int prio);
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
 *  * animation
 *  * movement
 *  * targeting
 */
struct EntitySystem
{
    // compoent requirements: requires and optionals for this system to work
    // tooling: user can see if the system is missing components 
};


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