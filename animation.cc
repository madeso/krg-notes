namespace std
{
    template <typename T> struct vector{};
    template<typename A, typename B> struct pair {};
    template<typename T> using optional = T*;
    using size_t = unsigned int;
}
template<typename T> bool is_within(T, T, T);
struct mat4{};
struct vec3{};
struct quat{};

vec3 lerp(vec3 from, vec3 to, float t);
quat slerp(quat from, quat to, float t);
vec3 operator*(vec3, float);
vec3 operator+(vec3, vec3);
quat operator*(quat, quat);
void assert(bool);

/*
# Dictionary
* skinning = mesh vertices weighted agains some set of world space transforms ("bones"), all "bones" are in world space, no hierchy/skeleton
* bind pose = position of bones when weighting vertices
* reference/rest pose = default pose when animating
* Pose = set of transformation for all bones in a skeleton
* Animation Pose = Pose using animation skeleton, generally stored in bone space
* Render Pose = Pose using render skeleton, generally converted to character space, and then relative to bind pose for skinning
* Sampling = read transform of the keyframes we are between and interpolate
* Blending = is roughly interpolation over time
* Bone mask = weight per bone in the animation skeleton

// Animation = (roughly) a set of Poses that produce motion

*/
// Animation (not just sampling and blending)
// https://www.youtube.com/watch?v=Jkv0pbp0ckQ

struct Transform { vec3 translation; quat rotation; vec3 scale; };

// todo: add "generate a pose pool" somewhere

// ===========================================================================
// SKINNING

// animation system needs ability to treat "body", facial and cape animations seperate. Body is blended with different animations, face is set once and cape is simulated. Easier to optimize

/// Render Pose. 
struct CompiledPose
{
    /// mesh-root-space to bone-space transform
    std::vector<mat4> transforms;
};

/// contain mesh, material, and per-vertex binding to matrix
struct MeshPart
{
};

/// mesh specific bone for rendering
struct MeshBone
{
};

/** Regular mesh with animation skeleton.
 Multipart character are handled via multiple meshes since they might not reference all core bones
 */
struct Mesh
{
    std::vector<MeshPart> parts;
    std::vector<MeshBone> bones;

	/**
      Add
        - mapping function that take a skellington and maps to actual bones
        - and procedurally calculate other bones
        - procedurally calcualted bones, used for deformation and rendering (depending on the mesh/clothing, the number of bones could change drastically, 10-15 => 25-50)
    */
    std::vector<std::pair<std::size_t, std::size_t>> copy_bones;

    // why seperated? reuse anim, support different characters, multipart character/add clothing support
    // clothing can change bone count
    // torso: tank-top 10-15 bones, jacket 25-50, jacket introduce RBF
    // different render/mesh skeletons based on lod?

    // deformation helper bones:
    // deformation issues is solved with procedurally calculated "deformation helper bones":
    // * roll/twist bones, that take a percent of the parent
    // * pokes
    // * look at
    // a topic of its own https://youtu.be/Jkv0pbp0ckQ?t=958

    // rigid strucutre solvers: 
    // static pieces that needs to be animated, based on a a set of bones, specify transform of other bones
    // * set-driven keys (rotation/and or position linked together... when value 1 is x%, value 2..n also needs to be x%)
    // * Weight driver: RBF (Radial Basis Function) solver
    // * Weight driver: Vector Angle?
};

// gameplay/animation skeleton
// core bones + gameplay bones (shared asset on disk: female01, male01, horse01)
// core "animation skeleton" defined (average male, large female, bipedal mech, horse...)
// animators animate this skeleton using control rig
// helper bones for gameplay could be
// * attachment bones(holsters, weapon bones etc..)
// * anchor bones(anchor character in environment)
// * ik targets/offsets (environment ik)
struct Skellington
{
	// parent data
};


// contain core bones in bone space
struct Pose
{
    /// local transforms
    std::vector<Transform> transforms;
};


// sample source animation at some fps
// interpolate between two keyframes
Animation sample_animation(const SourceAnimation&, float t);


// ===========================================================================
// ANIMATION

// generally acceptable to change speed with 20% to match gameplay metrics
// when imported the animation is sampled (preferable at the same fps that it was authred at)
// common techniques: curve fitting, quantization+RLE
// https://www.reddit.com/r/GraphicsProgramming/comments/ffhjqt/an_idiots_guide_to_animation_compression/
// https://takinginitiative.net/2020/03/07/an-idiots-guide-to-animation-compression/
// tldr: save animation by track each frame, if position/scale [x, y or z] is the same only serialize a constant

struct AnimationData
{
	std::vector<std::vector<float>> float_tracks;
	std::vector<std::vector<quat>> rotation_tracks;
};
struct AnimationTrack
{
    /// Indicateds if this is a single value (only index is valid) or many (values are from index to index+length)
	struct Data { bool single; int index; };

	Data pos_x;
	Data pos_y;
	Data pos_z;
	Data rotation;
	Data scale_x;
	Data scale_y;
	Data scale_z;
	
    /// samples a transform
    /// start_index refer to the index into the fixed array
   /// scaled_offset is [0-1] and how much into the start_index+1 the transform should lerped into
	Transform get_transform(const AnimationData&, int start_index, float scaled_offset);
};

struct Animation
{
	AnimationData data;
	std::vector<AnimationTrack> tracks; // per bone

    // samples a pose
	Pose get_pose(const AnimationData&, int start_index, float scaled_offset);
};


// Two types of blending: Interpolative and Additive
// sometime you might want to blend things globally

/// Interpolative: blend one transform to another: translation+scale = lerp, rotation=slerp (nlerp can cause problems with low fps and helper bones)
struct Blend_Interpolative
{
    static quat rotation(quat from, quat to, float t) { return slerp(from, to, t); }
    static vec3 translation(vec3 from, vec3 to, float t) { return lerp(from, to, t); }
    static vec3 scale(vec3 from, vec3 to, float t) { return lerp(from, to, t); }
};

/// Additive: Authored with a reference pose, stored as offsets that are then additive applied to the "source" pose with some percentage
struct Blend_Additive
{
    // operator*
    // Concatenate the rotation of this onto rhs and return the result i.e. first rotate by rhs then by this
    // This means order of rotation is right-to-left: child-rotation * parent-rotation
    static quat rotation(quat from, quat to, float t) { return slerp(from, from * to, t); }
    static vec3 translation(vec3 from, vec3 to, float t) { return to*t + from; }
    static vec3 scale(vec3 from, vec3 to, float t) { return to*t + from; }
};

/** Provide extra factors when blending.
  This is useful for "layers".  Need tool support
 - define shoulder 100%, hand 20% and feather bones in between
 - this bone and all children 100%
 */
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
// when extracted, root motion is extracted and root is always at (0, 0, 0)
// so we can choose to use anim or gameplay movement when animating

// blending root motion: important to remember you're working with deltas
// https://takinginitiative.net/2016/07/10/blending-animation-root-motion/
// tldr: use vector slerp
// root motion is uncompressed

// tip: author with "ground" object to avoid fiddling with chracter root, fix in import with transform relative to root object
// tip: always manually animate, using a capsule representation... automatic is less smooth and fiddly

// ===========================================================================
// Bone space: relative to parent bone, "local" in DCC
// Character space: Relative to character root
// World space: Relative to scene origin, "global" in DCC



// ===========================================================================
// anim driven: animation drives: select animation, run physics, possible select new animation based on physics (could run into a wall), could be sluggish, animations tweak "gameplay"
// gameplay driven: select animation based on gameplay physics
// tip: start with gameplay driven when designing game since it's easier to tweak


// ===========================================================================
// events and synchronisation
// need a way to annotate or add markup to animation
// 2 types, duration(multi frame) and immediate
// just meta data, need to be a complex type
// when sampling all events during the update are returned
struct EventData
{
    Animation* source_animation;
    void* data;
    float weight; // start out as 1, changed by blending
    float percentage; // position of end point, 0-1, 1 for immediate
    // metadata
    bool ignored; // can for example ignore all events in a layer
    bool trigged_by_leaving_branch; // we are transitioning away from the animaiton that triggered this event
};

// at the end of the frame forward events to consumers
// trigger sfx/vfx, enable show/hide meshes, spawn objects, change damage shape
// consumer can reason about what should be done, multiple foot events -> only triggger highest

// additional usecase: trigger gameplay damage effect, block all transitioning

// ===========================================================================
// synchronization:
// blend walk with jog cycle => need to control animation playback
// solution: mark each anim has a sync track: [left down] [right passing] [right down] [left passing]
// not track => default sync track with 1 event
// use `Least Common Multiple` algorithm to get even number of sync frames and create virtual frames
// length of new animation track is calculated based on blend weight

// can also blend from different animations, when blending you can say start blend between specific "track [entries]", aka start from [track] in src and sync with [another track] in dst

// ===========================================================================
// motion warping
// modify root motion to reach a new endpoint
// modify: example: steer walk cycle, apply rotation to movement
// warp: warp/stetch track to reach endpoint (example: snap to open door/pull lever or climb over thing)

// use events, only allow translation of xy and then only allow translation of z
// use anchor/helper bone to find out what to sync/warp to

// calculatte a warp
// can recalc if target is moving

// can use motion warping to fix motion matching
