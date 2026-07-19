class DialogueWindowLauncher
{
	protected static ref DialogueWindowLauncher s_Instance;

	protected ref DialogueWindowMenu m_ActiveWindow;

	static DialogueWindowLauncher GetInstance()
	{
		if (!s_Instance)
			s_Instance = new DialogueWindowLauncher();
		return s_Instance;
	}

	void OpenDeferred(DialogueTree tree, int npcID, string npcName)
	{
		Print("[DialogueFramework] [DIAG] DialogueWindowLauncher.OpenDeferred() firing.");
		m_ActiveWindow = new DialogueWindowMenu(tree, npcID, npcName);
		GetGame().GetUIManager().ShowScriptedMenu(m_ActiveWindow, null);
	}

	void ReleaseWindow(DialogueWindowMenu window)
	{
		if (m_ActiveWindow == window)
			m_ActiveWindow = null;
	}
}
