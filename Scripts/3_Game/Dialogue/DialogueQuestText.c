class DialogueQuestText
{
	int QuestID = -1;

	ref array<string> AcceptTexts;
	ref array<string> DeclineTexts;
	ref array<string> TurnInTexts;
	ref array<string> NotYetTexts;
	ref array<string> InProgressTexts;

	//! Spoken above this NPC's quest list once this quest is completed and it
	//! is the furthest of theirs the player has finished. One picked at random.
	ref array<string> QuestListTexts;

	//! Shown when this NPC has nothing available AND this is the furthest quest
	//! of theirs the player has completed. One line picked at random.
	ref array<string> NoQuestsTexts;

	//! Buttons offered alongside NoQuestsTexts. Every entry is its own button.
	//! Back returns to the tree's RootNodeID, Leave ends the conversation.
	ref array<string> NoQuestsBackTexts;
	ref array<string> NoQuestsLeaveTexts;

	//! Per-screen "back to the conversation" buttons, overriding this quest's
	//! NPC's Quest talk defaults for the screens this quest drives. Empty falls
	//! back to the tree, then to the mod default. NoQuestsBackTexts above still
	//! serves the no-quests screen.
	ref array<string> QuestListBackTexts;
	ref array<string> OfferBackTexts;
	ref array<string> InProgressBackTexts;
	ref array<string> TurnInBackTexts;

	string RewardSelectText = "";

	void DialogueQuestText()
	{
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
	//! Bumped whenever new fields are added, so an existing file can have them
	//! written in on the next server start without touching what is already
	//! there. Version 1 added QuestListTexts and the NoQuests* fields.
	//! Version 2 added the per-screen *BackTexts fields.
	static const int CURRENT_VERSION = 2;
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

	//! Returns true when the file on disk is behind and should be rewritten.
	//! Nothing is changed here beyond the version stamp -- Sanitize() has
	//! already filled every missing field with its default, and every value
	//! the owner wrote was loaded before this runs.
	bool UpgradeFromOlderVersion()
	{
		if (ConfigVersion >= CURRENT_VERSION)
			return false;

		ConfigVersion = CURRENT_VERSION;
		return true;
	}
}
