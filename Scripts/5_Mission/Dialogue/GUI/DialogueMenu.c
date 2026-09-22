#ifdef EXPANSIONMODQUESTS
modded class ExpansionQuestMenu
{
	//! Expansion subscribes every quest menu it builds to the quest-menu event
	//! in the constructor and only unsubscribes in the destructor -- and the
	//! subscription itself keeps the object alive, so a menu the player closed
	//! long ago still answers. Without this mod those stale answers only fill
	//! a hidden menu; with it, each one used to open another conversation
	//! window. They stacked up behind one another, each holding the player's
	//! inputs, with no way to reach the close button of the ones underneath.
	//! That is the 1.5.0 report of a window appearing by itself that could not
	//! be escaped without killing the game.
	//!
	//! So: only the menu actually on screen hands over, and only once.
	protected bool m_DialogueFW_HandedOver;

	override void SetQuests(string npcName = "", string defaultText = "", int questNPCID = -1, int questID = -1, int serverTime = 0)
	{
		if (m_DialogueFW_HandedOver || !DialogueFW_IsTheOpenMenu())
		{
			Print("[DialogueFramework] [DIAG] SetQuests() questNPCID=" + questNPCID + " -- not the quest menu on screen (or already handed over), leaving it to Expansion.");
			super.SetQuests(npcName, defaultText, questNPCID, questID, serverTime);
			return;
		}

		DialogueTree tree = DialogueManager.GetInstance().GetTreeForNPC(questNPCID);

		if (!tree)
		{
			Print("[DialogueFramework] [DIAG] SetQuests() questNPCID=" + questNPCID + " -- no custom tree found, falling through to stock menu.");
			super.SetQuests(npcName, defaultText, questNPCID, questID, serverTime);
			return;
		}

		DialogueManager.GetInstance().DumpTreeDiagnostic(tree, "INTERACTION questNPCID=" + questNPCID);

		m_DialogueFW_HandedOver = true;
		CloseMenu();

		//! At most one conversation waiting to open, whoever asked for it.
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(DialogueWindowLauncher.GetInstance().OpenDeferred);
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(DialogueWindowLauncher.GetInstance().OpenDeferred, 100, false, tree, questNPCID, npcName);
	}

	//! True only for the quest menu Expansion currently has open.
	protected bool DialogueFW_IsTheOpenMenu()
	{
		if (!GetDayZGame() || !GetDayZGame().GetExpansionGame())
			return false;

		ExpansionUIManager uiManager = GetDayZGame().GetExpansionGame().GetExpansionUIManager();
		if (!uiManager)
			return false;

		return ExpansionQuestMenu.Cast(uiManager.GetMenu()) == this;
	}
}
#endif
