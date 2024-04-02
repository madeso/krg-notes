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
// * RBF solver

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
