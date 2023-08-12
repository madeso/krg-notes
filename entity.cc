// "ecs" strcutrure: https://www.youtube.com/watch?v=jjEsB611kxs&t=7072s
/*
Dictionary:
    Entity Component System
    ECS - Individual unique entities which have components
    ECS appear mostly in dedicated engines

    Actor Component System
    ACS - Precomposed actors with components and behaviours
    UE uses ACS

    Script Component System
    SCS - Game objects described wholely by scripts or sometimes an amalgamation of scripts and ECS-style components
    Unity uses SCS
*/
#include <vector>
#include <string>
#include <cassert>
#include <array>
#include <algorithm>
#include <string_view>
#include <unordered_map>
#include <memory>


namespace core
{
    /// external type 
    struct Guid {};

    /// external type
    struct mat4f {};

    /// external type
    struct Obb {};

    // struct HashedStringView {};
    using HashedStringView = std::string_view;

    template<typename T, typename F>
    void update_and_erase(std::vector<T>* asrc, F&& update)
    {
        assert(asrc);
        std::vector<T>& src = *asrc;

        std::size_t index = 0;
        std::size_t count = src.size();
        while(index < count)
        {
            const bool remove = update(src[index]);
            if(remove)
            {
                const auto last_index = count-1;
                if(index != last_index)
                {
                    std::swap(src[index], src[last_index]);
                }
                count -= 1;
                src.pop_back();
            }
            else
            {
                index += 1;
            }
        }
    }
}

namespace entity
{





    ///////////////////////////////////////////////////////////////////////////////////////////////
    // forward declare

    struct EntitySystemWithPrio;
    struct EntitySystemUpdateStageList;
    struct EntitySystemUpdate;

    struct WorldSystemWithPrio;
    struct WorldSystemUpdateStageList;
    struct WorldSystemUpdate;

    struct Entity;
    struct Component;
        struct ComponentType;
        struct ComponentFactory;
    struct SpatialComponent;
    struct RequestedComponents;
    struct EntitySystem;
        struct EntitySystemType;
        struct EntitySystemFactory;
    struct WorldSystem;
        struct WorldSystemType;
        struct WorldSystemFactory;
    struct World;




    ///////////////////////////////////////////////////////////////////////////////////////////////
    // enums

    enum class UpdateStage
    {
        start_frame,
        before_physics,
        physics,
        after_physics,
        end_frame
    };
    constexpr unsigned int UpdateStageCount = static_cast<unsigned int>(UpdateStage::end_frame) + 1;

    enum class EntityState
    {
        /// all components are unloaded
        unloaded,
        
        /// all components are loaded
        loaded,
        
        /// entity has been "turned on" in the world and entity components have been registered with all systems
        activated
    };

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




    ///////////////////////////////////////////////////////////////////////////////////////////////
    // "headers"

    struct Alive
    {
        void kill();
        void update_for_frame();

        bool is_pending_removal() const;
        bool delete_owner() const;
    private:
        int health = 0;
    };


    // assume there's a Alive called alive on T
    template<typename T>
    void update_and_remove_alives
    (
        std::vector<std::unique_ptr<T>>* alives,
        std::vector<std::unique_ptr<T>>* deads
    )
    {
        core::update_and_erase(deads,
            [](std::unique_ptr<T>& c) -> bool
            {
                c->alive.update_for_frame();
                return c->alive.delete_owner();
            }
        );
        core::update_and_erase(alives,
            [deads](std::unique_ptr<T>& c) -> bool
            {
                c->alive.update_for_frame();
                if(c->alive.is_pending_removal())
                {
                    // move to dead to keep alive for a few more frames
                    deads->emplace_back(std::move(c));
                    return true;
                }
                else
                {
                    return false;
                }
            }
        );
    }


    struct EntitySystemWithPrio
    {
        EntitySystemWithPrio(EntitySystem* s, int p);

        EntitySystem* system;
        int prio;
    };

    struct EntitySystemUpdateStageList
    {
        void update(UpdateStage stage);
        void add(EntitySystem* sys, int prio);
        void remove(EntitySystem* sys);

    private:
        std::vector<EntitySystemWithPrio> systems;
        // can't iterate a std::priority_queue so let's not use that for now
    };

    /** Updates EntitySystem in the right order.
    */
    struct EntitySystemUpdate
    {
        std::array<EntitySystemUpdateStageList, UpdateStageCount> systems;

        void update(UpdateStage stage);
        void add(EntitySystem* sys, UpdateStage stage, int prio);
        void remove(EntitySystem* sys, UpdateStage stage);
    };


    /**
     * A generic thing that you can't derive from.
     * @todo flesh out documentation
    */
    struct Entity
    {
        core::Guid guid;
        std::vector<std::unique_ptr<Component>> components;
        std::vector<std::unique_ptr<Component>> dead_components;
        EntitySystemUpdate systems;

        // dynamically add/remove components

        Component* root_component = nullptr;
        bool is_spatial_entity() const { return root_component != nullptr; }

        /** Turn the enity on in the world.
         * * register entity for all systems
         * * create update list
         * * create entity attachment
         * 
         * activate components/local systems paralellized with one enity per thread
         * activate world systems with one system per thread
        */ 
        void activate()
        {
            // thread safe?:
            //   register each componet with all local systems
            //   create per-stage local system update lists
            //   creates entity attachment (if required)
            // not thread safe:
            //   registers each componet with all global systems
        }

        /// Turn the entity off, remove entity from all systems
        void deactive();

        void update(UpdateStage stage);

        /// Load all components (resource, memory...)
        void load();

        /// Unload all components (resource, memory...)
        void unload();

        Alive alive;
    };

    void attach(World* world, Entity* parent_id, Entity* child_id);


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

        Alive alive;

        // settings that are serialized
        // resources that are loaded

        virtual void on_load(); virtual void on_unload();
        virtual void on_initialize(); virtual void on_shutdown();
    };


    struct ComponentType
    {
        core::HashedStringView name;
        
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
        std::unique_ptr<Component> create() const override;\
    };\
    constexpr TYPE NAME;

    /** Helpful factory to create Component.
    */
    struct ComponentFactory
    {
        void add(const ComponentType* name);
        const ComponentType* from_name_or_null(core::HashedStringView name) const;
    private:
        std::unordered_map<core::HashedStringView, const ComponentType*> types;
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
            {
                child->update_world_transform();
            }
        }

    private:
        /// non-inclusive bounds in local space
        core::Obb local_bounds;

        /// non-inclusive bounds in world space
        core::Obb world_bounds;

        // parent spatial components + socket attachment
        Entity* parent;

        /// can reference other spatial components: https://youtu.be/jjEsB611kxs?t=6639
        /// @todo find out if this can reference entities in other components or not
        std::vector<SpatialComponent*> children;
    };


    /** Required and optional components for EntitySystem and WorldSystem.
    Contains requireed and optional components for a system to work.
    With tooling a user can see if the added system is missing components.

    Example: Character animation system requires Animation and Skeletal Mesh and has a optional Cloth.
    */
    struct RequestedComponents
    {
        std::vector<const ComponentType*> required;
        std::vector<const ComponentType*> optional;
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


    struct EntitySystemType
    {
        core::HashedStringView name;

        EntitySystemType(core::HashedStringView);
        virtual ~EntitySystemType() = default;

        virtual std::unique_ptr<EntitySystem> create() = 0;
    };

    struct EntitySystemFactory
    {
        void add(const EntitySystemType* ny);
        const EntitySystemType* from_name_or_null(core::HashedStringView name);

    private:
        std::unordered_map<core::HashedStringView, const EntitySystemType*> types;
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

    struct WorldSystemType
    {
        core::HashedStringView name;

        WorldSystemType(core::HashedStringView);
        virtual ~WorldSystemType() = default;

        virtual std::unique_ptr<WorldSystem> create() = 0;
    };

    struct WorldSystemFactory
    {
        void add(const WorldSystemType* ty);
        const WorldSystemType* from_name_or_null(core::HashedStringView name);

    private:
        std::unordered_map<core::HashedStringView, const WorldSystemType*> types;
    };

    struct WorldSystemWithPrio
    {
        WorldSystemWithPrio(WorldSystem* s, int p);

        WorldSystem* system;
        int prio;
    };

    struct WorldSystemUpdateStageList
    {
        void update(UpdateStage stage);
        void add(WorldSystem* sys, int prio);
        void remove(WorldSystem* sys);

    private:
        std::vector<WorldSystemWithPrio> systems;
    };

    /** Updates WorldSystem.
    */
    struct WorldSystemUpdate
    {
        void update(UpdateStage);

        void add(WorldSystem*, UpdateStage, int prio);
        void remove(WorldSystem*, UpdateStage stage);

    private:
        std::array<WorldSystemUpdateStageList, UpdateStageCount> systems;
    };

    struct World
    {
        void update(UpdateStage s);

    private:
        std::vector<std::unique_ptr<Entity>> entities;
        std::vector<std::unique_ptr<WorldSystem>> systems;
        WorldSystemUpdate system_update;
    };





    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Implementations

    void Alive::kill()
    {
        if(health == 0)
        {
            health = 1;
        }
    }

    void Alive::update_for_frame()
    {
        if(health > 0)
        {
            health += 1;
        }
    }

    bool Alive::is_pending_removal() const
    {
        return health > 0;
    }

    bool Alive::delete_owner() const
    {
        return health > 10;
    }


    // ------------------------------------------------------------------------
    // EntitySystemWithPrio

    EntitySystemWithPrio::EntitySystemWithPrio(EntitySystem* s, int p)
        : system(s)
        , prio(p)
    {
    }


    // ------------------------------------------------------------------------
    // EntitySystemUpdateStageList

    void EntitySystemUpdateStageList::update(UpdateStage stage)
    {
        for(auto& es: systems)
        {
            es.system->update(stage);
        }
    }

    void EntitySystemUpdateStageList::add(EntitySystem* sys, int prio)
    {
        systems.emplace_back(sys, prio);
        std::sort(systems.begin(), systems.end(), [](const auto& lhs, const auto& rhs)
        {
            return lhs.prio < rhs.prio;
        });
    }

    void EntitySystemUpdateStageList::remove(EntitySystem* sys)
    {
        core::update_and_erase(&systems,
            [sys](const EntitySystemWithPrio& es)
            { return es.system == sys;}
        );
    }


    // ------------------------------------------------------------------------
    // EntitySystemUpdate

    void EntitySystemUpdate::update(UpdateStage stage)
    {
        systems[static_cast<std::size_t>(stage)].update(stage);
    }

    void EntitySystemUpdate::add(EntitySystem* sys, UpdateStage stage, int prio)
    {
        systems[static_cast<std::size_t>(stage)].add(sys, prio);
    }

    void EntitySystemUpdate::remove(EntitySystem* sys, UpdateStage stage)
    {
        systems[static_cast<std::size_t>(stage)].remove(sys);
    }


    // ------------------------------------------------------------------------
    // ComponentType

    // ------------------------------------------------------------------------
    // ComponentFactory
    void ComponentFactory::add(const ComponentType* ty)
    {
        types.insert({ty->name, ty});
    }
    
    const ComponentType* ComponentFactory::from_name_or_null(core::HashedStringView name) const
    {
        auto found = types.find(name);
        if(found != types.end()) { return found->second; }
        else { return nullptr; }
    }

    // ------------------------------------------------------------------------
    // EntitySystemType

    // ------------------------------------------------------------------------
    // EntitySystemFactory
    void EntitySystemFactory::add(const EntitySystemType* ty)
    {
        types.insert({ty->name, ty});
    }

    const EntitySystemType* EntitySystemFactory::from_name_or_null(core::HashedStringView name)
    {
        auto found = types.find(name);
        if(found != types.end()) { return found->second; }
        else { return nullptr; }
    }

    // ------------------------------------------------------------------------
    // WorldSystemType

    // ------------------------------------------------------------------------
    // WorldSystemFactory
    void WorldSystemFactory::add(const WorldSystemType* ty)
    {
        types.insert({ty->name, ty});
    }

    const WorldSystemType* WorldSystemFactory::from_name_or_null(core::HashedStringView name)
    {
        auto found = types.find(name);
        if(found != types.end()) { return found->second; }
        else { return nullptr; }
    }


    // ------------------------------------------------------------------------
    // Entity

    void Entity::update(UpdateStage stage)
    {
        systems.update(stage);

        if(stage == UpdateStage::end_frame)
        {
            alive.update_for_frame();
            update_and_remove_alives(&components, &dead_components);
        }
    }

    // ------------------------------------------------------------------------
    // Component

    // ------------------------------------------------------------------------
    // SpatialComponent

    void attach(World* world, Entity* parent, Entity* child)
    {
        assert(parent->is_spatial_entity() && child->is_spatial_entity());
        // todo(Gustav): implement attachments
    }

    // ------------------------------------------------------------------------
    // RequestedComponents

    // ------------------------------------------------------------------------
    // EntitySystem

    // ------------------------------------------------------------------------
    // WorldSystem

    // ------------------------------------------------------------------------
    // WorldSystemWithPrio
    WorldSystemWithPrio::WorldSystemWithPrio(WorldSystem* s, int p)
        : system(s)
        , prio(p)
    {
    }

    // ------------------------------------------------------------------------
    // WorldSystemUpdateStageList
    void WorldSystemUpdateStageList::update(UpdateStage stage)
    {
        for(auto& es: systems)
        {
            es.system->update(stage);
        }
    }

    void WorldSystemUpdateStageList::add(WorldSystem* sys, int prio)
    {
        systems.emplace_back(sys, prio);
        std::sort(systems.begin(), systems.end(), [](const auto& lhs, const auto& rhs)
        {
            return lhs.prio < rhs.prio;
        });
    }

    void WorldSystemUpdateStageList::remove(WorldSystem* sys)
    {
        swap_back_and_erase(&systems, [sys](const WorldSystemWithPrio& es) { return es.system == sys;});
    }

    // ------------------------------------------------------------------------
    // WorldSystemUpdate

    void WorldSystemUpdate::update(UpdateStage stage)
    {
        systems[static_cast<std::size_t>(stage)].update(stage);
    }

    void WorldSystemUpdate::add(WorldSystem* system, UpdateStage stage, int prio)
    {
        systems[static_cast<std::size_t>(stage)].add(system, prio);
    }

    void WorldSystemUpdate::remove(WorldSystem* system, UpdateStage stage)
    {
        systems[static_cast<std::size_t>(stage)].remove(system);
    }

    // ------------------------------------------------------------------------
    // World

    void World::update(UpdateStage stage)
    {
        // todo(Gustav): implement threading for entity
        // todo(Gustav): take care of updating parent before child
        // parallelized, spatial parent is updated before child (worker threads: nuber of cores - 1)
        // place attached entities on the same thread as parent, schedule parent to update before the child
        for(auto& ent: entities)
        {
            ent->update(stage);
        }
        
        // todo(Gustav): implement threading for world
        // sequential, can use worker threads if needed
        system_update.update(stage);
    }


    

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Examples
#if 0
    namespace example
    {
        struct Skeleton {};
        struct Pose {};

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
        struct SimpleSingleAnimationSystem : EntitySystem
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
        struct SimpleMultiAnimationSystem : EntitySystem
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
#endif
}

int main(int arc, char** argv)
{
    return 0;
}
