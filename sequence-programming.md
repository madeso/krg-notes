# Sequence programming

## custom (scripting) language
[Retro Game Internals - Punch-Out Behavior Script Visualization](https://www.youtube.com/watch?v=_Q9_7vSLndQ)

## yield return coroutine
> It's just a coroutine where you wait on the conditions to go to the next state with yields
> The one thing coroutines break down with is non-trivial control flow and needing to react to external events (like collisions and such) If the logic of the entity is dictated by those, I pull out a full explicit statemachine to do the logic The former can sometimes be managed with nested coroutines if they make sense, but the latter is an inherent weakness

[AnodyneSharp/Mitra.cs at master · PixieCatSupreme/AnodyneSharp · GitHub](https://github.com/PixieCatSupreme/AnodyneSharp/blob/master/AnodyneSharp/AnodyneSharp.Shared/Entities/Interactive/Npc/Mitra.cs#L131)
[AnodyneSharp/WallBoss.cs at master · PixieCatSupreme/AnodyneSharp · GitHub](https://github.com/PixieCatSupreme/AnodyneSharp/blob/master/AnodyneSharp/AnodyneSharp.Shared/Entities/Enemy/Crowd/WallBoss.cs)

```csharp
IEnumerator<CutsceneEvent> Exit()
{
	yield return new DialogueEvent(DialogueManager.GetDialogue("mitra", "initial_overworld", 3));
	//back to bike
	velocity = Vector2.UnitX * 20;
	while ((Position - bike.Position).Length() > 8)
		yield return null;
	OnBike();
	bike.exists = false;
	//go off-screen
	velocity = Vector2.UnitY * 50;
	Vector2 UL = MapUtilities.GetRoomUpperLeftPos(GlobalState.CurrentMapGrid);
	while ((Position - UL).Y < 190)
		yield return null;
	volume.SetTarget(0.2f);
	while (!volume.ReachedTarget)
		yield return null;
	SoundManager.PlaySong("overworld");
	exists = false;
	yield break;
}
```

```csharp
IEnumerator fight_logic = FightLogic();
while (face.Health > 0)
{
	fight_logic.MoveNext();
	yield return null;
}
```

## Lua Coroutines
[Automating Sequences via Lua Coroutines in C++](https://www.youtube.com/watch?v=E42Lyv2Ra1c)

## Behaviour tree script
General: [Behavior trees for AI: How they work](https://www.gamedeveloper.com/programming/behavior-trees-for-ai-how-they-work)
Thin matrix: [Programming Gymnastics and Restaurants! - YouTube](https://www.youtube.com/watch?v=VtdhP8sZCt0&list=PLRIWtICgwaX2_2R9UcpGo1_vw050fZbTY)

* root: at the top
* selector: execute one child
* sequence: execute children in sequence
* leaf: performs actions

```java
protected void initSequence(List<Action> steps) {
	steps.add(new GotoWait(reception, 2));
	steps.add(new Wait(3.0f));
	steps.add(new GotoDo(emtpy_seat, new EatingAnimation(mover)));
	steps.add(new Goto(entrance_node));
}

class Wait implements Action {
	public void update();
	public boolean isFinished();
	public void init();
	public void finish();
}
// TimedAction implements action for a time
//   essentially setting the yaw, pitch, roll for a local transformation from a few easing animations
// LoopAction performs a action N times
```

