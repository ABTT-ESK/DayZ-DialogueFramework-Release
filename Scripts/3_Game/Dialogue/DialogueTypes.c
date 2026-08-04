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

	void OnSend(ScriptRPC rpc)
	{
		rpc.Write(Threshold);
		rpc.Write(Label);
	}

	bool OnRecieve(ParamsReadContext ctx)
	{
		if (!ctx.Read(Threshold)) return false;
		if (!ctx.Read(Label)) return false;
		return true;
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
		string label = "";
		int best = 0;
		bool found = false;
		foreach (DialogueRepTier tier : tiers)
		{
			if (!tier)
				continue;
			if (value >= tier.Threshold && (!found || tier.Threshold >= best))
			{
				best = tier.Threshold;
				label = tier.Label;
				found = true;
			}
		}
		return label;
	}
}

class DialogueSpeakerLine
{
	string Text;
	int RequiredQuestID = -1;
	int OverrideQuestID = -1;

	ref array<string> VoiceLineIDs;
	ref array<ref DialogueVarOp> RequiredVars;

	void DialogueSpeakerLine()
	{
		VoiceLineIDs = new array<string>;
		RequiredVars = new array<ref DialogueVarOp>;
	}

	void Sanitize()
	{
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

	void OnSend(ScriptRPC rpc)
	{
		rpc.Write(Text);
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

	ref array<string> VoiceLineIDs;

	ref array<ref DialogueSpeakerLine> SpeakerLines;

	ref array<ref DialogueResponse> Responses;

	void DialogueNode()
	{
		VoiceLineIDs = new array<string>;
		SpeakerLines = new array<ref DialogueSpeakerLine>;
		Responses = new array<ref DialogueResponse>;
	}

	void Sanitize()
	{
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

class DialogueResponse
{
	string Text;
	int NextNodeID;
	int RequiredQuestID = -1;
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

	int AIPatrolID = 0;

	int AIPatrolSubID = 0;

	string ReputationVar = "";

	ref array<ref DialogueRepTier> ReputationTiers;

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

		if (AIPatrolID < 0)
			AIPatrolID = 0;

		if (AIPatrolSubID < 0)
			AIPatrolSubID = 0;

		if (!ReputationTiers)
			ReputationTiers = new array<ref DialogueRepTier>;

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

		rpc.Write(AIPatrolID);
		rpc.Write(AIPatrolSubID);
		rpc.Write(ReputationVar);
		DialogueRepTierList.Write(rpc, ReputationTiers);
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

		if (!ctx.Read(AIPatrolID)) return false;
		if (!ctx.Read(AIPatrolSubID)) return false;
		if (!ctx.Read(ReputationVar)) return false;
		if (!DialogueRepTierList.Read(ctx, ReputationTiers)) return false;

		return true;
	}
}
