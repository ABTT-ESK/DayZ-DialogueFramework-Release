#ifdef EXPANSIONMODAI
class DialogueFW_FactionDef
{
	string Name;
	string Loadout;
	string PlayerStance;
	ref TStringArray FriendlyFactions;

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
	}

	static DialogueFW_FactionDef GetDef(int slot)
	{
		if (!s_Defs)
			return null;
		if (slot < 0 || slot >= s_Defs.Count())
			return null;
		return s_Defs[slot];
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
