// this is based on a lost scripting system based on xml I made a long time ago
// it is also expanded with ideas (states, events and multi tasking) from naught dog: https://www.gdcvault.com/play/1730/State-Based-Scripting-in-UNCHARTED
// this has notes about optimizing/a better implementation than the notes below
// todo(Gustav): look into half life alyx scripting: https://twitter.com/ImplicitAction/status/1355319716517007361

/*
Where used:
Gameplay scripting system

*/

#include <string>
#include <unordered_map>
#include <vector>
#include <functional>

struct HashedString {};
struct Value {};

/** A function that can be executed over a number of frames.
Greedily executes commands until a update returns "false"
*/
struct Command
{
    virtual ~Command() {}
    virtual bool update(float dt) = 0;
};

/** Support running 2 commands at the same time with basic synchronization
Example with player and friend shaking hands
```xml
<State name="shake-hands">
    <On event="begin">
        <TrackList>
            <Track>
                <WaitMoveTo actor="player" to="waypoint" />
                <Signal signal="player-at-waypoint" />
                <WaitForSignal signal="friend-at-waypoint" />
                <WaitAnimate actor="player" animation="shake-friend-hand" />
            </Track>
            <Track>
                <WaitMoveTo actor="friend" to="waypoint" />
                <Signal signal="friend-at-waypoint" />
                <WaitForSignal signal="player-at-waypoint" />
                <WaitAnimate actor="friend" animation="shake-player-hand" />
            </Track>
        </TrackList>
    </On>
</State>
```
*/
struct TrackList : Command
{
};

// converts a <WaitForSeconds Time="1000"/> to a Command
using CommandParser = std::function<Command* (const std::unordered_map<std::string, Value>&)>;
using CommandMap = std::unordered_map<std::string, CommandParser>;

// events: could be a regular event, a update event, or enter/leave area event or implicit begin/end state event
struct EventHandler
{
    std::vector<Command*> commands;
};

struct State
{
    std::unordered_map<std::string, EventHandler> on_event;
};

/** Represents a script
 * Scripts can be assigned to a
 * object
 * Trigger Volume: with enter, exit and ocupancy events
 * stand alone/world as a "director"
 
## Example: Kickable gate
```xml
<Script name="kickable-gate" initial_state="locked">
    <Variables>
        <int32 name="num-attempts" />
        <bool name="is-locked" value="true" />
    </Variables>
    <!-- properties can be configured in the editor -->
    <Properties>
        <Property name="kick-anim" default="gate-kick" />
    </Properties>
    <State name="locked">
        <On event="kick">
            <Go state="kicked" />
        </On>
        <On event="begin">
            <Print string="Starting idle" />
            <Animate target="self" anim="locked-idle" />
        </On>
        <On event="update">
            ...
        </On>
    </State>
    <State name="kicked">
        <On state="begin">
            <If condition="is_locked">
                <SetInt32 var="num-attempts">
                    <Add>
                        <Constant value="1">
                        <GetInt32 var="num-attempts" />
                    </Add>
                </SetInt32>
                <WaitAnimate target="self" anim="kick-failure" />
                <Go state="locked" />
            </If>

            <Note value="else..."/> <!-- Note commands are stripped but visible in gui editor -->
            <WaitAnimate taget="self" anim="kick-success" />
            <Go state="open" />
        </On>
    </State>
</Script>
```

## Example: Falling sign
```xml
<Script name="falling-sign">
    <State name="untouched">
        <On event="update">
            <If>
                <If.condition>
                    <TaskComplete task="wz-post-combat"/>
                </If.condition>
                <Go state="fallen"/>
            </If>
        </On>
       <On event="hanging-from">
            <Go state="breaking" />
        </On>
    </State>
    <State name="breaking">
        <On event="begin">
            <SpawnParticlesAtJoint target="self"
                joint="hinge"
                particles="sign-break-dust"/>
            <WaitAnimate target="self" anim="sign-break" />
            <Go state="fallen" />
       </On>
    </State>
    <State name="fallen">
        <On event="begin">
            <Note text="looping" />
            <Animate target="self" anim="sign-broken" />
        </On>
    </State>
</Script>
```

## Example: Falling sign (generic)
```xml
<Script name="simple-animating-obj">
    <Properties>
        <PropertyString name="done-task" />
        <PropertyString name="particle-joint" />
        <PropertyString name="particle-name" />
        <PropertyString name="anim-name" />
        <PropertyString name="done-anim-name" />
    </Properties>
    <State name="initial">
        <On event="update">
            <If>
                <If.Condition>
                    <TaskComplete>
                        <TaskCompletion.task>
                            <PropertyString name="done-task" />
                        </TaskCompletion.task>
                    </TaskComplete>
                </If.Condition>
                <Go state="done" />
            </If>
        </On>
       <On event="hanging-from">
            <Go state="animating" />
        </On>
    </State>
    <State name="animating">
        <On event="begin">
            <SpawnParticlesAtJoint target="self">
                <SpawnParticlesAtJoint.joint>
                    <PropertyString name="particle-joint" />
                </SpawnParticlesAtJoint.joint>
                <SpawnParticlesAtJoint.particles>
                    <PropertyString name="particle-name" />
                </SpawnParticlesAtJoint.particles>
            </SpawnParticlesAtJoint>
            <WaitAnimate taget="self">
                <WaitAnimate.anim>
                    <PropertyString name="anim-name" />
                </WaitAnimate.anim>
            </WaitAnimate>
            <Go state="done" />
       </On>
    </State>
    <State name="done">
        <On event="begin">
            <Animate taget="self">
                <Animate.anim>
                    <PropertyString name="done-anim-name" />
                </Animate.anim>
            </Animate>
        </On>
    </State>
</Script>
```
## Example: in game cinematics (should this be suppoted?)
```
<Script name="wz-bus-crash">
    <State name="spawn-soldiers">
       <On event="begin">
            <PlayerDisableControls controls="all-but-right-stick" />
            <SpawnNpcInCombat npc="npc-wz-52" />
            <SpawnNpcInCombat npc="npc-wz-53" />
            ...
            <Go state="crash" />
       </On>
    </State>
    <State name="crash">
        <On event="begin">
            <TrackList>
                <Track name="bus">
                    <WaitAnimate target="bus-1" anim="bus-crash">
                        <WaitAnimate.location>
                            <GetLocator name="ref-bus-crash-1" />
                        </WaitAnimate.location>
                    </WaitAnimate>
                    <Signal signal="bus-done">
                </Track>
                <Track name="player">
                    <Animate target="player" anim="player-watch-crash">
                        <Animate.location>
                            <GetLocator name="ref-bus-crash-1" />
                        </Animate.location>
                    </Animate>
                    <WaitUntil frame="250"/>
                    <Say "player" "vox-wz-drk-01-what-the" />
                    <Signal signal="player-done" />
                </Track>
            </TrackList>
            ...
        </On>
    </State>
</Script>
```

# Perhaps switching to a more sexpr like structure...?

## Example: Falling sign
```lisp
(script falling-sign
    (state untouched
        (on update
            (when (task-complete wz-post-combat)
                (go fallen)
            )
        )
       (on hanging-from
            (go breaking)
        )
    )
    (state breaking
        (on begin
            (spawn-particles-at-joint self hinge sign-break-dust)
            (wait-animate self sign-break)
            (go fallen)
       )
    )
    (state fallen
        (on begin
            (animate self sign-broken) ;; looping
        )
    )
)
```

## Example: Falling sign (generic)
```lisp
(script simple-animating-obj
    (properties
        (prop-string done-task)
        (prop-string particle-joint)
        (prop-string particle-name)
        (prop-string anim-name))
        (prop-string done-anim-name)
    )
    (state initial
        (on update
            (when (task-complete
                (prop-string done-task))
                (go done)
            )
        )
       (on hanging-from
            (go animating)
        )
    )
    (state animating
        (on begin
            (spawn-particles-at-joint self
                (prop-string particle-joint)
                (prop-string particle-name))
            (wait-animate self
                (prop-string anim-name))
            (go done)
       )
    )
    (state done
        (on begin
            (animate self
                (prop-string done-anim-name))
        )
    )
)
```

## Example: in game cinematics (should this be suppoted?)
```lisp
(script wz-bus-crash
    (state spawn-soldiers
        (on begin
            (player-disable-controls controls:all-but-right-stick)
            (spawn-npc-in-combat npc-wz-52)
            (spawn-npc-in-combat npc-wz-53)
            ...
            (go crash)
       )
    )
    (state crash
        (on begin
            (track-list
                (track bus
                    (wait-animate bus-1 bus-crash
                        (get-locator ref-bus-crash-1))
                    (signal bus-done)
                )
                (track player
                    (animate player player-watch-crash
                        (get-locator ref-bus-crash-1))
                    (wait-until-frame 250)
                    (say player vox-wz-drk-01-what-the)
                    (signal player-done)
                )
            )
        )
    )
)
```


## Example: Falling sign (json)
```json
[script falling-sign
    [state untouched
        [on update
            [when [task-complete {name: wz-post-combat}]
                [go fallen]
            )
        [
        [on hanging-from
            [go breaking]
        ]
    ]
    [state breaking
        [on begin
            [spawn-particles-at-joint self hinge sign-break-dust]
            [wait-animate self sign-break]
            [go fallen]
       ]
    ]
    [state fallen
        [on begin
            [animate self sign-broken] // looping
        ]
    ]
]
```



*/
struct Script
{
    std::unordered_map<std::string, Value> variables;
    std::vector<State> states;
    int current_state = 0;
};

