#ifdef EXPANSIONMODAI
//! Where a player stands with a whole faction, and what falls out of it.
//!
//! The standing itself is nothing new: it is an ordinary variable, changed by
//! the same ops a conversation button or a quest hand-in has always used. That
//! is deliberate -- it means one deed can move several factions at once, in
//! whatever direction each of them would actually feel about it, with no
//! mechanism of its own to learn.
//!
//! What is new is that a faction can read one. Fall to the point the server
//! owner picked and that faction's AI stop counting you as one of their own;
//! climb back above it and they calm down again, including forgetting a
//! grudge that would otherwise have been permanent. A player can always work
//! their way back in.
//!
//! Server only. Standings live in the player's state file, which only the
//! server has, and only the server decides who the AI shoot at.
class DialogueFW_FactionStanding
{
	//! Must match the key DialogueAggro writes its per-faction counter under.
	static const string AGGRO_PREFIX = "__aggro_f_";

	//! How long a faction's AI have to notice that a player has been forgiven
	//! and let go of them. They check every couple of seconds, so this only
	//! has to outlast one round of checks -- and it deliberately does not
	//! outlast much more than that. Dropping a target has to be tied to the
	//! moment the standing crossed back, not to the standing being fine:
	//! somebody in good standing who opens fire is a target the AI is
	//! entitled to keep.
	static const int FORGIVE_WINDOW_MS = 15000;

	//! The standings as they were before the current batch of ops, one entry
	//! per faction in registry order. Remember and SettleCrossings are called
	//! either side of a single synchronous Apply, so one snapshot is enough.
	protected static ref array<int> s_Before;

	//! "uid|faction" -> the moment the window on it closes.
	protected static ref map<string, int> s_Forgiven;

	static bool IsHostile(string uid, DialogueFW_FactionDef def)
	{
		if (!def || !def.WatchesStanding())
			return false;
		if (uid == "" || !GetGame() || !GetGame().IsServer())
			return false;

		return Standing(uid, def) <= def.HostileBelow;
	}

	//! Noted the moment a standing climbs back above the line, and read by
	//! the faction's AI on their next check so they drop the player.
	protected static void Forgive(string uid, string factionName)
	{
		if (!s_Forgiven)
			s_Forgiven = new map<string, int>;

		s_Forgiven.Set(uid + "|" + factionName, GetGame().GetTime() + FORGIVE_WINDOW_MS);
	}

	static bool JustForgiven(string uid, string factionName)
	{
		if (!s_Forgiven || uid == "" || factionName == "")
			return false;

		string key = uid + "|" + factionName;
		int expiry;
		if (!s_Forgiven.Find(key, expiry))
			return false;

		if (GetGame().GetTime() > expiry)
		{
			s_Forgiven.Remove(key);
			return false;
		}

		return true;
	}

	protected static int Standing(string uid, DialogueFW_FactionDef def)
	{
		DialoguePlayerState state = DialogueVars.GetInstance().GetServerState(uid);
		if (!state)
			return 0;

		return state.Get(def.ReputationVar);
	}

	//! Snapshot, taken before ops are applied.
	static void Remember(string uid, DialoguePlayerState state)
	{
		s_Before = null;

		if (!state || !DialogueFW_FactionRegistry.AnyWatchesStanding())
			return;

		s_Before = new array<int>;

		for (int i = 0; i < DialogueFW_FactionRegistry.MAX_SLOTS; i++)
		{
			DialogueFW_FactionDef def = DialogueFW_FactionRegistry.GetDef(i);
			if (!def)
				break;

			if (def.WatchesStanding())
				s_Before.Insert(state.Get(def.ReputationVar));
			else
				s_Before.Insert(0);
		}
	}

	//! Compared after. Only a standing that actually crossed the line is acted
	//! on, so a quest that leaves a faction's number alone never disturbs a
	//! grudge that faction is entitled to hold.
	static void SettleCrossings(string uid, DialoguePlayerState state)
	{
		if (!s_Before || !state)
			return;

		for (int i = 0; i < s_Before.Count(); i++)
		{
			DialogueFW_FactionDef def = DialogueFW_FactionRegistry.GetDef(i);
			if (!def || !def.WatchesStanding())
				continue;

			int before = s_Before[i];
			int after = state.Get(def.ReputationVar);

			bool wasHostile = before <= def.HostileBelow;
			bool nowHostile = after <= def.HostileBelow;

			if (wasHostile == nowHostile)
				continue;

			if (nowHostile)
			{
				Print("[DialogueFramework] [Factions] " + uid + " has fallen to " + after + " with '" + def.Name + "' (hostile at " + def.HostileBelow + " or below) -- they will be treated as an enemy.");
				Tell(uid, def.Name, "STR_DIALOGUEFW_FACTION_HOSTILE", true);
				continue;
			}

			//! Back above the line. Clearing the faction's aggro counter is
			//! the whole point: without it a player who had been fought
			//! often enough to earn a permanent grudge could never buy their
			//! way back in, whatever they did for the faction afterwards.
			string aggroKey = AGGRO_PREFIX + def.Name;
			if (state.Get(aggroKey) != 0)
				state.Set(aggroKey, 0);

			Forgive(uid, def.Name);

			Print("[DialogueFramework] [Factions] " + uid + " is back to " + after + " with '" + def.Name + "' -- they calm down.");
			Tell(uid, def.Name, "STR_DIALOGUEFW_FACTION_CALM", false);
		}

		s_Before = null;
	}

	//! Told to the player, because nothing else would. The window's own
	//! reputation toast is raised by the button that was pressed, so a quest
	//! handed in on Expansion's screen changes standings in silence -- and
	//! being shot at with no idea why is not a mechanic.
	//!
	//! The faction's name is the title and is left exactly as the owner wrote
	//! it: CF_Localiser only translates text that starts with STR_, so a name
	//! passes through and the line under it still arrives in the player's own
	//! language.
	protected static void Tell(string uid, string factionName, string bodyKey, bool bad)
	{
		DialogueManager manager = DialogueManager.GetInstance();
		if (manager)
		{
			DialogueMenuConfig config = manager.GetMenuConfig();
			if (config && !config.ShowReputationNotifications)
				return;
		}

		PlayerBase player = PlayerBase.GetPlayerByUID(uid);
		if (!player || !player.GetIdentity())
			return;

		if (bad)
			ExpansionNotification(factionName, bodyKey).Error(player.GetIdentity());
		else
			ExpansionNotification(factionName, bodyKey).Success(player.GetIdentity());
	}
}
#endif
