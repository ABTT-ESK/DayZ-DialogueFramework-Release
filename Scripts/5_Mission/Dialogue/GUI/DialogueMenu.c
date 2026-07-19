#ifdef EXPANSIONMODQUESTS
modded class ExpansionQuestMenu
{
	override void SetQuests(string npcName = "", string defaultText = "", int questNPCID = -1, int questID = -1, int serverTime = 0)
	{
		DialogueTree tree = DialogueManager.GetInstance().GetTreeForNPC(questNPCID);

		if (!tree)
		{
			Print("[DialogueFramework] [DIAG] SetQuests() questNPCID=" + questNPCID + " -- no custom tree found, falling through to stock menu.");
			super.SetQuests(npcName, defaultText, questNPCID, questID, serverTime);
			return;
		}

		DialogueManager.GetInstance().DumpTreeDiagnostic(tree, "INTERACTION questNPCID=" + questNPCID);

		CloseMenu();

		GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(DialogueWindowLauncher.GetInstance().OpenDeferred, 100, false, tree, questNPCID, npcName);
	}
}
#endif
