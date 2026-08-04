#ifdef EXPANSIONMODAI
modded class MissionBase
{
	override UIScriptedMenu CreateScriptedMenu(int id)
	{
		if (id == MENU_DIALOGUEFW_AI)
		{
			DialogueAISession session = DialogueAISession.GetInstance();

			DialogueWindowMenu menu = new DialogueWindowMenu(session.m_PendingTree, -1, session.m_PendingName);
			menu.DialogueFW_SetTargetAI(session.m_PendingAI);
			return menu;
		}

		return super.CreateScriptedMenu(id);
	}
}
#endif
