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

		if (!tree)
		{
			Print("[DialogueFramework] [DIAG] [WARN] OpenDeferred() with no conversation -- nothing opened.");
			return;
		}

		//! One conversation at a time. A second window sits behind the first,
		//! holding the player's inputs, and its close button can't be reached
		//! -- the 1.5.0 report of a window that could not be escaped.
		CloseOpenWindow("a new conversation is opening");

		m_ActiveWindow = new DialogueWindowMenu(tree, npcID, npcName);
		GetGame().GetUIManager().ShowScriptedMenu(m_ActiveWindow, null);
	}

	//! The AI and P2P windows are built by the mission rather than here, but
	//! they are still the one conversation on screen, so they are tracked the
	//! same way. Those two enter their menu rather than showing it on top, so
	//! the engine is already replacing whatever was open -- this only stops
	//! the launcher pointing at the old one.
	void TrackWindow(DialogueWindowMenu window)
	{
		if (!window || m_ActiveWindow == window)
			return;

		m_ActiveWindow = window;
	}

	void CloseOpenWindow(string why)
	{
		if (!m_ActiveWindow)
			return;

		if (!m_ActiveWindow.DialogueFW_IsClosed())
		{
			Print("[DialogueFramework] [DIAG] Closing the conversation already on screen -- " + why + ".");
			m_ActiveWindow.Close();
		}

		m_ActiveWindow = null;
	}

	void ReleaseWindow(DialogueWindowMenu window)
	{
		if (m_ActiveWindow == window)
			m_ActiveWindow = null;
	}
}
