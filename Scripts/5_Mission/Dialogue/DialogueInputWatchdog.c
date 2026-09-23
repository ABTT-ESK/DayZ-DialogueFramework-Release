//! The conversation window takes the player's inputs while it is up and gives
//! them back when it hides. If a window ever goes away without that happening
//! -- the player dies, another menu replaces it, the engine tears it down --
//! the player is left with nothing: no movement, no menu key, no way out but
//! killing the game. A player reported exactly that on 1.5.0.
//!
//! So the check runs from the mission instead of from the window: it outlives
//! any window, and puts the player right on the next frame.
modded class MissionGameplay
{
	//! Ours runs BEFORE super, not after. Every mod's OnUpdate is one chain,
	//! and a script error anywhere in it takes the rest of the chain with it.
	//! A mod on the user's own test server throws roughly a thousand times a
	//! session from its own OnUpdate (2026-09-22, "Function 'SetTransform' not
	//! linked"), and while it does, nothing after super.OnUpdate runs -- which
	//! is exactly when the player is left with no controls and no HUD and this
	//! check is the only thing that would give them back.
	//!
	//! Running first costs nothing: the check only reads state the window set
	//! last frame.
	override void OnUpdate(float timeslice)
	{
		DialogueWindowMenu.DialogueFW_WatchInputLock(timeslice);

		super.OnUpdate(timeslice);
	}
}
