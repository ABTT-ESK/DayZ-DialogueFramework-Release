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

//! One candidate line for a node. When a node carries these, the menu picks
//! one at random from the base SpeakerText plus whichever of these the player
//! qualifies for, so a greeting can vary and a line can be revealed once a
//! quest is completed. RequiredQuestID <= 0 means "always shown".
class DialogueSpeakerLine
{
	string Text;

	//! Eligibility gate: <= 0 means always in the random pool, otherwise this
	//! line only joins the pool once the quest is COMPLETED.
	int RequiredQuestID = -1;

	//! Override gate: once this quest is COMPLETED this line stops being one of
	//! the random options and becomes the fixed greeting. If several lines
	//! qualify, the one with the highest OverrideQuestID wins. <= 0 = no
	//! override.
	int OverrideQuestID = -1;

	ref array<string> VoiceLineIDs;

	void DialogueSpeakerLine()
	{
		VoiceLineIDs = new array<string>;
	}

	void Sanitize()
	{
		if (RequiredQuestID <= 0)
			RequiredQuestID = -1;

		if (OverrideQuestID <= 0)
			OverrideQuestID = -1;

		if (!VoiceLineIDs)
			VoiceLineIDs = new array<string>;
	}

	void OnSend(ScriptRPC rpc)
	{
		rpc.Write(Text);
		rpc.Write(RequiredQuestID);

		rpc.Write(VoiceLineIDs.Count());
		foreach (string voiceLine : VoiceLineIDs)
			rpc.Write(voiceLine);

		rpc.Write(OverrideQuestID);
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

		return true;
	}
}

class DialogueNode
{
	int ID;
	string Type = DialogueNodeType.STANDARD;
	string SpeakerText;

	ref array<string> VoiceLineIDs;

	//! Optional extra lines. Empty = just SpeakerText, exactly as before.
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

	//! Lines spoken above the live quest list, one picked at random each time.
	//! Empty = built-in wording.
	ref array<string> QuestListTexts;

	//! Fallback for when this NPC has nothing available and no completed quest
	//! of theirs supplies its own NoQuestsTexts. One picked at random.
	ref array<string> NoQuestsTexts;

	//! Fallback buttons for that same case. Every entry is its own button.
	ref array<string> NoQuestsBackTexts;
	ref array<string> NoQuestsLeaveTexts;

	//! Voice lines for the no-quests step, one picked at random.
	ref array<string> NoQuestsVoiceLineIDs;

	//! Per-screen "back to the conversation" buttons. Each renders only on the
	//! screen it is named for; empty falls back to the mod default (no button,
	//! except the no-quests step which keeps its built-in Back). The no-quests
	//! screen is still served by NoQuestsBackTexts above.
	ref array<string> QuestListBackTexts;
	ref array<string> OfferBackTexts;
	ref array<string> InProgressBackTexts;
	ref array<string> TurnInBackTexts;

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
