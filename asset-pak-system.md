# Asset system

## Why:
* Loading many small files are slow
* Deleting many small things are slow
* Don't want to track many small things that may or may not be loaded

## Design
* Assets lives in PAKs or WADs to a Block
* Blocks are loaded on a stack
* Asset in blocks can only reference stacks loaded earlier

```cpp
struct Wad
{
    Texture[] textures;
    Material[] materials;
    Mesh[] meshes;
    Room[] rooms;
    PhysicsMaterial[] physics_materials;
    Custom[] custom_data;
}
```
* Need to allow custom data.
* Can custom data extend existing types? for example adding extra data to physic materials?

## Possible setup:
* UI assets
* Common game assets
* Level theme assets (snow player)
* Actual level assets

This allows us to easily unload assets for a specific level, loading becomes easier since we know exactly what will be loaded and can easily add a progress bar.

* A full "stack" is specified and loaded, never parts of the stack (though one may use "templates" as this stack is the `"snow_level"` stack but with `"snow_01.lvl"` added ontop)
* Streaming can be solved by temporaily allowing a branch in the stack and do the actual pop/push when switching area.
* Asset handles need to reference the block and index and block revision. No revision on actual asset since it's part of the block.

## Issues
* Packing can be a problem during dev.
  This could be solved by having a slower vfs/filesystem pak implementation and switch (with DI?) to regular pak for dist builds
* Can be hard to author data.
  This could be solved by placing assets in special folders and have a vfs like build step where one could author the "stack tree"
* No paths can be specified in code.
  This could both be a good thing since assets should be provided(perhaps via a something similar to typescript interfaces), this would also be useful to make validating data easier. This could also be solved by having a pre-compilation step that replaces paths with actual references...


# Data files

Slightly related to pak files, data files are general containers for data. Partly inspired by [scriptable objects from unity](https://www.youtube.com/watch?v=6vmRwLYWNRo).

## Uscase 1: Data inheritence

```cpp
struct Entity
{
    string group;
    string name;
    int hitpoints;
    int damage;
};

namespace assets
{
    // specified by a designer in the editor
    abstract Entity Undead = { .group = "Undead"; };
    abstract Entity Basic = { .damage = 10; .hitpoints = 10; };
    Entity Gnome inherits Basic, Undead = { .name = "Gnome"; };
    Entity Zombie inherits Basic, Undead = { .name = "Zombie; .hitpoints = 100; };
}

Entity ent = grab_from_editor<Entity>();
spawn(ent);
spawn(assets::Zombie);
```
* data is set by a designer in a editor
* data inherit from other data files and "overwrite" the defaults
* when data is compiled objects are flattened and inheritence is removed

## Usecase 2: 
Add script function in data block and allow other datablock inherit it with "custom" self script functions.

```cpp
struct PowerupEffect {
    fun apply(Player);
}
collidedWith((powerup, player) => {
  powerup.apply(player);
});

struct HealthBuff : PowerupEffect {
    float ammount;
    fun apply(player p) {
        p.health += ammount;
    }
}

struct Instakill : PowerupEffect {
    Effect effect;
    fun apply(Player p) {
        p.kill();
        effect.spawn();
    }
}
```
# Data storage model

```cpp
struct Object
{
    GUID id; // root = 0
    map<string, Value> properties;
};

struct Value = null, bool, double, vector3, quaternaion, string, data, guid, set<guid>


Object root;
```

Change lists are easy to create and merge, if there is a conflict then pick latest or ask to user/highlight the change

* create/destroy object
* set property 
* add/remove from set

