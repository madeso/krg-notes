// logic to play footstep sounds when walking on different materials
// inspired by Ovani Sounds
// get inspiration from jedi knight engine

#include <vector>
#include <string>
#include <optional>

template <typename T>
struct Handle
{
    int index;
    const T *operator->() const;

    const operator bool() const { return index != 0; }
    constexpr static inline Handle<T> null_ptr = {0};
};

struct Texture
{
    std::string resource_name;
};
struct Material
{
    std::string foot_material; // @see MaterialSpecification
    std::vector<Handle<Texture>> get_albedo_or_all() const;
};

/// reference to a list of sounds that can be randomly played with some small pitch variations
struct SoundEvent;

/// actual sound resoource: examples are snow, water and wood
struct MaterialSpecification
{
    std::string display_name;
    std::vector<std::string> auto_tags;

    Handle<SoundEvent> soft_steps; /// used for crouching
    Handle<SoundEvent> med_steps;  /// usef for walking
    Handle<SoundEvent> hard_steps; /// usef for running
    Handle<SoundEvent> scuffs;     /// played whn the character stopped moving (in a fixed update each 1/8s check speed if current speed is low but was "high" half a second ago, play scuff)
    Handle<SoundEvent> jumps;      /// jumping off
    Handle<SoundEvent> landings;   /// landing
};

// resource describing the "foot", example: barefoot, shoes, etc
struct FootProfile
{
    std::string display_name;
    std::vector<Handle<MaterialSpecification>> materials;
};

// fill out a material with the sfx references, call this for each foot profile
Handle<MaterialSpecification> find_material(FootProfile foot, std::string name)
{
    name = name_after_slash(name);
    for (const auto &mat_spec : foot.materials)
    {
        for (const auto tag : mat_spec->auto_tags)
        {
            if (name.contains(tag))
            {
                return mat_spec;
            }
        }
    }

    return Handle<MaterialSpecification>::null_ptr;
}
Handle<MaterialSpecification> automate_mat_spec(Material mat, FootProfile foot_profile)
{
    auto found = find_material(foot_profile, mat.foot_material);
    if (found)
    {
        return found;
    }

    for (auto texture : mat.get_albedo_or_all())
    {
        auto found = find_material(foot_profile, texture->resource_name);
        if (found)
        {
            return found;
        }
    }

    return Handle<MaterialSpecification>::null_ptr;
}