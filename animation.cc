// https://www.youtube.com/watch?v=Jkv0pbp0ckQ

// ===========================================================================
// SKINNING

// skinning = mesh vertices weighted agains some set of world space transforms ("bones")
// all "bones" are in world space, no hierchy/skeleton
// multipart character: mesh might not reference all core bones

// deformation helper bones:
// deformation issues is solved with procedurally generated "deformation helper bones":
// * roll/twist bones, twist with a percent of the parent
// * pokes
// * look at
// a topic of its own https://youtu.be/Jkv0pbp0ckQ?t=958

// rigid strucutre solvers: 
// static pieces that needs to be animated, based on a a set of bones, specify transform of other bones
// * set-driven keys
// * RBF (Radial Basis Function) solver

// bind pose = position of bones when weighting vertices
// reference/rest pose = default pose when animating

// skeleton
// core bones
// procedurally calcualted bones, used for deformation and rendering (depending on the mesh/clothing, the number of bones could change drastically, 10-15 => 25-50)
// * helper bones
// * rigid structure solvers
// needs ability to treat "body", facial and cape animations seperate. Body is blended with different animations, face is set once and cape is simulated. Easier to optimize

// core "animation skeleton" defined (average male, large female, bipedal mech, horse...)
// animators animate this skeleton using control rig

// helper bones for gameplay
// * attachment bones(holsters, weapon bones etc..)
// * anchor bones(anchor character in environment)
// * ik targets/offsets (environment ik)

// regular mesh with animation skeleton
struct Mesh
{
    // mesh data with weights
    // mesh skeleton with skellington bones
    // functions to procedurally calculate other bones
};

// animation skeleton
struct Skellington
{
};

// contain core bones in bone space
struct Pose
{
};

// another struct to contain gameplay bones?

// ===========================================================================
// ANIMATION

// Pose = set of transformation for all bones in a skeleton
// Animation Pose = Pose using animation skeleton, generally stored in bone space
// Render Pose = Pose using render skeleton, generally converted to character space, and then relative to bind pose for skinning


// Animation = Set of Poses that produce motion
// when imported the animation is sampled (preferable at the same fps that it was authred at)
// common techniques: curve fitting, quantization+RLE
// https://www.reddit.com/r/GraphicsProgramming/comments/ffhjqt/an_idiots_guide_to_animation_compression/
// https://takinginitiative.net/2020/03/07/an-idiots-guide-to-animation-compression/

// Sampling = read transform of the keyframes we are between and interpolate

// Blending = is roughly interpolation over time
// Two types: Interpolative and Additive
// Interpolative: blend one transform to another: translation+scale = lerp, rotation=slerp (nlerp can cause problems with low fps and helper bones)
// Additive: Authored with a reference pose, stored as offsets that are then additive applied to the "source" pose with some percentage

// sometime you might want to blend things globally

struct InterpolativeBlend
{
    static quat rotation(quat from, quat to, float t) { return slerp(from, to, t); }
    static vec3 translation(vec3 from, vec3 to, float t) { return lerp(from, to, t); }
    static vec3 scale(vec2 from, vec3 to, float t) { return lerp(from, to, t); }
};
struct AdditiveBlend
{
    // operator*
    // Concatenate the rotation of this onto rhs and return the result i.e. first rotate by rhs then by this
    // This means order of rotation is right-to-left: child-rotation * parent-rotation
    static quat rotation(quat from, quat to, float t) { return slerp(from, from * to, t); }
    static vec3 translation(vec3 from, vec3 to, float t) { return to*t + from; }
    static vec3 scale(vec2 from, vec3 to, float t) { return to*t + from; }
};
template<typename TBlend>
void local_blend(Pose source_pose, Pose target_pose, float blend_weight, BoneMask mask, Pose* result)
{
    for(int bone_id: result->bone_count)
    {
        auto weight = calc_bone_weight(blend_weight, mask, bone_id);
        if(weight == 0)
        {
            result->transform[bone_id] = source_pose.transform[bone_id];
            continue;
        }
        const auto source = source_pose.transform[bone_id];
        const auto target = target_pose.transform[bone_id];

        const auto translation = TBlend::translation(source.translation, target.translation, weight);
        const auto rotation = TBlend::rotation(source.rotation, target.rotation, weight);
        const auto scale = TBlend::scale(source.scale, target.scale, weight);
        result->transform[bone_id] = {translation, rotation, scale};
    }
}

// ===========================================================================
// Bone space: relative to parent bone
// Character space: Relative to character root
// World space: Relative to scene origin

// Bone mask: weight per bone in the animation skeleton
// tool support: define shoulder 100%, hand 20% and feather bones in between


