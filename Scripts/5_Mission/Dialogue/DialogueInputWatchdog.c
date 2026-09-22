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
	override void OnUpdate(float timeslice)
	{
		super.OnUpdate(timeslice);

		DialogueWindowMenu.DialogueFW_WatchInputLock(timeslice);
	}
}
