#ifdef EXPANSIONMODAI
modded class MissionBase
{
	override UIScriptedMenu CreateScriptedMenu(int id)
	{
		if (id == MENU_DIALOGUEFW_AI)
		{
			DialogueAISession session = DialogueAISession.GetInstance();

			//! No conversation waiting means this menu was entered without an
			//! AI behind it -- a stale or already-used session. Opening anyway
			//! gives an empty box that says nothing and offers nothing, while
			//! holding the player's controls.
			if (!session.m_PendingTree)
			{
				Print("[DialogueFramework] [AI] [WARN] The AI conversation window was asked for with no conversation waiting -- not opening it.");
				return super.CreateScriptedMenu(id);
			}

			DialogueWindowMenu menu = new DialogueWindowMenu(session.m_PendingTree, -1, session.m_PendingName);
			menu.DialogueFW_SetTargetAI(session.m_PendingAI);
			DialogueWindowLauncher.GetInstance().TrackWindow(menu);
			return menu;
		}

		return super.CreateScriptedMenu(id);
	}
}
#endif
