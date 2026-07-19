class DialogueQuestText
{
	int QuestID = -1;

	ref array<string> AcceptTexts;
	ref array<string> DeclineTexts;
	ref array<string> TurnInTexts;
	ref array<string> NotYetTexts;
	ref array<string> InProgressTexts;

	string RewardSelectText = "";

	void DialogueQuestText()
	{
		AcceptTexts = new array<string>;
		DeclineTexts = new array<string>;
		TurnInTexts = new array<string>;
		NotYetTexts = new array<string>;
		InProgressTexts = new array<string>;
	}

	void Sanitize()
	{
		if (!AcceptTexts)
			AcceptTexts = new array<string>;
		if (!DeclineTexts)
			DeclineTexts = new array<string>;
		if (!TurnInTexts)
			TurnInTexts = new array<string>;
		if (!NotYetTexts)
			NotYetTexts = new array<string>;
		if (!InProgressTexts)
			InProgressTexts = new array<string>;
	}

	void OnSend(ScriptRPC rpc)
	{
		rpc.Write(QuestID);
		rpc.Write(RewardSelectText);
		WriteList(rpc, AcceptTexts);
		WriteList(rpc, DeclineTexts);
		WriteList(rpc, TurnInTexts);
		WriteList(rpc, NotYetTexts);
		WriteList(rpc, InProgressTexts);
	}

	protected void WriteList(ScriptRPC rpc, array<string> source)
	{
		rpc.Write(source.Count());
		foreach (string entry : source)
			rpc.Write(entry);
	}

	bool OnRecieve(ParamsReadContext ctx)
	{
		if (!ctx.Read(QuestID)) return false;
		if (!ctx.Read(RewardSelectText)) return false;
		if (!ReadList(ctx, AcceptTexts)) return false;
		if (!ReadList(ctx, DeclineTexts)) return false;
		if (!ReadList(ctx, TurnInTexts)) return false;
		if (!ReadList(ctx, NotYetTexts)) return false;
		if (!ReadList(ctx, InProgressTexts)) return false;
		return true;
	}

	protected bool ReadList(ParamsReadContext ctx, array<string> target)
	{
		if (!target)
			return false;

		int count;
		if (!ctx.Read(count)) return false;

		target.Clear();
		for (int i = 0; i < count; i++)
		{
			string entry;
			if (!ctx.Read(entry)) return false;
			target.Insert(entry);
		}
		return true;
	}
}

class DialogueQuestTextFile
{
	ref array<ref DialogueQuestText> Quests;

	void DialogueQuestTextFile()
	{
		Quests = new array<ref DialogueQuestText>;
	}

	void Sanitize()
	{
		if (!Quests)
			Quests = new array<ref DialogueQuestText>;

		foreach (DialogueQuestText quest : Quests)
		{
			if (quest)
				quest.Sanitize();
		}
	}
}
