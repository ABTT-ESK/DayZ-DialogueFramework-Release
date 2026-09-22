#ifdef EXPANSIONMODQUESTS
//! Reputation earned or lost by finishing a quest.
//!
//! Hooked on the quest's own turn-in rather than the module's CompleteQuest:
//! CompleteQuest carries on past a failed turn-in to report the failure, so
//! hooking it would pay a player for a quest they did not actually hand in.
//! OnQuestTurnIn returns false in exactly that case, so what it hands back
//! is the signal worth acting on.
//!
//! Server only. The ops live in the quest's QuestText entry, which the server
//! has in full; nothing about this needs to reach a client except the
//! resulting reputation, which is sent the same way a dialogue button's
//! change is.
modded class ExpansionQuest
{
	override bool OnQuestTurnIn(string playerUID, ExpansionQuestRewardConfig reward = null, int selectedObjItemIndex = -1)
	{
		if (!super.OnQuestTurnIn(playerUID, reward, selectedObjItemIndex))
			return false;

		DialogueFW_PayReputation(playerUID);
		return true;
	}

	protected void DialogueFW_PayReputation(string playerUID)
	{
		if (playerUID == "" || !GetGame().IsServer())
			return;

		ExpansionQuestConfig config = GetQuestConfig();
		if (!config)
			return;

		DialogueManager manager = DialogueManager.GetInstance();
		if (!manager)
			return;

		DialogueQuestText text = manager.GetQuestText(config.GetID());
		if (!text || !text.RepOnComplete || text.RepOnComplete.Count() == 0)
			return;

		DialogueVars.GetInstance().ApplyServer(playerUID, text.RepOnComplete);

		//! Straight back to the player, so the marker beside the NPC's name
		//! is right the moment they turn round and talk to them again.
		PlayerBase player = PlayerBase.GetPlayerByUID(playerUID);
		if (player && player.GetIdentity())
			DialogueFrameworkSyncModule.DialogueFW_SendVars(player.GetIdentity());

		string changed = "";
		foreach (DialogueVarOp op : text.RepOnComplete)
		{
			if (!op)
				continue;
			changed = changed + " " + op.Name + " " + op.Op + " " + op.Value.ToString();
		}

		Print("[DialogueFramework] [QUEST] Quest " + config.GetID() + " handed in by " + playerUID + " --" + changed);
	}
}
#endif
