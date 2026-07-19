class DialogueManager
{
	static ref DialogueManager s_Instance;

	static DialogueManager GetInstance()
	{
		if (!s_Instance)
			s_Instance = new DialogueManager();
		return s_Instance;
	}

	protected const string DIALOGUE_PROFILE_FOLDER = "$profile:\\DialogFramework\\Dialogues\\";
	protected const string MENU_CONFIG_FILE = "$profile:\\DialogFramework\\MenuConfig.json";
	protected const string QUEST_TEXT_FOLDER = "$profile:\\DialogFramework\\QuestText\\";
	protected const string SHARED_SUBFOLDER = "Shared";
	protected const string NPC_FOLDER_PREFIX = "NPC_";
	protected const string TRADER_FOLDER_PREFIX = "Trader_";

	protected ref map<int, ref DialogueTree> m_TreesByNPCID;

	protected ref array<ref DialogueTree> m_AllTrees;

	protected ref array<string> m_LoadIssues;

	protected ref DialogueMenuConfig m_MenuConfig;
	protected ref map<int, ref DialogueQuestText> m_QuestTexts = new map<int, ref DialogueQuestText>;

	DialogueQuestText GetQuestText(int questID)
	{
		if (m_QuestTexts.Contains(questID))
			return m_QuestTexts.Get(questID);

		return null;
	}

	map<int, ref DialogueQuestText> GetAllQuestTexts()
	{
		return m_QuestTexts;
	}

	void ClearQuestTexts()
	{
		m_QuestTexts.Clear();
	}

	void RegisterQuestText(DialogueQuestText questText)
	{
		if (questText && questText.QuestID > 0)
			m_QuestTexts.Set(questText.QuestID, questText);
	}

	void LoadQuestTexts()
	{
		m_QuestTexts.Clear();

		if (!FileExist(QUEST_TEXT_FOLDER))
		{
			MakeDirectory(QUEST_TEXT_FOLDER);
			WriteQuestTextExample();
		}

		string fileName;
		FileAttr attr;
		FindFileHandle handle = FindFile(QUEST_TEXT_FOLDER + "*.json", fileName, attr, FindFileFlags.ALL);

		if (fileName != "")
			LoadQuestTextFile(QUEST_TEXT_FOLDER + fileName);

		while (FindNextFile(handle, fileName, attr))
			LoadQuestTextFile(QUEST_TEXT_FOLDER + fileName);

		CloseFindFile(handle);

		Print("[DialogueFramework] Loaded per-quest text for " + m_QuestTexts.Count() + " quest(s).");
	}

	protected void LoadQuestTextFile(string path)
	{
		DialogueQuestTextFile file = new DialogueQuestTextFile();
		JsonFileLoader<DialogueQuestTextFile>.JsonLoadFile(path, file);

		if (!file)
		{
			LogIssue("Could not parse quest text file: " + path);
			return;
		}

		file.Sanitize();

		foreach (DialogueQuestText questText : file.Quests)
		{
			if (!questText)
				continue;

			if (questText.QuestID <= 0)
			{
				LogIssue("Quest text entry with missing/invalid QuestID in " + path);
				continue;
			}

			if (m_QuestTexts.Contains(questText.QuestID))
				LogIssue("Duplicate quest text for quest ID " + questText.QuestID + " in " + path + " -- last one wins.");

			m_QuestTexts.Set(questText.QuestID, questText);
		}
	}

	protected void WriteQuestTextExample()
	{
		string path = QUEST_TEXT_FOLDER + "Example.json";

		FileHandle f = OpenFile(path, FileMode.WRITE);
		if (!f)
			return;

		FPrintln(f, "{");
		FPrintln(f, "    \"Quests\": [");
		FPrintln(f, "        {");
		FPrintln(f, "            \"QuestID\": 9999,");
		FPrintln(f, "            \"AcceptTexts\": [ \"I'll do it.\", \"Consider it handled.\" ],");
		FPrintln(f, "            \"DeclineTexts\": [ \"Not this time.\" ],");
		FPrintln(f, "            \"TurnInTexts\": [ \"It's done.\" ],");
		FPrintln(f, "            \"NotYetTexts\": [ \"Not yet.\" ],");
		FPrintln(f, "            \"InProgressTexts\": [ \"Still working on it.\" ],");
		FPrintln(f, "            \"RewardSelectText\": \"Take your pick.\"");
		FPrintln(f, "        }");
		FPrintln(f, "    ]");
		FPrintln(f, "}");
		CloseFile(f);

		Print("[DialogueFramework] Created quest text example: " + path);
	}

	DialogueMenuConfig GetMenuConfig()
	{
		if (!m_MenuConfig)
			m_MenuConfig = new DialogueMenuConfig();

		return m_MenuConfig;
	}

	void SetMenuConfig(DialogueMenuConfig config)
	{
		m_MenuConfig = config;
	}

	void LoadMenuConfig()
	{
		string path = MENU_CONFIG_FILE;

		m_MenuConfig = new DialogueMenuConfig();

		if (!FileExist(path))
		{
			JsonFileLoader<DialogueMenuConfig>.JsonSaveFile(path, m_MenuConfig);
			Print("[DialogueFramework] Created default menu config: " + path);
			return;
		}

		JsonFileLoader<DialogueMenuConfig>.JsonLoadFile(path, m_MenuConfig);

		if (!m_MenuConfig)
			m_MenuConfig = new DialogueMenuConfig();

		m_MenuConfig.Sanitize();

		if (m_MenuConfig.UpgradeFromOlderVersion())
		{
			JsonFileLoader<DialogueMenuConfig>.JsonSaveFile(path, m_MenuConfig);
			Print("[DialogueFramework] Menu config upgraded with new fields and saved: " + path);
		}

		Print("[DialogueFramework] Loaded menu config: position=" + m_MenuConfig.Position + " size=" + m_MenuConfig.PanelWidth + "x" + m_MenuConfig.PanelHeight + " border=" + m_MenuConfig.WindowBorderThickness + "px visitedOpacity=" + m_MenuConfig.VisitedResponseOpacity);
	}

	void DialogueManager()
	{
		m_TreesByNPCID = new map<int, ref DialogueTree>;
		m_AllTrees = new array<ref DialogueTree>;
		m_LoadIssues = new array<string>;
	}

	void Init()
	{
		LoadAllTrees();
		LoadMenuConfig();
		LoadQuestTexts();
	}

	DialogueTree GetTreeForTraderEntity(string traderID, string className, vector position)
	{
		DialogueTree best = null;
		int bestScore = 0;
		float bestDistance = -1;

		foreach (DialogueTree tree : m_AllTrees)
		{
			if (!tree)
				continue;

			int declared = 0;
			int matched = 0;
			float distance = -1;
			bool positionMatched = false;

			if (tree.TraderPositions && tree.TraderPositions.Count() > 0)
			{
				declared++;

				distance = ClosestListedDistance(tree, position);

				if (distance >= 0 && distance <= tree.TraderPositionRadius)
				{
					matched++;
					positionMatched = true;
				}
			}

			if (tree.TraderClassNames && tree.TraderClassNames.Count() > 0)
			{
				declared++;
				if (ListContainsIgnoreCase(tree.TraderClassNames, className))
					matched++;
			}

			if (tree.TraderIDs && tree.TraderIDs.Count() > 0)
			{
				declared++;
				if (ListContainsIgnoreCase(tree.TraderIDs, traderID))
					matched++;
			}

			if (declared == 0 || matched == 0)
				continue;

			int required = tree.TraderMinKeyMatches;
			if (required > declared)
				required = declared;

			if (matched < required)
				continue;

			int score = matched * 10;
			if (positionMatched)
				score = score + 5;

			bool better = false;
			if (score > bestScore)
			{
				better = true;
			}
			else if (score == bestScore && distance >= 0)
			{
				if (bestDistance < 0 || distance < bestDistance)
					better = true;
			}

			if (better)
			{
				best = tree;
				bestScore = score;
				bestDistance = distance;
			}
		}

		if (best)
			Print("[DialogueFramework] [TRADER] Matched tree ID=" + best.ID + " (score " + bestScore + ", nearest listed position " + bestDistance + "m)");

		return best;
	}

	protected float ClosestListedDistance(DialogueTree tree, vector position)
	{
		float closest = -1;

		foreach (string entry : tree.TraderPositions)
		{
			if (entry == "")
				continue;

			vector listed = entry.ToVector();
			float distance = vector.Distance(listed, position);

			if (closest < 0 || distance < closest)
				closest = distance;
		}

		return closest;
	}

	protected bool ListContainsIgnoreCase(array<string> haystack, string needle)
	{
		if (needle == "")
			return false;

		string wanted = needle;
		wanted.ToLower();

		foreach (string candidate : haystack)
		{
			string lowered = candidate;
			lowered.ToLower();
			if (lowered == wanted)
				return true;
		}

		return false;
	}

	DialogueTree GetTreeForTrader(string traderID)
	{
		if (traderID == "")
			return null;

		string wanted = traderID;
		wanted.ToLower();

		foreach (DialogueTree tree : m_AllTrees)
		{
			if (!tree || !tree.TraderIDs)
				continue;

			foreach (string candidate : tree.TraderIDs)
			{
				string lowered = candidate;
				lowered.ToLower();
				if (lowered == wanted)
					return tree;
			}
		}

		return null;
	}

	DialogueTree GetTreeForNPC(int npcID)
	{
		return m_TreesByNPCID.Get(npcID);
	}

	void ValidateTree(DialogueTree tree, string context)
	{
		if (!tree || !tree.Nodes)
			return;

		array<int> seenIDs = new array<int>;
		foreach (DialogueNode node : tree.Nodes)
		{
			if (!node)
				continue;

			if (seenIDs.Find(node.ID) != -1)
				LogIssue(context + ": duplicate node ID " + node.ID + " -- only the first one found will ever be reached, the rest are dead.");
			else
				seenIDs.Insert(node.ID);
		}

		if (seenIDs.Find(tree.RootNodeID) == -1)
			LogIssue(context + ": RootNodeID " + tree.RootNodeID + " doesn't match any node -- this tree will never render anything for its NPC(s).");

		foreach (DialogueNode checkNode : tree.Nodes)
		{
			if (!checkNode || !checkNode.Responses)
				continue;

			foreach (DialogueResponse response : checkNode.Responses)
			{
				if (!response)
					continue;

				if (response.NextNodeID == -1)
					continue;

				if (response.ActionType != DialogueActionType.NONE)
					continue;

				if (seenIDs.Find(response.NextNodeID) == -1)
					LogIssue(context + ": node " + checkNode.ID + " has a response (\"" + response.Text + "\") pointing at NextNodeID " + response.NextNodeID + ", which doesn't exist -- likely a typo.");
			}
		}
	}

	void DumpTreeDiagnostic(DialogueTree tree, string context)
	{
		if (!tree)
		{
			Print("[DialogueFramework] [DUMP-" + context + "] Tree is NULL.");
			return;
		}

		string npcIDsStr = "";
		if (tree.NPCIDs)
		{
			foreach (int npcID : tree.NPCIDs)
				npcIDsStr = npcIDsStr + npcID + " ";
		}
		else
		{
			npcIDsStr = "NULL";
		}

		int greetCount = -1;
		if (tree.GreetingVoiceLineIDs)
			greetCount = tree.GreetingVoiceLineIDs.Count();

		int farewellCount = -1;
		if (tree.FarewellVoiceLineIDs)
			farewellCount = tree.FarewellVoiceLineIDs.Count();

		int nodeCount = -1;
		if (tree.Nodes)
			nodeCount = tree.Nodes.Count();

		Print("[DialogueFramework] [DUMP-" + context + "] Tree ID=" + tree.ID + " RootNodeID=" + tree.RootNodeID + " NPCIDs=[" + npcIDsStr + "] GreetingCount=" + greetCount + " FarewellCount=" + farewellCount + " NodeCount=" + nodeCount);

		if (!tree.Nodes)
			return;

		foreach (DialogueNode node : tree.Nodes)
		{
			if (!node)
			{
				Print("[DialogueFramework] [DUMP-" + context + "]   Node entry is NULL.");
				continue;
			}

			int voiceCount = -1;
			if (node.VoiceLineIDs)
				voiceCount = node.VoiceLineIDs.Count();

			int responseCount = -1;
			if (node.Responses)
				responseCount = node.Responses.Count();

			Print("[DialogueFramework] [DUMP-" + context + "]   Node ID=" + node.ID + " Type=" + node.Type + " VoiceLineCount=" + voiceCount + " ResponseCount=" + responseCount);
		}
	}

	void AuditVoiceLines()
	{
		if (!GetGame().IsClient())
			return;

		array<string> missing = new array<string>;
		int checkedCount = 0;

		foreach (DialogueTree tree : m_AllTrees)
		{
			if (!tree)
				continue;

			foreach (string greetingID : tree.GreetingVoiceLineIDs)
			{
				checkedCount++;
				AuditOneVoiceLine(greetingID, missing);
			}

			foreach (string farewellID : tree.FarewellVoiceLineIDs)
			{
				checkedCount++;
				AuditOneVoiceLine(farewellID, missing);
			}

			if (!tree.Nodes)
				continue;

			foreach (DialogueNode node : tree.Nodes)
			{
				if (!node || !node.VoiceLineIDs)
					continue;

				foreach (string nodeVoiceID : node.VoiceLineIDs)
				{
					checkedCount++;
					AuditOneVoiceLine(nodeVoiceID, missing);
				}
			}
		}

		if (missing.Count() == 0)
		{
			Print("[DialogueFramework] [VOICE-AUDIT] " + checkedCount + " voice line ID(s) checked, all have a matching SoundSet.");
			return;
		}

		Print("[DialogueFramework] [VOICE-AUDIT] " + checkedCount + " voice line ID(s) checked, " + missing.Count() + " with NO matching SoundSet (these will be silent):");
		foreach (string missingID : missing)
			Print("[DialogueFramework] [VOICE-AUDIT]   missing: DialogueFW_" + missingID + "_SoundSet");
	}

	protected void AuditOneVoiceLine(string voiceLineID, array<string> missing)
	{
		if (voiceLineID == "")
			return;

		if (GetGame().ConfigIsExisting("CfgSoundSets DialogueFW_" + voiceLineID + "_SoundSet"))
			return;

		foreach (string alreadyListed : missing)
		{
			if (alreadyListed == voiceLineID)
				return;
		}

		missing.Insert(voiceLineID);
	}

	array<ref DialogueTree> GetAllTrees()
	{
		return m_AllTrees;
	}

	void ClearClientTrees()
	{
		m_TreesByNPCID.Clear();
		m_AllTrees.Clear();
	}

	void RegisterTree(DialogueTree tree)
	{
		if (!tree)
			return;

		m_AllTrees.Insert(tree);

		if (!tree.NPCIDs)
			return;

		foreach (int npcID : tree.NPCIDs)
		{
			if (!m_TreesByNPCID.Contains(npcID))
				m_TreesByNPCID.Insert(npcID, tree);
		}
	}

	protected void LogIssue(string message)
	{
		Print("[DialogueFramework] [WARNING] " + message);
		m_LoadIssues.Insert(message);
	}

	protected void LoadAllTrees()
	{
		m_TreesByNPCID.Clear();
		m_AllTrees.Clear();
		m_LoadIssues.Clear();

		if (!FileExist(DIALOGUE_PROFILE_FOLDER))
		{
			ExpansionStatic.MakeDirectoryRecursive(DIALOGUE_PROFILE_FOLDER);
			WriteExampleFiles();
			Print("[DialogueFramework] No dialogue trees found -- wrote an example to " + DIALOGUE_PROFILE_FOLDER);
			WriteLoadLog(0, 0);
			return;
		}

		array<string> entries = new array<string>;
		string entryName;
		FileAttr entryAttr;
		FindFileHandle handle = FindFile(DIALOGUE_PROFILE_FOLDER + "*", entryName, entryAttr, FindFileFlags.ALL);
		if (handle)
		{
			if (entryName.Length() > 0)
				entries.Insert(entryName);

			while (FindNextFile(handle, entryName, entryAttr))
			{
				if (entryName.Length() > 0)
					entries.Insert(entryName);
			}

			CloseFindFile(handle);
		}

		if (entries.Count() == 0)
		{
			WriteExampleFiles();
			Print("[DialogueFramework] Dialogues folder empty -- wrote an example to " + DIALOGUE_PROFILE_FOLDER);
			WriteLoadLog(0, 0);
			return;
		}

		int treeCount = 0;
		int npcCount = 0;

		foreach (string entry : entries)
		{
			if (entry == "." || entry == ".." || entry == "README.txt" || entry == "LoadLog.txt")
				continue;

			if (ExpansionString.EndsWithIgnoreCase(entry, ".json"))
			{
				LoadTreeFile(DIALOGUE_PROFILE_FOLDER + entry, -1, "", treeCount, npcCount);
				continue;
			}

			string folderTraderID = "";
			if (entry.Length() > TRADER_FOLDER_PREFIX.Length() && entry.IndexOf(TRADER_FOLDER_PREFIX) == 0)
				folderTraderID = entry.Substring(TRADER_FOLDER_PREFIX.Length(), entry.Length() - TRADER_FOLDER_PREFIX.Length());

			int folderNPCID = -1;
			if (entry.Length() > NPC_FOLDER_PREFIX.Length() && entry.IndexOf(NPC_FOLDER_PREFIX) == 0)
			{
				string idPart = entry.Substring(NPC_FOLDER_PREFIX.Length(), entry.Length() - NPC_FOLDER_PREFIX.Length());
				folderNPCID = idPart.ToInt();
				if (folderNPCID == 0)
					folderNPCID = -1;
			}

			string subfolder = DIALOGUE_PROFILE_FOLDER + entry + "\\";
			array<string> jsonFiles = ExpansionStatic.FindFilesInLocation(subfolder, ".json");
			if (!jsonFiles)
				continue;

			foreach (string fileName : jsonFiles)
				LoadTreeFile(subfolder + fileName, folderNPCID, folderTraderID, treeCount, npcCount);
		}

		Print("[DialogueFramework] Loaded " + treeCount + " dialogue tree file(s) covering " + npcCount + " NPC ID(s).");
		WriteLoadLog(treeCount, npcCount);
	}

	protected void LoadTreeFile(string fullPath, int folderNPCID, string folderTraderID, inout int treeCount, inout int npcCount)
	{
		DialogueTree tree = new DialogueTree();
		JsonFileLoader<DialogueTree>.JsonLoadFile(fullPath, tree);
		tree.Sanitize();
		DumpTreeDiagnostic(tree, "SERVER-LOAD (" + fullPath + ")");
		ValidateTree(tree, fullPath);

		if ((!tree.NPCIDs || tree.NPCIDs.Count() == 0) && folderNPCID != -1)
			tree.NPCIDs.Insert(folderNPCID);

		if ((!tree.TraderIDs || tree.TraderIDs.Count() == 0) && folderTraderID != "")
			tree.TraderIDs.Insert(folderTraderID);

		bool hasNPCs = tree.NPCIDs && tree.NPCIDs.Count() > 0;

		bool hasTraders = false;
		if (tree.TraderIDs && tree.TraderIDs.Count() > 0)
			hasTraders = true;
		if (tree.TraderClassNames && tree.TraderClassNames.Count() > 0)
			hasTraders = true;
		if (tree.TraderPositions && tree.TraderPositions.Count() > 0)
			hasTraders = true;

		if (!hasNPCs && !hasTraders)
		{
			LogIssue(fullPath + " has no NPCIDs and no trader keys (TraderIDs / TraderClassNames / TraderPositions), and isn't in a NPC_<id> or Trader_<name> folder -- skipping. (This also fires if the file failed to parse as valid JSON.)");
			return;
		}

		if (hasTraders)
		{
			foreach (string traderID : tree.TraderIDs)
				Print("[DialogueFramework] Loaded trader tree for '" + traderID + "' from " + fullPath);
		}

		if (!hasNPCs)
		{
			m_AllTrees.Insert(tree);
			treeCount++;
			return;
		}

		if (folderNPCID != -1 && tree.NPCIDs.Find(folderNPCID) == -1)
		{
			LogIssue(fullPath + " is in folder NPC_" + folderNPCID + " but its NPCIDs list doesn't include " + folderNPCID + " -- likely a copy/paste mistake. Using what the file actually declares.");
		}

		foreach (int npcID : tree.NPCIDs)
		{
			if (m_TreesByNPCID.Contains(npcID))
			{
				LogIssue("NPC ID " + npcID + " is claimed by more than one dialogue file (" + fullPath + " conflicts with an earlier file) -- keeping the first one loaded, this file's entry for NPC " + npcID + " was NOT loaded.");
				continue;
			}

			m_TreesByNPCID.Insert(npcID, tree);
			npcCount++;
		}

		m_AllTrees.Insert(tree);
		treeCount++;
	}

	protected void WriteLoadLog(int treeCount, int npcCount)
	{
		string path = DIALOGUE_PROFILE_FOLDER + "LoadLog.txt";
		FileHandle f = OpenFile(path, FileMode.WRITE);
		if (f == 0)
			return;

		int year, month, day, hour, minute, second;
		GetYearMonthDay(year, month, day);
		GetHourMinuteSecond(hour, minute, second);

		FPrintln(f, "DialogueFramework -- Last Load Report");
		FPrintln(f, "=============================================");
		FPrintln(f, "Server start: " + year + "-" + month + "-" + day + " " + hour + ":" + minute + ":" + second);
		FPrintln(f, "Loaded " + treeCount + " dialogue tree file(s) covering " + npcCount + " NPC ID(s).");
		FPrintln(f, "");

		if (m_LoadIssues.Count() == 0)
		{
			FPrintln(f, "No warnings or errors.");
		}
		else
		{
			FPrintln(f, "" + m_LoadIssues.Count() + " warning(s)/error(s) -- files or NPC IDs listed below were NOT loaded as written:");
			FPrintln(f, "");
			for (int i = 0; i < m_LoadIssues.Count(); i++)
				FPrintln(f, "" + (i + 1) + ". " + m_LoadIssues[i]);
		}

		CloseFile(f);
	}

	protected void WriteExampleFiles()
	{
		string exampleFolder = DIALOGUE_PROFILE_FOLDER + "NPC_9999\\";
		ExpansionStatic.MakeDirectoryRecursive(exampleFolder);

		DialogueTree example = BuildExampleTree();
		JsonFileLoader<DialogueTree>.JsonSaveFile(exampleFolder + "Dialogue.json", example);

		ExpansionStatic.MakeDirectoryRecursive(DIALOGUE_PROFILE_FOLDER + SHARED_SUBFOLDER + "\\");

		WriteReadme();
	}

	protected DialogueTree BuildExampleTree()
	{
		DialogueTree tree = new DialogueTree();
		tree.ID = 9999;

		tree.RootNodeID = 1;

		tree.GreetingVoiceLineIDs.Insert("Example_Greeting_1");
		tree.GreetingVoiceLineIDs.Insert("Example_Greeting_2");
		tree.GreetingVoiceLineIDs.Insert("Example_Greeting_3");
		tree.FarewellVoiceLineIDs.Insert("Example_Farewell_1");
		tree.FarewellVoiceLineIDs.Insert("Example_Farewell_2");
		tree.FarewellVoiceLineIDs.Insert("Example_Farewell_3");

		DialogueNode root = new DialogueNode();
		root.ID = 1;
		root.Type = DialogueNodeType.STANDARD;
		root.SpeakerText = "Well, look who wandered in. What do you need?";

		DialogueResponse work = new DialogueResponse();
		work.Text = "I'm looking for work.";
		work.ActionType = DialogueActionType.SHOW_QUEST_LIST;
		root.Responses.Insert(work);

		DialogueResponse nevermind = new DialogueResponse();
		nevermind.Text = "Nevermind.";
		nevermind.ActionType = DialogueActionType.END_CONVERSATION;
		root.Responses.Insert(nevermind);

		DialogueResponse smalltalk1 = new DialogueResponse();
		smalltalk1.Text = "Rough weather we're having.";
		smalltalk1.ActionType = DialogueActionType.NONE;
		smalltalk1.NextNodeID = 2;
		root.Responses.Insert(smalltalk1);

		DialogueResponse smalltalk2 = new DialogueResponse();
		smalltalk2.Text = "Seen any infected nearby?";
		smalltalk2.ActionType = DialogueActionType.NONE;
		smalltalk2.NextNodeID = 3;
		root.Responses.Insert(smalltalk2);

		DialogueResponse smalltalk3 = new DialogueResponse();
		smalltalk3.Text = "How long have you been out here?";
		smalltalk3.ActionType = DialogueActionType.NONE;
		smalltalk3.NextNodeID = 4;
		root.Responses.Insert(smalltalk3);

		DialogueNode weatherNode = new DialogueNode();
		weatherNode.ID = 2;
		weatherNode.SpeakerText = "Tell me about it. Hasn't let up in days.";
		DialogueResponse back1 = new DialogueResponse();
		back1.Text = "Anyway...";
		back1.NextNodeID = 1;
		weatherNode.Responses.Insert(back1);

		DialogueNode infectedNode = new DialogueNode();
		infectedNode.ID = 3;
		infectedNode.SpeakerText = "A few stragglers past the treeline. Nothing I can't handle.";
		DialogueResponse back2 = new DialogueResponse();
		back2.Text = "Good to know.";
		back2.NextNodeID = 1;
		infectedNode.Responses.Insert(back2);

		DialogueNode howLongNode = new DialogueNode();
		howLongNode.ID = 4;
		howLongNode.SpeakerText = "Longer than I'd like. You lose count after a while.";
		DialogueResponse back3 = new DialogueResponse();
		back3.Text = "Fair enough.";
		back3.NextNodeID = 1;
		howLongNode.Responses.Insert(back3);

		tree.Nodes.Insert(root);
		tree.Nodes.Insert(weatherNode);
		tree.Nodes.Insert(infectedNode);
		tree.Nodes.Insert(howLongNode);

		return tree;
	}

	protected void WriteReadme()
	{
		string path = DIALOGUE_PROFILE_FOLDER + "README.txt";
		FileHandle f = OpenFile(path, FileMode.WRITE);
		if (f == 0)
			return;

		FPrintln(f, "DialogueFramework -- Dialogue Tree Reference");
		FPrintln(f, "=============================================");
		FPrintln(f, "");
		FPrintln(f, "FOLDER LAYOUT");
		FPrintln(f, "  NPC_<id>\\*.json  - dedicated to exactly one NPC ID. Leave NPCIDs");
		FPrintln(f, "                     empty in these files -- it's inferred from the");
		FPrintln(f, "                     folder name automatically. This is the folder to");
		FPrintln(f, "                     use for almost everything: copy NPC_9999, rename");
		FPrintln(f, "                     it to match your NPC's real ID, edit the json.");
		FPrintln(f, "  Shared\\*.json    - for one tree deliberately covering MULTIPLE NPCs.");
		FPrintln(f, "                     NPCIDs must be set explicitly here (list every ID");
		FPrintln(f, "                     that should use this tree).");
		FPrintln(f, "");
		FPrintln(f, "  If a file sits in a NPC_<id> folder but its own NPCIDs (if set)");
		FPrintln(f, "  doesn't include that id, a warning is logged on server start -- this");
		FPrintln(f, "  usually means you copy/pasted a file into a new folder and forgot to");
		FPrintln(f, "  update something, or just forgot to clear NPCIDs entirely.");
		FPrintln(f, "");
		FPrintln(f, "  After every server start, check LoadLog.txt in this same folder -- it");
		FPrintln(f, "  lists exactly what loaded, and exactly which files/NPC IDs (if any)");
		FPrintln(f, "  failed and were skipped, with the reason why. A conflict or bad file");
		FPrintln(f, "  never stops the server from starting -- it just means that one NPC");
		FPrintln(f, "  falls back to no custom dialogue until you fix it.");
		FPrintln(f, "");
		FPrintln(f, "Files are loaded on server start (restart required after editing).");
		FPrintln(f, "");
		FPrintln(f, "DialogueTree fields:");
		FPrintln(f, "  ID              - your own identifier for this tree, for logging only");
		FPrintln(f, "  NPCIDs          - array of Quest NPC IDs (from your QuestNPC_X.json");
		FPrintln(f, "                    'ID' field). Leave empty in a NPC_<id> folder to");
		FPrintln(f, "                    use that folder's id automatically. Required");
		FPrintln(f, "                    (non-empty) in Shared\\.");
		FPrintln(f, "  RootNodeID      - which node ID in Nodes[] is shown first");
		FPrintln(f, "  GreetingVoiceLineIDs - array of voice line IDs, one picked at random");
		FPrintln(f, "                    when the conversation opens");
		FPrintln(f, "  FarewellVoiceLineIDs - same, played when the player picks a response");
		FPrintln(f, "                    with ActionType END_CONVERSATION");
		FPrintln(f, "                  - your own text for the live quest-detail step's");
		FPrintln(f, "                    responses. EVERY entry shows as its own button (not");
		FPrintln(f, "                    a random pick) -- these are tone variants meant to");
		FPrintln(f, "                    appear together. Leave any of these empty to fall");
		FPrintln(f, "                    back to a plain default instead.");
		FPrintln(f, "                  - spoken prompt when a quest lets the player pick");
		FPrintln(f, "                    one of several rewards. ONE is picked at random");
		FPrintln(f, "                    (it is a line of dialogue, not a set of buttons).");
		FPrintln(f, "                    Empty falls back to a plain default.");
		FPrintln(f, "  Nodes           - array of DialogueNode");
		FPrintln(f, "");
		FPrintln(f, "DialogueNode fields:");
		FPrintln(f, "  ID              - referenced by Responses[].NextNodeID");
		FPrintln(f, "  Type            - STANDARD (normal authored node), QUEST_LIST or");
		FPrintln(f, "                    QUEST_DETAIL (both are built live by the mod, you");
		FPrintln(f, "                    never author these two yourself)");
		FPrintln(f, "  SpeakerText     - text shown for this line");
		FPrintln(f, "  VoiceLineIDs    - array of voice line IDs, one picked at random");
		FPrintln(f, "  Responses       - array of DialogueResponse");
		FPrintln(f, "");
		FPrintln(f, "DialogueResponse fields:");
		FPrintln(f, "  Text            - button text shown to the player");
		FPrintln(f, "  NextNodeID      - which node to show next. -1 ends the conversation");
		FPrintln(f, "  RequiredQuestID - optional, -1 for no gating. Set to only show this");
		FPrintln(f, "                    response if that quest ID is complete/active");
		FPrintln(f, "  ActionType      - NONE (just go to NextNodeID), SHOW_QUEST_LIST");
		FPrintln(f, "                    (opens live quest list), ACCEPT_QUEST /");
		FPrintln(f, "                    DECLINE_QUEST (only meaningful inside a QUEST_DETAIL");
		FPrintln(f, "                    step), END_CONVERSATION (plays a farewell line and");
		FPrintln(f, "                    closes)");
		FPrintln(f, "");
		FPrintln(f, "Per-quest button text:");
		FPrintln(f, "  Accept / decline / turn-in wording can be set PER QUEST in");
		FPrintln(f, "  $profile:\\DialogFramework\\QuestText\\*.json, so each quest can read");
		FPrintln(f, "  in its own voice. Anything not set there falls back to this tree's");
		FPrintln(f, "  Quest*Texts arrays, then to plain built-in wording.");
		FPrintln(f, "");
		FPrintln(f, "Voice line IDs:");
		FPrintln(f, "  A VoiceLineID of \"Example_Greeting_1\" expects a CfgSoundSets entry");
		FPrintln(f, "  named \"DialogueFW_Example_Greeting_1_SoundSet\" to exist -- defined in");
		FPrintln(f, "  the separate DialogueFramework_SoundSets addon, alongside the actual");
		FPrintln(f, "  .ogg file. See that addon's own README for how to add your own.");
		FPrintln(f, "  If a VoiceLineID has no matching SoundSet, that line just plays with");
		FPrintln(f, "  no audio -- it will not error or break the conversation.");
		FPrintln(f, "");
		FPrintln(f, "Validation on load (reported in LoadLog.txt, never fatal):");
		FPrintln(f, "  - A NextNodeID that doesn't match any node in the file (usually a typo)");
		FPrintln(f, "  - Two nodes in the same file sharing the same ID");
		FPrintln(f, "  - A RootNodeID that doesn't match any node at all");
		FPrintln(f, "  These don't stop the server or break other NPCs -- the affected branch");
		FPrintln(f, "  just won't be reachable until fixed. Check LoadLog.txt after every restart.");
		FPrintln(f, "");
		FPrintln(f, "See docs/DIALOGUE_TREE_GUIDE.md in the mod's repository for the full");
		FPrintln(f, "reference, including guidance on building large branching trees.");

		CloseFile(f);
	}
}
