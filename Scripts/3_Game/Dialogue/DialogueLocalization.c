class DialogueFWLanguages
{
	static const string ENGLISH = "english";

	//! Built at declaration, not lazily. A lazy `if (!s_All) s_All = new ...`
	//! threw "NULL pointer to instance" when this was first touched from
	//! MissionServer.OnInit, which is early enough that the assignment did not
	//! stick. Expansion declares its statics this way for the same reason.
	protected static ref array<string> s_All = new array<string>;

	static array<string> All()
	{
		if (s_All.Count() == 0)
		{
			s_All.Insert("english");
			s_All.Insert("czech");
			s_All.Insert("german");
			s_All.Insert("russian");
			s_All.Insert("polish");
			s_All.Insert("hungarian");
			s_All.Insert("italian");
			s_All.Insert("spanish");
			s_All.Insert("french");
			s_All.Insert("chinese");
			s_All.Insert("japanese");
			s_All.Insert("portuguese");
			s_All.Insert("chinesesimp");
		}

		return s_All;
	}

	static string Normalize(string language)
	{
		string lowered = language;
		lowered.ToLower();
		return lowered;
	}

	static bool IsKnown(string language)
	{
		return All().Find(Normalize(language)) != -1;
	}

	//! Plain-ASCII names, always correct and always distinct. The stringtable
	//! is asked first so a player sees their language written the way they
	//! write it, but two languages came back with the SAME name in game once,
	//! which made the picker unusable -- so anything ambiguous falls back here.
	static string PlainName(string language)
	{
		string key = Normalize(language);

		if (key == "english") return "English";
		if (key == "czech") return "Czech";
		if (key == "german") return "German";
		if (key == "russian") return "Russian";
		if (key == "polish") return "Polish";
		if (key == "hungarian") return "Hungarian";
		if (key == "italian") return "Italian";
		if (key == "spanish") return "Spanish";
		if (key == "french") return "French";
		if (key == "chinese") return "Chinese (Traditional)";
		if (key == "japanese") return "Japanese";
		if (key == "portuguese") return "Portuguese";
		if (key == "chinesesimp") return "Chinese (Simplified)";

		return key;
	}

	static string DisplayName(string language)
	{
		string key = Normalize(language);
		if (key == "")
			return "";

		string upper = key;
		upper.ToUpper();

		string translated = Widget.TranslateString("#STR_DIALOGUEFW_LANGNAME_" + upper);

		string plain = PlainName(key);
		if (translated == "" || translated.IndexOf("STR_DIALOGUEFW_LANGNAME_") != -1)
			return plain;

		//! Say what came back, so a wrong lookup is visible in the log rather
		//! than only as two identical rows in the picker.
		Print("[DialogueFramework] [LOC] Language name for '" + key + "' resolved to '" + translated + "'.");

		return translated;
	}
}

class DialogueLocKeys
{
	static string TreeList(string field, int index)
	{
		return "tree." + field + "." + index;
	}

	static string TreeSingle(string field)
	{
		return "tree." + field;
	}

	static string NodeSpeaker(int stageIndex, int nodeID)
	{
		return StagePrefix(stageIndex) + "node." + nodeID + ".SpeakerText";
	}

	static string NodeSpeakerLine(int stageIndex, int nodeID, int lineIndex)
	{
		return StagePrefix(stageIndex) + "node." + nodeID + ".SpeakerLines." + lineIndex;
	}

	static string NodeResponse(int stageIndex, int nodeID, int responseIndex)
	{
		return StagePrefix(stageIndex) + "node." + nodeID + ".Responses." + responseIndex;
	}

	static string QuestList(string field, int index)
	{
		return "quest." + field + "." + index;
	}

	static string QuestSingle(string field)
	{
		return "quest." + field;
	}

	protected static string StagePrefix(int stageIndex)
	{
		if (stageIndex < 0)
			return "";

		return "stage." + stageIndex + ".";
	}
}

class DialogueLocEntry
{
	string Key;
	string Text;

	void OnSend(ScriptRPC rpc)
	{
		rpc.Write(Key);
		rpc.Write(Text);
	}

	bool OnRecieve(ParamsReadContext ctx)
	{
		if (!ctx.Read(Key)) return false;
		if (!ctx.Read(Text)) return false;
		return true;
	}
}

class DialogueLocEntryList
{
	static void Write(ScriptRPC rpc, array<ref DialogueLocEntry> entries)
	{
		rpc.Write(entries.Count());
		foreach (DialogueLocEntry entry : entries)
			entry.OnSend(rpc);
	}

	static bool Read(ParamsReadContext ctx, array<ref DialogueLocEntry> target)
	{
		int count;
		if (!ctx.Read(count)) return false;

		target.Clear();
		for (int i = 0; i < count; i++)
		{
			DialogueLocEntry entry = new DialogueLocEntry();
			if (!entry.OnRecieve(ctx)) return false;
			target.Insert(entry);
		}
		return true;
	}
}

class DialogueLocTree
{
	int TreeID = 0;
	string TreeFile = "";

	ref array<ref DialogueLocEntry> Entries;

	void DialogueLocTree()
	{
		Entries = new array<ref DialogueLocEntry>;
	}

	void Sanitize()
	{
		if (!Entries)
			Entries = new array<ref DialogueLocEntry>;

		TreeFile = DialogueLocPath.Normalize(TreeFile);
	}

	void OnSend(ScriptRPC rpc)
	{
		rpc.Write(TreeID);
		rpc.Write(TreeFile);
		DialogueLocEntryList.Write(rpc, Entries);
	}

	bool OnRecieve(ParamsReadContext ctx)
	{
		if (!ctx.Read(TreeID)) return false;
		if (!ctx.Read(TreeFile)) return false;
		if (!DialogueLocEntryList.Read(ctx, Entries)) return false;
		return true;
	}
}

class DialogueLocQuest
{
	int QuestID = -1;

	ref array<ref DialogueLocEntry> Entries;

	void DialogueLocQuest()
	{
		Entries = new array<ref DialogueLocEntry>;
	}

	void Sanitize()
	{
		if (!Entries)
			Entries = new array<ref DialogueLocEntry>;
	}

	void OnSend(ScriptRPC rpc)
	{
		rpc.Write(QuestID);
		DialogueLocEntryList.Write(rpc, Entries);
	}

	bool OnRecieve(ParamsReadContext ctx)
	{
		if (!ctx.Read(QuestID)) return false;
		if (!DialogueLocEntryList.Read(ctx, Entries)) return false;
		return true;
	}
}

class DialogueLocPath
{
	static string Normalize(string path)
	{
		string result = path;
		result.Replace("\\", "/");
		result.ToLower();

		while (result.IndexOf("/") == 0)
			result = result.Substring(1, result.Length() - 1);

		return result;
	}
}

class DialogueLocFile
{
	static const int CURRENT_VERSION = 1;

	int ConfigVersion = 0;
	string Language = "";

	ref array<ref DialogueLocTree> Trees;
	ref array<ref DialogueLocQuest> Quests;

	void DialogueLocFile()
	{
		ConfigVersion = CURRENT_VERSION;
		Trees = new array<ref DialogueLocTree>;
		Quests = new array<ref DialogueLocQuest>;
	}

	void Sanitize()
	{
		if (!Trees)
			Trees = new array<ref DialogueLocTree>;

		foreach (DialogueLocTree tree : Trees)
		{
			if (tree)
				tree.Sanitize();
		}

		if (!Quests)
			Quests = new array<ref DialogueLocQuest>;

		foreach (DialogueLocQuest quest : Quests)
		{
			if (quest)
				quest.Sanitize();
		}

		Language = DialogueFWLanguages.Normalize(Language);
	}
}

class DialogueLocBundle
{
	string Language = "";

	ref array<ref DialogueLocTree> Trees;
	ref array<ref DialogueLocQuest> Quests;

	protected ref map<string, string> m_Lookup;

	void DialogueLocBundle()
	{
		Trees = new array<ref DialogueLocTree>;
		Quests = new array<ref DialogueLocQuest>;
	}

	int EntryCount()
	{
		int total = 0;

		foreach (DialogueLocTree tree : Trees)
		{
			if (tree && tree.Entries)
				total += tree.Entries.Count();
		}

		foreach (DialogueLocQuest quest : Quests)
		{
			if (quest && quest.Entries)
				total += quest.Entries.Count();
		}

		return total;
	}

	void Merge(DialogueLocFile file)
	{
		if (!file)
			return;

		foreach (DialogueLocTree tree : file.Trees)
		{
			if (tree)
				Trees.Insert(tree);
		}

		foreach (DialogueLocQuest quest : file.Quests)
		{
			if (quest)
				Quests.Insert(quest);
		}

		m_Lookup = null;
	}

	void BuildLookup()
	{
		m_Lookup = new map<string, string>;

		foreach (DialogueLocTree tree : Trees)
		{
			if (!tree || !tree.Entries)
				continue;

			foreach (DialogueLocEntry treeEntry : tree.Entries)
			{
				if (!treeEntry || treeEntry.Key == "" || treeEntry.Text == "")
					continue;

				if (tree.TreeFile != "")
					m_Lookup.Set("F|" + tree.TreeFile + "|" + treeEntry.Key, treeEntry.Text);

				if (tree.TreeID > 0)
				{
					string idScoped = "I|" + tree.TreeID + "|" + treeEntry.Key;
					if (!m_Lookup.Contains(idScoped))
						m_Lookup.Set(idScoped, treeEntry.Text);
				}
			}
		}

		foreach (DialogueLocQuest quest : Quests)
		{
			if (!quest || quest.QuestID <= 0 || !quest.Entries)
				continue;

			foreach (DialogueLocEntry questEntry : quest.Entries)
			{
				if (!questEntry || questEntry.Key == "" || questEntry.Text == "")
					continue;

				m_Lookup.Set("Q|" + quest.QuestID + "|" + questEntry.Key, questEntry.Text);
			}
		}
	}

	string Find(string scopedKey)
	{
		if (!m_Lookup)
			BuildLookup();

		string found;
		if (m_Lookup.Find(scopedKey, found))
			return found;

		return "";
	}

	void OnSend(ScriptRPC rpc)
	{
		rpc.Write(Language);

		rpc.Write(Trees.Count());
		foreach (DialogueLocTree tree : Trees)
			tree.OnSend(rpc);

		rpc.Write(Quests.Count());
		foreach (DialogueLocQuest quest : Quests)
			quest.OnSend(rpc);
	}

	bool OnRecieve(ParamsReadContext ctx)
	{
		if (!ctx.Read(Language)) return false;

		int treeCount;
		if (!ctx.Read(treeCount)) return false;
		Trees.Clear();
		for (int i = 0; i < treeCount; i++)
		{
			DialogueLocTree tree = new DialogueLocTree();
			if (!tree.OnRecieve(ctx)) return false;
			Trees.Insert(tree);
		}

		int questCount;
		if (!ctx.Read(questCount)) return false;
		Quests.Clear();
		for (int q = 0; q < questCount; q++)
		{
			DialogueLocQuest quest = new DialogueLocQuest();
			if (!quest.OnRecieve(ctx)) return false;
			Quests.Insert(quest);
		}

		m_Lookup = null;
		return true;
	}
}

class DialogueLoc
{
	protected static ref DialogueLoc s_Instance;

	protected ref DialogueLocBundle m_Bundle;

	protected ref map<string, ref DialogueLocBundle> m_Cache;

	protected ref array<string> m_ServerLanguages;

	protected string m_Detected = "";
	protected bool m_DetectionDone;


	protected string m_Requested = "";

	void DialogueLoc()
	{
		m_ServerLanguages = new array<string>;
		m_Cache = new map<string, ref DialogueLocBundle>;
	}

	static DialogueLoc GetInstance()
	{
		if (!s_Instance)
			s_Instance = new DialogueLoc();

		return s_Instance;
	}

	string DetectedLanguage()
	{
		if (m_DetectionDone)
			return m_Detected;

		m_DetectionDone = true;
		m_Detected = DialogueFWLanguages.ENGLISH;

		if (!GetGame() || !GetGame().IsClient())
			return m_Detected;

		string probed = Widget.TranslateString("#STR_DIALOGUEFW_LANGUAGE_ID");
		probed = DialogueFWLanguages.Normalize(probed);

		if (DialogueFWLanguages.IsKnown(probed))
			m_Detected = probed;
		else
			Print("[DialogueFramework] [LOC] Language probe returned '" + probed + "' which is not a known language -- falling back to english.");

		Print("[DialogueFramework] [LOC] Detected game language: " + m_Detected);
		return m_Detected;
	}

	string PreferredLanguage()
	{
		//! Kept in DialogueClientSettings so there is exactly one file holding
		//! what the player chose, alongside their other preferences.
		return DialogueClientSettings.Get().Language;
	}

	void SetPreferredLanguage(string language)
	{
		string wanted = DialogueFWLanguages.Normalize(language);
		if (wanted != "" && !DialogueFWLanguages.IsKnown(wanted))
			return;

		DialogueClientSettings settings = DialogueClientSettings.Get();
		settings.Language = wanted;
		settings.Save();

		Print("[DialogueFramework] [LOC] Preferred dialogue language set to '" + wanted + "' (empty means follow the game language).");
	}

	string EffectiveLanguage()
	{
		string preferred = PreferredLanguage();
		if (preferred != "")
			return preferred;

		return DetectedLanguage();
	}

	array<string> ServerLanguages()
	{
		return m_ServerLanguages;
	}

	void SetServerLanguages(array<string> languages)
	{
		m_ServerLanguages.Clear();

		if (!languages)
			return;

		foreach (string language : languages)
		{
			string normalized = DialogueFWLanguages.Normalize(language);
			if (normalized != "" && m_ServerLanguages.Find(normalized) == -1)
				m_ServerLanguages.Insert(normalized);
		}
	}

	bool ServerHasLanguage(string language)
	{
		return m_ServerLanguages.Find(DialogueFWLanguages.Normalize(language)) != -1;
	}

	string ActiveLanguage()
	{
		if (!m_Bundle)
			return "";

		return m_Bundle.Language;
	}

	void SetBundle(DialogueLocBundle bundle)
	{
		m_Bundle = bundle;

		if (bundle)
		{
			bundle.BuildLookup();

			if (bundle.Language != "")
				m_Cache.Set(bundle.Language, bundle);

			Print("[DialogueFramework] [LOC] Active translation: " + bundle.Language + " (" + bundle.EntryCount() + " line(s)).");
		}
	}

	bool UseCachedBundle(string language)
	{
		DialogueLocBundle cached;
		if (!m_Cache.Find(DialogueFWLanguages.Normalize(language), cached))
			return false;

		m_Bundle = cached;
		return true;
	}

	void ClearBundle()
	{
		m_Bundle = null;
	}

	string RequestedLanguage()
	{
		return m_Requested;
	}

	void SetRequestedLanguage(string language)
	{
		m_Requested = DialogueFWLanguages.Normalize(language);
	}

	bool NeedsBundleFor(string language)
	{
		string wanted = DialogueFWLanguages.Normalize(language);
		if (wanted == "")
			return false;

		if (!ServerHasLanguage(wanted))
			return false;

		return ActiveLanguage() != wanted;
	}

	string Lookup(string scopedKey, string fallback)
	{
		if (!m_Bundle)
			return fallback;

		string found = m_Bundle.Find(scopedKey);
		if (found == "")
			return fallback;

		return found;
	}

	static string ForTree(DialogueTree tree, string key, string fallback)
	{
		if (!tree || key == "")
			return fallback;

		DialogueLoc loc = GetInstance();
		string byFile = "";

		if (tree.LocKey != "")
			byFile = loc.Lookup("F|" + tree.LocKey + "|" + key, "");

		if (byFile != "")
			return byFile;

		if (tree.ID > 0)
			return loc.Lookup("I|" + tree.ID + "|" + key, fallback);

		return fallback;
	}

	static string ForQuest(int questID, string key, string fallback)
	{
		if (questID <= 0 || key == "")
			return fallback;

		return GetInstance().Lookup("Q|" + questID + "|" + key, fallback);
	}
}
