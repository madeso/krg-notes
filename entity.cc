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

    /// Load all components (resource, memory...)
    void load();

    /// Unload all components (resource, memory...)
    void unload();

    /** Turn the enity on in the world.
     * * register entity for all systems
     * * create update list
     * * create entity attachment
     * 
     * activate components/local systems paralellized with one enity per thread
     * activate world systems with one system per thread
    */ 
    void activate();

    /// Turn the entity off, remove entity from all systems
    void deactive();
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


/**
 * This is the only components that can reference other components.
 * All components in graph must belong to the same entity (so same thread), with the exception of spatial entities that are attached to other spatial entitites.
 * It's still safe since it's hidden from the user and sheduling ensures it's updated in order and on the same thread.
*/
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

    /// can reference other spatial components: https://youtu.be/jjEsB611kxs?t=6639
    /// @todo find out if this can reference entities in other components or not
    std::vector<SpatialComponent*> children;
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
struct RequestedComponents
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
    /// get requested components
    /// @todo move to a entity sytem type
    virtual RequestedComponents get_component_requests() = 0;

    /*
    design thought: who removes this from the update list when it's destroyed?
    better to return a register request and let the caller add/remove stages+prios to the update list?
    */
    virtual void register_updates(EntitySystemUpdate* updates) = 0;

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
 * 
 * Examples:
 *  * skeletal mesh
 *  * physics
 *  * static mesh
 *  * navmesh
 * 
 * @todo Add types similar to ComponentType and ComponentFactory
*/
struct WorldSystem
{
    virtual void register_updates(WorldSystemUpdate* updates) = 0;

    /*
    Design thought: if this is used to transfer data between components,
    should there be some ui hint system similar to get_component_requests?
    */

    /// Called by WorldSystemUpdate
    virtual void update(UpdateStage stage) = 0;

    virtual void system_was_added_to_world() = 0;
    virtual void system_was_removed_from_world() = 0;

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
        // parallelized, spatial parent is updated before child (worker threads: nuber of cores - 1)
        // place attached entities on the same thread as parent, schedule parent to update before the child
        entities.update(s);

        // sequential, can use worker threads if needed
        systems.update(s);
    }
};

namespace example
{

struct AnimationGraphComponent
{
    Skeleton* skeleton;
    Pose calc_pose();
};
struct SkeletalMeshComponent
{
    // property: MeshResource(defines skeleton, bindpose, inverse bindpose)
    // init/shutdown: allocate pose storage based on mesh

    Skeleton* skeleton;
    void set_pose(const Pose& pose);
};

/** Example demonstrating a EnitySystem.
 * * Animation system is repsonsible for executing animation tasks and trasferring
 * the results to the skeletal mesh component
 * * Domain specific logic sis kept within the indivual components
 * * The logic for what updates and what data to transfer is in the system.
*/
struct SimpleAnimationSystem : EntitySystem
{
    AnimationGraphComponent* animation_graph;
    SkeletalMeshComponent* mesh;

    void update(UpdateStage) override
    {
        if(animation_graph->skeleton == mesh->skeleton)
        {
            mesh->set_pose(animation_graph->calc_pose());
        }
    }
};

/// can have multiple meshes, or let the animationgraph return multiple poses/skeletons (custmization system)
struct SimpleAnimationSystem : EntitySystem
{
    AnimationGraphComponent* animation_graph;
    std::vector<SkeletalMeshComponent*> meshes;

    void update(UpdateStage) override
    {
        auto pose = animation_graph->calc_pose();
        for(SkeletalMeshComponent* mesh: meshes)
        {
            if(animation_graph->skeleton == mesh->skeleton)
            {
                mesh->set_pose(pose);
            }
        }
    }
};

}

}

