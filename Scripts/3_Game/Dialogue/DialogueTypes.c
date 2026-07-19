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
}

class DialogueNode
{
	int ID;
	string Type = DialogueNodeType.STANDARD;
	string SpeakerText;

	ref array<string> VoiceLineIDs;

	ref array<ref DialogueResponse> Responses;

	void DialogueNode()
	{
		VoiceLineIDs = new array<string>;
		Responses = new array<ref DialogueResponse>;
	}

	void Sanitize()
	{
		if (!VoiceLineIDs)
			VoiceLineIDs = new array<string>;

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

		return true;
	}
}

class DialogueResponse
{
	string Text;
	int NextNodeID;
	int RequiredQuestID = -1;
	string ActionType = DialogueActionType.NONE;

	void Sanitize()
	{
		if (RequiredQuestID <= 0)
			RequiredQuestID = -1;

		if (ActionType == "")
			ActionType = DialogueActionType.NONE;

		if (NextNodeID == 0)
			NextNodeID = -1;
	}

	void OnSend(ScriptRPC rpc)
	{
		rpc.Write(Text);
		rpc.Write(NextNodeID);
		rpc.Write(RequiredQuestID);
		rpc.Write(ActionType);
	}

	bool OnRecieve(ParamsReadContext ctx)
	{
		if (!ctx.Read(Text)) return false;
		if (!ctx.Read(NextNodeID)) return false;
		if (!ctx.Read(RequiredQuestID)) return false;
		if (!ctx.Read(ActionType)) return false;
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

	int RootNodeID;

	ref array<string> GreetingVoiceLineIDs;
	ref array<string> FarewellVoiceLineIDs;

	ref array<ref DialogueNode> Nodes;

	void DialogueTree()
	{
		NPCIDs = new array<int>;
		TraderIDs = new array<string>;
		TraderClassNames = new array<string>;
		TraderPositions = new array<string>;
		GreetingVoiceLineIDs = new array<string>;
		FarewellVoiceLineIDs = new array<string>;
		Nodes = new array<ref DialogueNode>;
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

		if (!GreetingVoiceLineIDs)
			GreetingVoiceLineIDs = new array<string>;

		if (!FarewellVoiceLineIDs)
			FarewellVoiceLineIDs = new array<string>;

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

		rpc.Write(Nodes.Count());
		foreach (DialogueNode node : Nodes)
			node.OnSend(rpc);
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

		int nodeCount;
		if (!ctx.Read(nodeCount)) return false;
		Nodes.Clear();
		for (int n = 0; n < nodeCount; n++)
		{
			DialogueNode node = new DialogueNode();
			if (!node.OnRecieve(ctx)) return false;
			Nodes.Insert(node);
		}

		return true;
	}
}
