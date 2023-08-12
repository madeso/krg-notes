// todo(Gustav): merge to a single system
// todo(Gustav): integrate firewatch ideas & tools https://www.youtube.com/watch?v=wj-2vbiyHnI

#include <vector>
#include <map>
#include <optional>

struct HashedString {};
struct Resource {};

/*
    Dynamic scriptable conversation system

    References:
    Astortian: https://www.youtube.com/watch?v=1LlF5p5Od6A
    Naughty dog / last of us: https://www.youtube.com/watch?v=Y7-OoXqNYgY
    Valve: https://www.youtube.com/watch?v=tAbBID3N64A
*/


///////////////////////////////////////////////////////////////////////////////
// astortian

#ifdef ASTORTIAN
struct Database;
struct Table;
struct Entry;
struct Fact; struct Rule; struct Event;

struct Database
{
    std::vector<Table> tables;
    // examples include: common, tutorial, scene
};


struct Table
{
    std::vector<Fact> facts; // examples include: reset_count, is_near_architect, visits_count
    std::vector<Event> events;
    std::vector<Rule> rules;
};

enum class FactScope { global, area, scene, temporary };
struct Fact
{
    HashedString id;
    int value;
    FactScope scope;
};


struct Event
{
    HashedString id;
    std::vector<HashedString> rules; // rules that this event can trigger
    bool run_once;

    int triggered_count;

    // when triggered:
    //  increment triggered_count
    //  loop through all rules and execute first matching rule
};

enum class CompareOperator { equal, less, greater, not_equal };
struct Criteria
{
    HashedString fact;
    CompareOperator op;
    int value;
};

enum class ModOperator { set, increment };
struct Modification
{
    HashedString fact;
    ModOperator op;
    int value;
};

struct Rule
{
    HashedString id;
    std::vector<Criteria> criterias;

    Resource line;
    int on_finished; // event
    std::vector<Modification> modifications;
};
#endif

///////////////////////////////////////////////////////////////////////////////
// Valve

#ifdef VALVE

struct KeyValue
{
    HashedString key;
    HashedString value; 
};

struct Context // aka Fact
{
    // should this be map or just a sorted vector?
    std::map<HashedString, HashedString> values;

    // collect data when calling say function
    // add context data: // nearest ally, nearest ally distance, current weapon, hitpoints, ammo, current map
    // add local context?
    // add world context?
};

struct ReplyTrigger
{
    HashedString target; // target character, special target `any` references other character in rule
    HashedString what; // use ContextReply and Reply=<what>
};

struct Response
{
    // response is sound, animation, lip-sync
    Resource resource;

    std::vector<ReplyTrigger> triggers;
};

enum class CompareOperator { equal, less, greater, not_equal };
struct Criterion
{
    // if the key is not present in the context, this is considered a fail
    HashedString key;

    CompareOperator op;
    union {HashedString string; int value; float fvalue;} value;
};

// if many rules match, pick the group with the most criterias and randomly select one
struct Rule
{
    // all criteras needs to be true for this rule to be executed
    // boolean values are encoded as 1/0, default value is 0
    // if both criteria and context is sorted, we can iterate both and easily reject missing keys
    std::vector<Criterion> criterias;

    // line to say
    // pick random, walk list sequentially, disable some responses after it has been used
    std::vector<Response> response;

    // when this rule is done, trigger a event/rule for another character
    std::vector<ReplyTrigger> triggers;

    // key values to store/replace into own/local context after rule is executed
    std::vector<KeyValue> remember;
};

struct Database
{
    // sample npc:
    // every 10 seconds say a line for one tagged objects
    // in the npc vision cone
    // using context: ConceptSeeObject and ObjectName=<object>

    // rules could be divided by category, sorted by number of criteria
    std::vector<Rule> rules;
};

// debugging tools:
// print context to console
// log requests
// log source of facts (where did this fact come from)
// Log all matched rules (why did this score highest)
// Log all tested rules (and which criteria passed/failed)
// dump corrent facts on any object (procedural and stored)
// hot swap/edit and continue
/// asset validation
#endif


///////////////////////////////////////////////////////////////////////////////
// naughty dog

#ifndef NAUGHTY_DOG


enum class Priority { death, hit_reaction, in_game_cinematic, ai_chatter, effors };

// logical line/physical line:
// example: out_of_ammo: [I'm out, I need more bullets]
struct DialogLine // (logical line)
{
    struct Character
    {
        HashedString id;
        std::vector<Resource> lines; // (physical line)
        // shuffle all at startup, iterate through, when restarting
        // shuffle again but not so that last is first
        // figure out how to handle weighted propabilities
    };

    HashedString id;
    std::vector<Character> for_character;
    Priority prio;
};

// basic conversation is say and wait-say for characters
// npc ai can `Say` things (battle chatter/grunts) directly

struct Value
{
    union { HashedString hash; int i; float f; } value;
};

struct Entry
{
    HashedString key;
    Value value;
};

// different types of dicts...
// global, per faction, per character, context (per event/request)
struct FactDictionary
{
    std::vector<Entry> entries;
};

// dynamic context:
// regions:
// general and specifc regions, if npc is in the same general region as target, use spefic region
// use region to do specific callouts: "he's in the store", "he's behind the counter"

struct Script
{
};

enum class CompareOperator { equal, less, greater, not_equal };
struct Condition
{
    // if omited searches context then global
    // magic values `speaker`, `listener`, `listener-b`, `listener-c` are references to npc dicts specified in the context 
    std::optional<HashedString> dictionary;
    HashedString key;

    CompareOperator op;
    Value value;

    // function that evaluates to true or false
};

struct Rule
{
    std::optional<int> score; // if none, then score is the number of conditions

    std::optional<HashedString> dialog_line;
    Script on_play; // executed when this rule is played, can set facts in dictionaries

    HashedString next_segment;
    // if npc changes state, abort conversation
    // if conversation is aborted, trigger event
    // if conversation event is sent but no response, trigger "are you okay?" and start search 

    // reject lower prio line even if character isn't speaking
    // abort conversation if higher prio line is requested but character isn't speaking
};

struct ConversationSegment
{
    HashedString id;

    std::vector<Rule> rules;
};

// debugging tools:
// list dictionary content
// say line
// start conversation with context

#endif


///////////////////////////////////////////////////////////////////////////////

