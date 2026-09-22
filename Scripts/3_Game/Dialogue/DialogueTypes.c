class DialogueNodeType
{
	static const string STANDARD = "STANDARD";
	static const string QUEST_LIST = "QUEST_LIST";
	static const string QUEST_DETAIL = "QUEST_DETAIL";
}

class DialogueActionType
{
	static const string NONE = "NONE";
	static const string SHOW_QUEST_LIST = "SHOW_QUEST_LIST";
	static const string ACCEPT_QUEST = "ACCEPT_QUEST";
	static const string DECLINE_QUEST = "DECLINE_QUEST";
	static const string TURN_IN_QUEST = "TURN_IN_QUEST";
	static const string END_CONVERSATION = "END_CONVERSATION";

	static const string OPEN_TRADER = "OPEN_TRADER";

	static const string RECRUIT_AI = "RECRUIT_AI";

	static const string GO_HOSTILE = "GO_HOSTILE";

	//! Opens the quest's offer screen -- description, items, accept/decline --
	//! for the quest named in the response's QuestID.
	static const string OFFER_QUEST = "OFFER_QUEST";
}

//! The game's JSON reader keeps at most 1023 bytes of any one string. A longer
//! line is stored as its start plus the rest in pieces ("...More"), each short
//! enough to survive. The pieces travel to the client as they are and are only
//! joined there, for display, so no long string crosses the network either.
class DialogueText
{
	static const int PIECE_BYTES_LIMIT = 1023;

	static string Join(string first, array<string> more)
	{
		if (!more || more.Count() == 0)
			return first;

		string joined = first;
		foreach (string piece : more)
			joined = joined + piece;

		return joined;
	}

	static void WritePieces(ScriptRPC rpc, array<string> pieces)
	{
		rpc.Write(pieces.Count());
		foreach (string piece : pieces)
			rpc.Write(piece);
	}

	static bool ReadPieces(ParamsReadContext ctx, array<string> pieces)
	{
		int count;
		if (!ctx.Read(count)) return false;

		pieces.Clear();
		for (int i = 0; i < count; i++)
		{
			string piece;
			if (!ctx.Read(piece)) return false;
			pieces.Insert(piece);
		}

		return true;
	}
}

class DialogueVarOp
{
	string Name;
	string Op = "SET";
	int Value = 0;

	void Sanitize()
	{
		if (Op == "")
			Op = "SET";
		Op.ToUpper();
	}

	void OnSend(ScriptRPC rpc)
	{
		rpc.Write(Name);
		rpc.Write(Op);
		rpc.Write(Value);
	}

	bool OnRecieve(ParamsReadContext ctx)
	{
		if (!ctx.Read(Name)) return false;
		if (!ctx.Read(Op)) return false;
		if (!ctx.Read(Value)) return false;
		return true;
	}
}

class DialogueVarOpList
{
	static void Write(ScriptRPC rpc, array<ref DialogueVarOp> ops)
	{
		rpc.Write(ops.Count());
		foreach (DialogueVarOp op : ops)
			op.OnSend(rpc);
	}

	static bool Read(ParamsReadContext ctx, array<ref DialogueVarOp> target)
	{
		int count;
		if (!ctx.Read(count)) return false;
		target.Clear();
		for (int i = 0; i < count; i++)
		{
			DialogueVarOp op = new DialogueVarOp();
			if (!op.OnRecieve(ctx)) return false;
			target.Insert(op);
		}
		return true;
	}

	static void SanitizeAll(array<ref DialogueVarOp> ops)
	{
		if (!ops)
			return;
		foreach (DialogueVarOp op : ops)
		{
			if (op)
				op.Sanitize();
		}
	}

	static bool Compare(int current, string op, int value)
	{
		if (op == "EQUALS") return current == value;
		if (op == "NOT_EQUAL") return current != value;
		if (op == "AT_LEAST") return current >= value;
		if (op == "AT_MOST") return current <= value;
		if (op == "MORE_THAN") return current > value;
		if (op == "BELOW") return current < value;
		return true;
	}

	static bool Evaluate(array<ref DialogueVarOp> conditions, DialoguePlayerState state)
	{
		if (!conditions || conditions.Count() == 0)
			return true;

		foreach (DialogueVarOp condition : conditions)
		{
			if (!condition)
				continue;

			int current = 0;
			if (state)
				current = state.Get(condition.Name);

			if (!Compare(current, condition.Op, condition.Value))
				return false;
		}

		return true;
	}

	static void Apply(array<ref DialogueVarOp> ops, DialoguePlayerState state)
	{
		if (!ops || !state)
			return;

		foreach (DialogueVarOp op : ops)
		{
			if (!op || op.Name == "")
				continue;

			int current = state.Get(op.Name);

			if (op.Op == "INCREASE")
				state.Set(op.Name, current + op.Value);
			else if (op.Op == "DECREASE")
				state.Set(op.Name, current - op.Value);
			else
				state.Set(op.Name, op.Value);
		}
	}
}

class DialogueRepTier
{
	int Threshold = 0;
	string Label;

	//! Which face shows at this rank: "", "HAPPY", "NEUTRAL" or "ANGRY".
	//! Kept apart from the Label on purpose -- the label is the owner's own
	//! word for the rank, "Cool" or "Pissed" or anything else, and the face
	//! is picked separately, so the two never have to agree.
	string Icon;

	void OnSend(ScriptRPC rpc)
	{
		rpc.Write(Threshold);
		rpc.Write(Label);
		rpc.Write(Icon);
	}

	bool OnRecieve(ParamsReadContext ctx)
	{
		if (!ctx.Read(Threshold)) return false;
		if (!ctx.Read(Label)) return false;
		if (!ctx.Read(Icon)) return false;
		return true;
	}

	//! The image file for this tier's face, or "" for no face at all.
	string IconFile()
	{
		string wanted = Icon;
		wanted.ToUpper();

		if (wanted == "HAPPY")
			return "icon_rep_happy_ca";
		if (wanted == "NEUTRAL")
			return "icon_rep_neutral_ca";
		if (wanted == "ANGRY")
			return "icon_rep_angry_ca";
		if (wanted == "THUMBUP")
			return "icon_rep_thumbup_ca";
		if (wanted == "THUMBSIDE")
			return "icon_rep_thumbside_ca";
		if (wanted == "THUMBDOWN")
			return "icon_rep_thumbdown_ca";

		return "";
	}
}

class DialogueRepName
{
	//! A reputation key as something worth showing a player: "yefim_rep" and
	//! "rep_yefim" both come out as "Yefim". Server owners name these keys
	//! themselves and never expected them on screen, so the raw key would
	//! read like a bug.
	static string Pretty(string key)
	{
		string name = key;
		name.ToLower();

		if (name.IndexOf("rep_") == 0)
			name = name.Substring(4, name.Length() - 4);

		int tail = name.Length() - 4;
		if (tail > 0 && name.Substring(tail, 4) == "_rep")
			name = name.Substring(0, tail);

		name.Replace("_", " ");

		string pretty = "";
		bool atStart = true;

		for (int i = 0; i < name.Length(); i++)
		{
			string letter = name.Get(i);
			if (letter == " ")
			{
				atStart = true;
				pretty = pretty + letter;
				continue;
			}

			if (atStart)
				letter.ToUpper();

			atStart = false;
			pretty = pretty + letter;
		}

		if (pretty == "")
			return key;

		return pretty;
	}
}

class DialogueRepTierList
{
	static void Write(ScriptRPC rpc, array<ref DialogueRepTier> tiers)
	{
		rpc.Write(tiers.Count());
		foreach (DialogueRepTier tier : tiers)
			tier.OnSend(rpc);
	}

	static bool Read(ParamsReadContext ctx, array<ref DialogueRepTier> target)
	{
		int count;
		if (!ctx.Read(count)) return false;
		target.Clear();
		for (int i = 0; i < count; i++)
		{
			DialogueRepTier tier = new DialogueRepTier();
			if (!tier.OnRecieve(ctx)) return false;
			target.Insert(tier);
		}
		return true;
	}

	static string LabelFor(array<ref DialogueRepTier> tiers, int value)
	{
		int index = LabelIndexFor(tiers, value);
		if (index < 0)
			return "";

		return tiers[index].Label;
	}

	//! The face for the rank this value falls in, or "" for none.
	static string IconFileFor(array<ref DialogueRepTier> tiers, int value)
	{
		int index = LabelIndexFor(tiers, value);
		if (index < 0)
			return "";

		return tiers[index].IconFile();
	}

	static int LabelIndexFor(array<ref DialogueRepTier> tiers, int value)
	{
		if (!tiers)
			return -1;

		int bestIndex = -1;
		int best = 0;

		for (int i = 0; i < tiers.Count(); i++)
		{
			DialogueRepTier tier = tiers[i];
			if (!tier)
				continue;

			if (value >= tier.Threshold && (bestIndex == -1 || tier.Threshold >= best))
			{
				best = tier.Threshold;
				bestIndex = i;
			}
		}

		return bestIndex;
	}
}

class DialogueSpeakerLine
{
	string Text;
	//! The rest of a line too long for Text alone -- see DialogueText.
	ref array<string> TextMore;
	int RequiredQuestID = -1;
	int OverrideQuestID = -1;

	ref array<string> VoiceLineIDs;
	ref array<ref DialogueVarOp> RequiredVars;

	void DialogueSpeakerLine()
	{
		TextMore = new array<string>;
		VoiceLineIDs = new array<string>;
		RequiredVars = new array<ref DialogueVarOp>;
	}

	void Sanitize()
	{
		if (!TextMore)
			TextMore = new array<string>;

		if (RequiredQuestID <= 0)
			RequiredQuestID = -1;

		if (OverrideQuestID <= 0)
			OverrideQuestID = -1;

		if (!VoiceLineIDs)
			VoiceLineIDs = new array<string>;

		if (!RequiredVars)
			RequiredVars = new array<ref DialogueVarOp>;
		DialogueVarOpList.SanitizeAll(RequiredVars);
	}

	string FullText()
	{
		return DialogueText.Join(Text, TextMore);
	}

	void OnSend(ScriptRPC rpc)
	{
		rpc.Write(Text);
		DialogueText.WritePieces(rpc, TextMore);
		rpc.Write(RequiredQuestID);

		rpc.Write(VoiceLineIDs.Count());
		foreach (string voiceLine : VoiceLineIDs)
			rpc.Write(voiceLine);

		rpc.Write(OverrideQuestID);
		DialogueVarOpList.Write(rpc, RequiredVars);
	}

	bool OnRecieve(ParamsReadContext ctx)
	{
		if (!ctx.Read(Text)) return false;
		if (!DialogueText.ReadPieces(ctx, TextMore)) return false;
		if (!ctx.Read(RequiredQuestID)) return false;

		int voiceCount;
		if (!ctx.Read(voiceCount)) return false;
		VoiceLineIDs.Clear();
		for (int i = 0; i < voiceCount; i++)
		{
			string voiceLine;
			if (!ctx.Read(voiceLine)) return false;
			VoiceLineIDs.Insert(voiceLine);
		}

		if (!ctx.Read(OverrideQuestID)) return false;
		if (!DialogueVarOpList.Read(ctx, RequiredVars)) return false;

		return true;
	}
}

class DialogueNode
{
	int ID;
	string Type = DialogueNodeType.STANDARD;
	string SpeakerText;
	//! The rest of a line too long for SpeakerText alone -- see DialogueText.
	ref array<string> SpeakerTextMore;

	ref array<string> VoiceLineIDs;

	ref array<ref DialogueSpeakerLine> SpeakerLines;

	ref array<ref DialogueResponse> Responses;

	void DialogueNode()
	{
		SpeakerTextMore = new array<string>;
		VoiceLineIDs = new array<string>;
		SpeakerLines = new array<ref DialogueSpeakerLine>;
		Responses = new array<ref DialogueResponse>;
	}

	string FullSpeakerText()
	{
		return DialogueText.Join(SpeakerText, SpeakerTextMore);
	}

	void Sanitize()
	{
		if (!SpeakerTextMore)
			SpeakerTextMore = new array<string>;

		if (!VoiceLineIDs)
			VoiceLineIDs = new array<string>;

		if (!SpeakerLines)
			SpeakerLines = new array<ref DialogueSpeakerLine>;

		foreach (DialogueSpeakerLine line : SpeakerLines)
		{
			if (line)
				line.Sanitize();
		}

		if (!Responses)
			Responses = new array<ref DialogueResponse>;

		foreach (DialogueResponse response : Responses)
		{
			if (response)
				response.Sanitize();
		}
	}

	void OnSend(ScriptRPC rpc)
	{
		rpc.Write(ID);
		rpc.Write(Type);
		rpc.Write(SpeakerText);
		DialogueText.WritePieces(rpc, SpeakerTextMore);

		rpc.Write(VoiceLineIDs.Count());
		foreach (string voiceLine : VoiceLineIDs)
			rpc.Write(voiceLine);

		rpc.Write(Responses.Count());
		foreach (DialogueResponse response : Responses)
			response.OnSend(rpc);

		rpc.Write(SpeakerLines.Count());
		foreach (DialogueSpeakerLine speakerLine : SpeakerLines)
			speakerLine.OnSend(rpc);
	}

	bool OnRecieve(ParamsReadContext ctx)
	{
		if (!ctx.Read(ID)) return false;
		if (!ctx.Read(Type)) return false;
		if (!ctx.Read(SpeakerText)) return false;
		if (!DialogueText.ReadPieces(ctx, SpeakerTextMore)) return false;

		int voiceCount;
		if (!ctx.Read(voiceCount)) return false;
		VoiceLineIDs.Clear();
		for (int i = 0; i < voiceCount; i++)
		{
			string voiceLine;
			if (!ctx.Read(voiceLine)) return false;
			VoiceLineIDs.Insert(voiceLine);
		}

		int responseCount;
		if (!ctx.Read(responseCount)) return false;
		Responses.Clear();
		for (int j = 0; j < responseCount; j++)
		{
			DialogueResponse response = new DialogueResponse();
			if (!response.OnRecieve(ctx)) return false;
			Responses.Insert(response);
		}

		int speakerLineCount;
		if (!ctx.Read(speakerLineCount)) return false;
		SpeakerLines.Clear();
		for (int k = 0; k < speakerLineCount; k++)
		{
			DialogueSpeakerLine speakerLine = new DialogueSpeakerLine();
			if (!speakerLine.OnRecieve(ctx)) return false;
			SpeakerLines.Insert(speakerLine);
		}

		return true;
	}
}

//! What a response can wait for. Strings rather than numbers so a config
//! reads plainly and an unknown value can be ignored instead of misbehaving.
class DialogueQuestStateFilter
{
	static const string ANY = "";
	static const string NOT_STARTED = "NOT_STARTED";
	static const string ACTIVE = "ACTIVE";
	static const string READY = "READY";
	static const string COMPLETED = "COMPLETED";

	static bool IsKnown(string value)
	{
		if (value == ANY || value == NOT_STARTED)
			return true;

		return value == ACTIVE || value == READY || value == COMPLETED;
	}
}

class DialogueResponse
{
	string Text;
	int NextNodeID;
	int RequiredQuestID = -1;

	//! The mirror of RequiredQuestID: once THIS quest is completed the
	//! response stops being offered. Lets a line retire itself.
	int HideAfterQuestID = -1;

	//! Which quest OFFER_QUEST / ACCEPT_QUEST act on. -1 means "whichever
	//! quest the live quest-detail step is showing", the old behaviour.
	int QuestID = -1;

	//! Show this response only while ShowWhileQuestID sits in this state.
	//! -1 / "" means no state gating at all, which is the old behaviour.
	int ShowWhileQuestID = -1;
	string ShowWhileQuestState = DialogueQuestStateFilter.ANY;

	string ActionType = DialogueActionType.NONE;

	ref array<ref DialogueVarOp> RequiredVars;
	ref array<ref DialogueVarOp> SetVars;

	int MaxUses = 0;
	string UsesKey = "";

	void DialogueResponse()
	{
		RequiredVars = new array<ref DialogueVarOp>;
		SetVars = new array<ref DialogueVarOp>;
	}

	void Sanitize()
	{
		if (RequiredQuestID <= 0)
			RequiredQuestID = -1;

		if (HideAfterQuestID <= 0)
			HideAfterQuestID = -1;

		if (QuestID <= 0)
			QuestID = -1;

		if (ShowWhileQuestID <= 0)
			ShowWhileQuestID = -1;

		//! An unrecognised state would otherwise hide the response forever.
		if (!DialogueQuestStateFilter.IsKnown(ShowWhileQuestState))
			ShowWhileQuestState = DialogueQuestStateFilter.ANY;

		if (ActionType == "")
			ActionType = DialogueActionType.NONE;

		if (NextNodeID == 0)
			NextNodeID = -1;

		if (MaxUses < 0)
			MaxUses = 0;

		if (!RequiredVars)
			RequiredVars = new array<ref DialogueVarOp>;
		DialogueVarOpList.SanitizeAll(RequiredVars);

		if (!SetVars)
			SetVars = new array<ref DialogueVarOp>;
		DialogueVarOpList.SanitizeAll(SetVars);
	}

	void OnSend(ScriptRPC rpc)
	{
		rpc.Write(Text);
		rpc.Write(NextNodeID);
		rpc.Write(RequiredQuestID);
		rpc.Write(ActionType);
		DialogueVarOpList.Write(rpc, RequiredVars);
		DialogueVarOpList.Write(rpc, SetVars);
		rpc.Write(MaxUses);
		rpc.Write(UsesKey);
		rpc.Write(HideAfterQuestID);
		rpc.Write(QuestID);
		rpc.Write(ShowWhileQuestID);
		rpc.Write(ShowWhileQuestState);
	}

	bool OnRecieve(ParamsReadContext ctx)
	{
		if (!ctx.Read(Text)) return false;
		if (!ctx.Read(NextNodeID)) return false;
		if (!ctx.Read(RequiredQuestID)) return false;
		if (!ctx.Read(ActionType)) return false;
		if (!DialogueVarOpList.Read(ctx, RequiredVars)) return false;
		if (!DialogueVarOpList.Read(ctx, SetVars)) return false;
		if (!ctx.Read(MaxUses)) return false;
		if (!ctx.Read(UsesKey)) return false;
		if (!ctx.Read(HideAfterQuestID)) return false;
		if (!ctx.Read(QuestID)) return false;
		if (!ctx.Read(ShowWhileQuestID)) return false;
		if (!ctx.Read(ShowWhileQuestState)) return false;
		return true;
	}
}

class DialogueTreeStage
{
	int RequiredQuestID = -1;
	int RootNodeID = 1;
	int Priority = 0;
	ref array<ref DialogueNode> Nodes;
	ref array<ref DialogueVarOp> RequiredVars;

	void DialogueTreeStage()
	{
		Nodes = new array<ref DialogueNode>;
		RequiredVars = new array<ref DialogueVarOp>;
	}

	void Sanitize()
	{
		if (RequiredQuestID <= 0)
			RequiredQuestID = -1;

		if (RootNodeID <= 0)
			RootNodeID = 1;

		if (!Nodes)
			Nodes = new array<ref DialogueNode>;

		foreach (DialogueNode node : Nodes)
		{
			if (node)
				node.Sanitize();
		}

		if (!RequiredVars)
			RequiredVars = new array<ref DialogueVarOp>;
		DialogueVarOpList.SanitizeAll(RequiredVars);
	}

	void OnSend(ScriptRPC rpc)
	{
		rpc.Write(RequiredQuestID);
		rpc.Write(RootNodeID);

		rpc.Write(Nodes.Count());
		foreach (DialogueNode node : Nodes)
			node.OnSend(rpc);

		rpc.Write(Priority);
		DialogueVarOpList.Write(rpc, RequiredVars);
	}

	bool OnRecieve(ParamsReadContext ctx)
	{
		if (!ctx.Read(RequiredQuestID)) return false;
		if (!ctx.Read(RootNodeID)) return false;

		int nodeCount;
		if (!ctx.Read(nodeCount)) return false;
		Nodes.Clear();
		for (int i = 0; i < nodeCount; i++)
		{
			DialogueNode node = new DialogueNode();
			if (!node.OnRecieve(ctx)) return false;
			Nodes.Insert(node);
		}

		if (!ctx.Read(Priority)) return false;
		if (!DialogueVarOpList.Read(ctx, RequiredVars)) return false;

		return true;
	}
}

class DialogueTree
{
	int ID;

	ref array<int> NPCIDs;

	ref array<string> TraderIDs;

	ref array<string> TraderClassNames;

	ref array<string> TraderPositions;

	float TraderPositionRadius = 8.0;

	int TraderMinKeyMatches = 1;

	//! Which player-to-player market trader(s) this conversation belongs to,
	//! by the P2P trader ID from expansion\p2pmarket\P2PTrader_<n>.json. Those
	//! ids are unique per trader, so no class or position narrowing is needed.
	ref array<int> P2PTraderIDs;

	int AIPatrolID = 0;

	int AIPatrolSubID = 0;

	string ReputationVar = "";

	//! The most this character's reputation is meant to reach. Only used for
	//! display -- the standing page reads "10 / 100" instead of a bare number
	//! so a player can see how far there is left to go. Nothing enforces it;
	//! 0 means don't show a total at all.
	int ReputationMax = 0;

	ref array<ref DialogueRepTier> ReputationTiers;

	string LocKey = "";

	int RootNodeID;

	ref array<string> GreetingVoiceLineIDs;
	ref array<string> FarewellVoiceLineIDs;

	ref array<string> QuestListTexts;
	ref array<string> NoQuestsTexts;
	ref array<string> NoQuestsBackTexts;
	ref array<string> NoQuestsLeaveTexts;
	ref array<string> NoQuestsVoiceLineIDs;
	ref array<string> QuestListBackTexts;
	ref array<string> OfferBackTexts;
	ref array<string> InProgressBackTexts;
	ref array<string> TurnInBackTexts;
	ref array<ref DialogueTreeStage> Stages;

	ref array<ref DialogueNode> Nodes;

	void DialogueTree()
	{
		NPCIDs = new array<int>;
		TraderIDs = new array<string>;
		TraderClassNames = new array<string>;
		TraderPositions = new array<string>;
		P2PTraderIDs = new array<int>;
		GreetingVoiceLineIDs = new array<string>;
		FarewellVoiceLineIDs = new array<string>;
		QuestListTexts = new array<string>;
		NoQuestsTexts = new array<string>;
		NoQuestsBackTexts = new array<string>;
		NoQuestsLeaveTexts = new array<string>;
		NoQuestsVoiceLineIDs = new array<string>;
		QuestListBackTexts = new array<string>;
		OfferBackTexts = new array<string>;
		InProgressBackTexts = new array<string>;
		TurnInBackTexts = new array<string>;
		Stages = new array<ref DialogueTreeStage>;
		Nodes = new array<ref DialogueNode>;
		ReputationTiers = new array<ref DialogueRepTier>;
	}

	void Sanitize()
	{
		if (!NPCIDs)
			NPCIDs = new array<int>;

		if (!TraderIDs)
			TraderIDs = new array<string>;

		if (!TraderClassNames)
			TraderClassNames = new array<string>;

		if (!TraderPositions)
			TraderPositions = new array<string>;

		if (TraderPositionRadius <= 0)
			TraderPositionRadius = 8.0;

		if (TraderMinKeyMatches < 1)
			TraderMinKeyMatches = 1;

		if (!P2PTraderIDs)
			P2PTraderIDs = new array<int>;

		if (AIPatrolID < 0)
			AIPatrolID = 0;

		if (AIPatrolSubID < 0)
			AIPatrolSubID = 0;

		if (ReputationMax < 0)
			ReputationMax = 0;

		if (!ReputationTiers)
			ReputationTiers = new array<ref DialogueRepTier>;

		//! A face nobody recognises is dropped rather than left to fail
		//! silently as a missing texture.
		foreach (DialogueRepTier repTier : ReputationTiers)
		{
			if (!repTier)
				continue;

			repTier.Icon.ToUpper();
			if (repTier.Icon != "" && repTier.IconFile() == "")
			{
				Print("[DialogueFramework] A reputation rank asks for the icon '" + repTier.Icon + "', which isn't one of HAPPY, NEUTRAL, ANGRY, THUMBUP, THUMBSIDE or THUMBDOWN -- no icon will be shown for it.");
				repTier.Icon = "";
			}
		}

		if (!GreetingVoiceLineIDs)
			GreetingVoiceLineIDs = new array<string>;

		if (!FarewellVoiceLineIDs)
			FarewellVoiceLineIDs = new array<string>;

		if (!QuestListTexts)
			QuestListTexts = new array<string>;

		if (!NoQuestsTexts)
			NoQuestsTexts = new array<string>;

		if (!NoQuestsBackTexts)
			NoQuestsBackTexts = new array<string>;

		if (!NoQuestsLeaveTexts)
			NoQuestsLeaveTexts = new array<string>;

		if (!NoQuestsVoiceLineIDs)
			NoQuestsVoiceLineIDs = new array<string>;

		if (!QuestListBackTexts)
			QuestListBackTexts = new array<string>;

		if (!OfferBackTexts)
			OfferBackTexts = new array<string>;

		if (!InProgressBackTexts)
			InProgressBackTexts = new array<string>;

		if (!TurnInBackTexts)
			TurnInBackTexts = new array<string>;

		if (!Stages)
			Stages = new array<ref DialogueTreeStage>;

		foreach (DialogueTreeStage stage : Stages)
		{
			if (stage)
				stage.Sanitize();
		}

		if (!Nodes)
			Nodes = new array<ref DialogueNode>;

		foreach (DialogueNode node : Nodes)
		{
			if (node)
				node.Sanitize();
		}
	}

	void OnSend(ScriptRPC rpc)
	{
		rpc.Write(ID);
		rpc.Write(RootNodeID);

		rpc.Write(NPCIDs.Count());
		foreach (int npcID : NPCIDs)
			rpc.Write(npcID);

		rpc.Write(TraderIDs.Count());
		foreach (string traderID : TraderIDs)
			rpc.Write(traderID);

		rpc.Write(TraderClassNames.Count());
		foreach (string traderClass : TraderClassNames)
			rpc.Write(traderClass);

		rpc.Write(TraderPositions.Count());
		foreach (string traderPos : TraderPositions)
			rpc.Write(traderPos);

		rpc.Write(TraderPositionRadius);
		rpc.Write(TraderMinKeyMatches);

		rpc.Write(GreetingVoiceLineIDs.Count());
		foreach (string greeting : GreetingVoiceLineIDs)
			rpc.Write(greeting);

		rpc.Write(FarewellVoiceLineIDs.Count());
		foreach (string farewell : FarewellVoiceLineIDs)
			rpc.Write(farewell);

		rpc.Write(QuestListTexts.Count());
		foreach (string questListLine : QuestListTexts)
			rpc.Write(questListLine);

		rpc.Write(NoQuestsTexts.Count());
		foreach (string noQuestLine : NoQuestsTexts)
			rpc.Write(noQuestLine);

		rpc.Write(NoQuestsBackTexts.Count());
		foreach (string noQuestBack : NoQuestsBackTexts)
			rpc.Write(noQuestBack);

		rpc.Write(NoQuestsLeaveTexts.Count());
		foreach (string noQuestLeave : NoQuestsLeaveTexts)
			rpc.Write(noQuestLeave);

		rpc.Write(NoQuestsVoiceLineIDs.Count());
		foreach (string noQuestVoice : NoQuestsVoiceLineIDs)
			rpc.Write(noQuestVoice);

		rpc.Write(QuestListBackTexts.Count());
		foreach (string questListBack : QuestListBackTexts)
			rpc.Write(questListBack);

		rpc.Write(OfferBackTexts.Count());
		foreach (string offerBack : OfferBackTexts)
			rpc.Write(offerBack);

		rpc.Write(InProgressBackTexts.Count());
		foreach (string inProgressBack : InProgressBackTexts)
			rpc.Write(inProgressBack);

		rpc.Write(TurnInBackTexts.Count());
		foreach (string turnInBack : TurnInBackTexts)
			rpc.Write(turnInBack);

		rpc.Write(Stages.Count());
		foreach (DialogueTreeStage stage : Stages)
			stage.OnSend(rpc);

		rpc.Write(Nodes.Count());
		foreach (DialogueNode node : Nodes)
			node.OnSend(rpc);

		rpc.Write(P2PTraderIDs.Count());
		foreach (int p2pTraderID : P2PTraderIDs)
			rpc.Write(p2pTraderID);

		rpc.Write(AIPatrolID);
		rpc.Write(AIPatrolSubID);
		rpc.Write(ReputationVar);
		rpc.Write(ReputationMax);
		DialogueRepTierList.Write(rpc, ReputationTiers);
		rpc.Write(LocKey);
	}

	bool OnRecieve(ParamsReadContext ctx)
	{
		if (!ctx.Read(ID)) return false;
		if (!ctx.Read(RootNodeID)) return false;

		int npcCount;
		if (!ctx.Read(npcCount)) return false;
		NPCIDs.Clear();
		for (int i = 0; i < npcCount; i++)
		{
			int npcID;
			if (!ctx.Read(npcID)) return false;
			NPCIDs.Insert(npcID);
		}

		int traderCount;
		if (!ctx.Read(traderCount)) return false;
		TraderIDs.Clear();
		for (int t = 0; t < traderCount; t++)
		{
			string traderID;
			if (!ctx.Read(traderID)) return false;
			TraderIDs.Insert(traderID);
		}

		int traderClassCount;
		if (!ctx.Read(traderClassCount)) return false;
		TraderClassNames.Clear();
		for (int tc = 0; tc < traderClassCount; tc++)
		{
			string traderClass;
			if (!ctx.Read(traderClass)) return false;
			TraderClassNames.Insert(traderClass);
		}

		int traderPosCount;
		if (!ctx.Read(traderPosCount)) return false;
		TraderPositions.Clear();
		for (int tp = 0; tp < traderPosCount; tp++)
		{
			string traderPos;
			if (!ctx.Read(traderPos)) return false;
			TraderPositions.Insert(traderPos);
		}

		if (!ctx.Read(TraderPositionRadius)) return false;
		if (!ctx.Read(TraderMinKeyMatches)) return false;

		int greetCount;
		if (!ctx.Read(greetCount)) return false;
		GreetingVoiceLineIDs.Clear();
		for (int g = 0; g < greetCount; g++)
		{
			string greeting;
			if (!ctx.Read(greeting)) return false;
			GreetingVoiceLineIDs.Insert(greeting);
		}

		int farewellCount;
		if (!ctx.Read(farewellCount)) return false;
		FarewellVoiceLineIDs.Clear();
		for (int f = 0; f < farewellCount; f++)
		{
			string farewell;
			if (!ctx.Read(farewell)) return false;
			FarewellVoiceLineIDs.Insert(farewell);
		}

		int questListCount;
		if (!ctx.Read(questListCount)) return false;
		QuestListTexts.Clear();
		for (int qlt = 0; qlt < questListCount; qlt++)
		{
			string questListLine;
			if (!ctx.Read(questListLine)) return false;
			QuestListTexts.Insert(questListLine);
		}

		int noQuestTextCount;
		if (!ctx.Read(noQuestTextCount)) return false;
		NoQuestsTexts.Clear();
		for (int nqt = 0; nqt < noQuestTextCount; nqt++)
		{
			string noQuestLine;
			if (!ctx.Read(noQuestLine)) return false;
			NoQuestsTexts.Insert(noQuestLine);
		}

		int noQuestBackCount;
		if (!ctx.Read(noQuestBackCount)) return false;
		NoQuestsBackTexts.Clear();
		for (int nqb = 0; nqb < noQuestBackCount; nqb++)
		{
			string noQuestBack;
			if (!ctx.Read(noQuestBack)) return false;
			NoQuestsBackTexts.Insert(noQuestBack);
		}

		int noQuestLeaveCount;
		if (!ctx.Read(noQuestLeaveCount)) return false;
		NoQuestsLeaveTexts.Clear();
		for (int nql = 0; nql < noQuestLeaveCount; nql++)
		{
			string noQuestLeave;
			if (!ctx.Read(noQuestLeave)) return false;
			NoQuestsLeaveTexts.Insert(noQuestLeave);
		}

		int noQuestVoiceCount;
		if (!ctx.Read(noQuestVoiceCount)) return false;
		NoQuestsVoiceLineIDs.Clear();
		for (int nqv = 0; nqv < noQuestVoiceCount; nqv++)
		{
			string noQuestVoice;
			if (!ctx.Read(noQuestVoice)) return false;
			NoQuestsVoiceLineIDs.Insert(noQuestVoice);
		}

		int questListBackCount;
		if (!ctx.Read(questListBackCount)) return false;
		QuestListBackTexts.Clear();
		for (int qlb = 0; qlb < questListBackCount; qlb++)
		{
			string questListBack;
			if (!ctx.Read(questListBack)) return false;
			QuestListBackTexts.Insert(questListBack);
		}

		int offerBackCount;
		if (!ctx.Read(offerBackCount)) return false;
		OfferBackTexts.Clear();
		for (int ob = 0; ob < offerBackCount; ob++)
		{
			string offerBack;
			if (!ctx.Read(offerBack)) return false;
			OfferBackTexts.Insert(offerBack);
		}

		int inProgressBackCount;
		if (!ctx.Read(inProgressBackCount)) return false;
		InProgressBackTexts.Clear();
		for (int ipb = 0; ipb < inProgressBackCount; ipb++)
		{
			string inProgressBack;
			if (!ctx.Read(inProgressBack)) return false;
			InProgressBackTexts.Insert(inProgressBack);
		}

		int turnInBackCount;
		if (!ctx.Read(turnInBackCount)) return false;
		TurnInBackTexts.Clear();
		for (int tib = 0; tib < turnInBackCount; tib++)
		{
			string turnInBack;
			if (!ctx.Read(turnInBack)) return false;
			TurnInBackTexts.Insert(turnInBack);
		}

		int stageCount;
		if (!ctx.Read(stageCount)) return false;
		Stages.Clear();
		for (int st = 0; st < stageCount; st++)
		{
			DialogueTreeStage stage = new DialogueTreeStage();
			if (!stage.OnRecieve(ctx)) return false;
			Stages.Insert(stage);
		}

		int nodeCount;
		if (!ctx.Read(nodeCount)) return false;
		Nodes.Clear();
		for (int n = 0; n < nodeCount; n++)
		{
			DialogueNode node = new DialogueNode();
			if (!node.OnRecieve(ctx)) return false;
			Nodes.Insert(node);
		}

		int p2pCount;
		if (!ctx.Read(p2pCount)) return false;
		P2PTraderIDs.Clear();
		for (int pt = 0; pt < p2pCount; pt++)
		{
			int p2pTraderID;
			if (!ctx.Read(p2pTraderID)) return false;
			P2PTraderIDs.Insert(p2pTraderID);
		}

		if (!ctx.Read(AIPatrolID)) return false;
		if (!ctx.Read(AIPatrolSubID)) return false;
		if (!ctx.Read(ReputationVar)) return false;
		if (!ctx.Read(ReputationMax)) return false;
		if (!DialogueRepTierList.Read(ctx, ReputationTiers)) return false;
		if (!ctx.Read(LocKey)) return false;

		return true;
	}
}
