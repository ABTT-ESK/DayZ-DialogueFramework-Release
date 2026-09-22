class DialogueQuestText
{
	int QuestID = -1;

	ref array<string> AcceptTexts;
	ref array<string> DeclineTexts;
	ref array<string> TurnInTexts;
	ref array<string> NotYetTexts;
	ref array<string> InProgressTexts;

	ref array<string> QuestListTexts;
	ref array<string> NoQuestsTexts;
	ref array<string> NoQuestsBackTexts;
	ref array<string> NoQuestsLeaveTexts;
	ref array<string> QuestListBackTexts;
	ref array<string> OfferBackTexts;
	ref array<string> InProgressBackTexts;
	ref array<string> TurnInBackTexts;

	string RewardSelectText = "";

	//! What finishing this quest does to the player's reputation. The same
	//! ops a dialogue button uses -- Increase by, Decrease by, Set to -- so a
	//! quest can raise the giver's standing and lower a rival's at once.
	//!
	//! Deliberately NOT sent to clients: it is applied on the server when the
	//! quest is handed in, and a client has no use for it. Anyone adding it
	//! to OnSend must add it to OnRecieve in the same place, or every field
	//! after it arrives in the wrong slot.
	ref array<ref DialogueVarOp> RepOnComplete;

	void DialogueQuestText()
	{
		RepOnComplete = new array<ref DialogueVarOp>;
		AcceptTexts = new array<string>;
		DeclineTexts = new array<string>;
		TurnInTexts = new array<string>;
		NotYetTexts = new array<string>;
		InProgressTexts = new array<string>;
		QuestListTexts = new array<string>;
		NoQuestsTexts = new array<string>;
		NoQuestsBackTexts = new array<string>;
		NoQuestsLeaveTexts = new array<string>;
		QuestListBackTexts = new array<string>;
		OfferBackTexts = new array<string>;
		InProgressBackTexts = new array<string>;
		TurnInBackTexts = new array<string>;
	}

	void Sanitize()
	{
		if (!RepOnComplete)
			RepOnComplete = new array<ref DialogueVarOp>;

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
		if (!QuestListTexts)
			QuestListTexts = new array<string>;
		if (!NoQuestsTexts)
			NoQuestsTexts = new array<string>;
		if (!NoQuestsBackTexts)
			NoQuestsBackTexts = new array<string>;
		if (!NoQuestsLeaveTexts)
			NoQuestsLeaveTexts = new array<string>;
		if (!QuestListBackTexts)
			QuestListBackTexts = new array<string>;
		if (!OfferBackTexts)
			OfferBackTexts = new array<string>;
		if (!InProgressBackTexts)
			InProgressBackTexts = new array<string>;
		if (!TurnInBackTexts)
			TurnInBackTexts = new array<string>;
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
		WriteList(rpc, QuestListTexts);
		WriteList(rpc, NoQuestsTexts);
		WriteList(rpc, NoQuestsBackTexts);
		WriteList(rpc, NoQuestsLeaveTexts);
		WriteList(rpc, QuestListBackTexts);
		WriteList(rpc, OfferBackTexts);
		WriteList(rpc, InProgressBackTexts);
		WriteList(rpc, TurnInBackTexts);
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
		if (!ReadList(ctx, QuestListTexts)) return false;
		if (!ReadList(ctx, NoQuestsTexts)) return false;
		if (!ReadList(ctx, NoQuestsBackTexts)) return false;
		if (!ReadList(ctx, NoQuestsLeaveTexts)) return false;
		if (!ReadList(ctx, QuestListBackTexts)) return false;
		if (!ReadList(ctx, OfferBackTexts)) return false;
		if (!ReadList(ctx, InProgressBackTexts)) return false;
		if (!ReadList(ctx, TurnInBackTexts)) return false;
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
	static const int CURRENT_VERSION = 3;
	int ConfigVersion = 0;

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

	bool UpgradeFromOlderVersion()
	{
		if (ConfigVersion >= CURRENT_VERSION)
			return false;

		ConfigVersion = CURRENT_VERSION;
		return true;
	}
}
