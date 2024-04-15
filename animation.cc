namespace std
{
    template <typename T>
    struct vector{};

    template<typename T> using optional = T*;
}
template<typename T> bool is_within(T, T, T);
struct mat4{};
struct vec3{};
struct quat{};
void assert(bool);

// Animation ((not just sampling and blending)
// https://www.youtube.com/watch?v=Jkv0pbp0ckQ

// todo: add "generate a pose pool" somewhere

// ===========================================================================
// SKINNING

// skinning = mesh vertices weighted agains some set of world space transforms ("bones")
// all "bones" are in world space, no hierchy/skeleton
// multipart character: mesh might not reference all core bones

// deformation helper bones:
// deformation issues is solved with procedurally calculated "deformation helper bones":
// * roll/twist bones, that take a percent of the parent
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
    // mesh skeleton
	// mapping function that take a skellington and maps to actual bones
    // and procedurally calculate other bones

// why seperated? reuse anim, support different characters, multipart character/add clothing support
// clothing can change bone count
// torso: tank-top 10-15 bones, jacket 25-50, jacket introduce RBF
// different render/mesh skeletons based on lod?
};

// gameplay/animation skeleton
// core bones + gameplay bones (shared asset on disk: female01, male01, horse01)
struct Skellington
{
	// parent data
};

struct Transform { vec3 translation; quat rotation; vec3 scale; };
// contain core bones in bone space
using Pose = std::vector<Transform> transforms;


// sample source animation at some fps, function to extract pose from animation time
// interpolate between two keyframes
Pose sample_animation(const Animation&, float t);

// another struct to contain gameplay bones?

// ===========================================================================
// ANIMATION

// Pose = set of transformation for all bones in a skeleton
// Animation Pose = Pose using animation skeleton, generally stored in bone space
// Render Pose = Pose using render skeleton, generally converted to character space, and then relative to bind pose for skinning


// Animation = (roughly) a set of Poses that produce motion
// generally acceptable to change speed with 20% to match gameplay metrics
// when imported the animation is sampled (preferable at the same fps that it was authred at)
// common techniques: curve fitting, quantization+RLE
// https://www.reddit.com/r/GraphicsProgramming/comments/ffhjqt/an_idiots_guide_to_animation_compression/
// https://takinginitiative.net/2020/03/07/an-idiots-guide-to-animation-compression/
// tldr: save animation by track each frame, if position/scale [x, y or z] is the same only serialize a constant
struct AnimationShared
{
	std::vector<std::vector<float>> float_tracks;
	std::vector<std::vector<quat>> rotation_tracks;
};
struct AnimationTrack
{
	template<typename T=float>
	struct Data {bool single; union { T single_value; int index; } data; };
	std::optional<Data> pos_x;
	std::optional<Data> pos_y;
	std::optional<Data> pos_z;
	std::optional<Data<quat>> rotation;
	std::optional<Data> scale_x;
	std::optional<Data> scale_y;
	std::optional<Data> scale_z;
	
	Transform get_transform(const AnimationShared&, int start_index, float scaled_offset);
};

struct Animation
{
	AnimationShared shared;
	std::vector<AnimationTrack> tracks; // per bone
};


// Sampling = read transform of the keyframes we are between and interpolate

// when extracted, root motion is extracted and root is always at (0, 0, 0)
// so we can choose to use anim or gameplay movement when animating
// uncompressed
// extract average velocity, total displacement, ending position and rotation, can be used for animatio nselection
// tip: author with "ground" object to avoid fiddling with chracter root, fix in import with transform relative to root object

// Blending = is roughly interpolation over time
// Two types: Interpolative and Additive
// Interpolative: blend one transform to another: translation+scale = lerp, rotation=slerp (nlerp can cause problems with low fps and helper bones)
// Additive: Authored with a reference pose, stored as offsets that are then additive applied to the "source" pose with some percentage

// sometime you might want to blend things globally

struct Blend_Interpolative
{
    static quat rotation(quat from, quat to, float t) { return slerp(from, to, t); }
    static vec3 translation(vec3 from, vec3 to, float t) { return lerp(from, to, t); }
    static vec3 scale(vec2 from, vec3 to, float t) { return lerp(from, to, t); }
};
struct Blend_Additive
{
    // operator*
    // Concatenate the rotation of this onto rhs and return the result i.e. first rotate by rhs then by this
    // This means order of rotation is right-to-left: child-rotation * parent-rotation
    static quat rotation(quat from, quat to, float t) { return slerp(from, from * to, t); }
    static vec3 translation(vec3 from, vec3 to, float t) { return to*t + from; }
    static vec3 scale(vec2 from, vec3 to, float t) { return to*t + from; }
};

/// extra factors when blending, useful for "layers"
struct BoneMask { std::vector<float> mask; };

template<typename TBlend>
void local_blend(Pose source_pose, Pose target_pose, float blend_weight, BoneMask mask, Pose* result)
{
	assert(is_within(0.0f, weight, 1.0f));
    assert(result != nullptr);
    for(int bone_id: result->bone_count)
    {
        const float weight = calc_bone_weight(blend_weight, mask, bone_id);
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

// extract root motion: essentially just taking 2 positions and getting the delta
// blending root motion: mportant to remember you're working with deltas
// https://takinginitiative.net/2016/07/10/blending-animation-root-motion/
// tldr: use vector slerp

// ===========================================================================
// Bone space: relative to parent bone, "local" in DCC
// Character space: Relative to character root
// World space: Relative to scene origin, "global" in DCC

// Bone mask: weight per bone in the animation skeleton
// tool support: define shoulder 100%, hand 20% and feather bones in between


