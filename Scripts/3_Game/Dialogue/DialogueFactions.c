#ifdef EXPANSIONMODAI
class DialogueFW_FactionDef
{
	string Name;
	string Loadout;
	string PlayerStance;
	ref TStringArray FriendlyFactions;

	//! Where this faction's standing with a player is kept.
	//!
	//! It is an ordinary variable -- the same kind a conversation button or a
	//! quest hand-in already changes -- which is what lets one deed move
	//! several factions at once: a quest that pleases the militia and annoys
	//! everyone they fight is four ops on the one list, not a special case.
	//! Empty means this faction keeps no standing and nothing below applies.
	string ReputationVar = "";

	//! 1 = they turn on a player whose standing with them has fallen far
	//! enough. Off by default on purpose: most standings start at 0, so a
	//! faction that merely keeps score would otherwise open fire on sight.
	int HostileWhenLow = 0;

	//! The point it has to fall to. At or below this the faction stops
	//! counting the player as one of their own; back above it they calm down.
	int HostileBelow = 0;

	void DialogueFW_FactionDef()
	{
		FriendlyFactions = new TStringArray;
	}

	void Sanitize()
	{
		if (!FriendlyFactions)
			FriendlyFactions = new TStringArray;
		PlayerStance.ToUpper();
		if (PlayerStance != "FRIENDLY" && PlayerStance != "GUARD" && PlayerStance != "HOSTILE")
			PlayerStance = "FRIENDLY";

		if (HostileWhenLow != 0)
			HostileWhenLow = 1;
		if (ReputationVar == "")
			HostileWhenLow = 0;
	}

	bool WatchesStanding()
	{
		return ReputationVar != "" && HostileWhenLow == 1;
	}
}

class DialogueFW_FactionFile
{
	ref array<ref DialogueFW_FactionDef> Factions;

	void DialogueFW_FactionFile()
	{
		Factions = new array<ref DialogueFW_FactionDef>;
	}
}

class DialogueFW_FactionRegistry
{
	static const int MAX_SLOTS = 32;
	static const string SLOT_PREFIX = "DialogueFW";
	static const string CONFIG_FILE = "$profile:\\DialogFramework\\Factions\\Factions.json";

	static ref array<ref DialogueFW_FactionDef> s_Defs;
	static ref map<string, int> s_NameToSlot;
	static bool s_AnyStanding;

	static string Lower(string value)
	{
		string copy = value;
		copy.ToLower();
		return copy;
	}

	static void Load()
	{
		s_Defs = new array<ref DialogueFW_FactionDef>;
		s_NameToSlot = new map<string, int>;
		s_AnyStanding = false;

		if (!FileExist(CONFIG_FILE))
		{
			Print("[DialogueFramework] [Factions] No Factions.json -- built-in Expansion factions only.");
			return;
		}

		DialogueFW_FactionFile file = new DialogueFW_FactionFile();
		JsonFileLoader<DialogueFW_FactionFile>.JsonLoadFile(CONFIG_FILE, file);

		if (!file || !file.Factions)
		{
			Print("[DialogueFramework] [Factions] Factions.json has no Factions array (or failed to parse).");
			return;
		}

		int count = 0;
		foreach (DialogueFW_FactionDef def : file.Factions)
		{
			if (!def)
				continue;

			def.Sanitize();
			if (def.Name == "")
				continue;

			if (count >= MAX_SLOTS)
			{
				Print("[DialogueFramework] [Factions] More than " + MAX_SLOTS + " factions defined -- '" + def.Name + "' and any after it are ignored.");
				break;
			}

			s_Defs.Insert(def);
			s_NameToSlot.Set(Lower(def.Name), count);
			count++;
		}

		Print("[DialogueFramework] [Factions] Loaded " + count + " custom faction(s).");

		//! Named one by one: a faction that turns hostile on its own is the
		//! kind of thing a server owner wants to see confirmed at startup
		//! rather than discover from a player being shot at.
		foreach (DialogueFW_FactionDef watcher : s_Defs)
		{
			if (!watcher || watcher.ReputationVar == "")
				continue;

			if (watcher.WatchesStanding())
			{
				s_AnyStanding = true;
				Print("[DialogueFramework] [Factions] '" + watcher.Name + "' keeps standing in '" + watcher.ReputationVar + "' and turns hostile at " + watcher.HostileBelow + " or below.");
			}
			else
			{
				Print("[DialogueFramework] [Factions] '" + watcher.Name + "' keeps standing in '" + watcher.ReputationVar + "' but never turns hostile over it.");
			}
		}
	}

	static DialogueFW_FactionDef GetDef(int slot)
	{
		if (!s_Defs)
			return null;
		if (slot < 0 || slot >= s_Defs.Count())
			return null;
		return s_Defs[slot];
	}

	//! Whether any faction at all watches a standing. Worked out once at load
	//! rather than on the spot: every AI asks it on every frame before doing
	//! anything heavier, so a server that never set one up pays one bool for
	//! the whole feature.
	static bool AnyWatchesStanding()
	{
		return s_AnyStanding;
	}

	static int SlotForName(string name)
	{
		if (!s_NameToSlot)
			return -1;
		string key = Lower(name);
		if (s_NameToSlot.Contains(key))
			return s_NameToSlot.Get(key);
		return -1;
	}

	static string SlotClassSuffix(int slot)
	{
		return SLOT_PREFIX + slot.ToString();
	}

	static bool IsFriendlyName(DialogueFW_FactionDef def, string otherName)
	{
		if (!def || !def.FriendlyFactions)
			return false;
		string key = Lower(otherName);
		foreach (string entry : def.FriendlyFactions)
		{
			if (Lower(entry) == key)
				return true;
		}
		return false;
	}
}
#endif
