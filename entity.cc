// "ecs" strcutrure

namespace core
{

/// external type 
struct Guid {};

/// external type
struct mat4f {};
}

namespace entity
{

/**
 * A generic thing that you can't derive from.
 * @todo flesh out documentation
*/
struct Entity
{
    core::Guid guid;
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


/** Basic data storage.
 * 
 * Properties that define settings and resource.
 * 
 * * Components have no access to other components.
 * * Components have no access to the entity.
 * * Components have no default update.
 * * Inheritance of components is allowed.
 * * Can contain logic if needed (example: animation graph component contains everything related to the animation graph) and be viewed as a black box.
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

    core::Guid guid;

    /// for debug and tools
    std::string name;

    // settings that are serialized
    // resources that are loaded

    virtual void on_load(); virtual void on_unload();
    virtual void on_initialize(); virtual void on_shutdown();
};

namespace example
{
    struct SkeletalMeshComponent : Component
    {
        // property: MeshResource(defines skeleton, bindpose, inverse bindpose)
        // init/shutdown: allocate pose storage based on mesh
    };
}

struct SpatialComponent : Component
{
private:
    core::mat4f _local_transform;
    core::mat4f _global_transform;

public:
    void set_local_transform(const core::mat4f& m) { _local_transform = m; update_world_transform(); }
    

    const core::mat4f& get_local_transform() { return _local_transform; };
    const core::mat4f& get_global_transform() { return _global_transform; };

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

/** Helpful factory to create Component.
*/
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


/** Updates EntitySystem in the right order.
*/
struct EntitySystemUpdate
{
    struct EntitySystemWithPrio
    {
        EntitySystem* system;
        int prio;
    };

    struct UpdateStageList
    {
        std::vector<EntitySystemWithPrio> systems;

        /*
        design thought: should this be a regular vector sorted manually
        or a 
        */

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

/** Required and optional components for EntitySystem and WorldSystem.
Contains requireed and optional components for a system to work.
With tooling a user can see if the added system is missing components.

Example: Character animation system requires Animation and Skeletal Mesh and has a optional Cloth.
*/
struct RequiredComponents
{
    std::vector<ComponentType*> required;
    std::vector<ComponentType*> optional;
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
    /// get required components
    /// @todo move to a entity sytem type
    virtual RequiredComponents get_requirements() = 0;

    /// Called by EntitySystemUpdate
    virtual void update(UpdateStage stage) = 0;

    /*
    design thoughts:
    should we get a callback for each component, or a general... here is a the latest state of the components you care about
    current setup is probably easier to implement but requirements could be different from actual usage

    keep in mind that the system is per entity so it's just getting references to components and storing them as member variables
    */

    /// a component was added or activated on the entity
    virtual void component_was_added(Component* c) = 0;

    /// a component was removed or deactivated on the entity
    virtual void component_was_removed(Component* c) = 0;
};


/*
Design thoughts: should we have a single instance or a per-entity instance.
Currently it's written as a per-entity instance.

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
    /// get required components
    virtual RequiredComponents get_requirements() = 0;

    /// Called by WorldSystemUpdate
    virtual void update(UpdateStage stage) = 0;

    /// a component was added or activated on a entity
    virtual void component_was_added(Entity* ent, Component* c) = 0;

    /// a component was removed or deactivated on a entity
    virtual void component_was_removed(Entity* ent, Component* c) = 0;
};

/** Updates WorldSystem.
*/
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

}

