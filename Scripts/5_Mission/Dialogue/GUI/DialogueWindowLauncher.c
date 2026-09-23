class DialogueWindowLauncher
{
	protected static ref DialogueWindowLauncher s_Instance;

	protected ref DialogueWindowMenu m_ActiveWindow;

	//! The conversation last asked for, and whether it has already had its one
	//! retry. See DialogueFW_BuildFailed.
	protected ref DialogueTree m_LastTree;
	protected int m_LastNPCID = -1;
	protected string m_LastNPCName = "";
	protected bool m_RetryUsed;
	protected bool m_Retrying;

	//! The conversation on screen, and how it was opened -- a failed build has
	//! to be tried again the same way it was asked for. The AI and P2P
	//! conversations are entered as the mission's own menus rather than shown
	//! by this launcher, and they rebuild from their own sessions.
	protected DialogueWindowMenu m_RetryOwner;
	protected int m_RetryMode;

	static const int OPENED_BY_LAUNCHER = 0;
	static const int OPENED_AS_AI = 1;
	static const int OPENED_AS_P2P = 2;

	//! TEST BUILDS ONLY -- must be false in anything that ships.
	//!
	//! On, the launcher stops refusing to open on top of another menu. That
	//! refusal is what keeps the client out of the state where the game hands
	//! back layouts with nothing inside them, so with it off the state can be
	//! reached on purpose and the rebuild below can actually be exercised.
	//!
	//! The cost while it is on is every bug the refusal prevents: a pause
	//! screen or an inventory that cannot be closed, and a game that has to be
	//! killed. `tools/preflight.py` shouts about this being on.
	static const bool TEST_ALLOW_STACKING = false;

	//! True for the instant a retry re-enters one of the mission's menus. The
	//! bridge that builds the window reads it and clears it, so the second
	//! attempt is built from the plain layout like any other.
	protected static bool s_PlainLayoutNext;

	static bool TakePlainLayout()
	{
		bool wanted = s_PlainLayoutNext;
		s_PlainLayoutNext = false;
		return wanted;
	}

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

		//! Never on top of something else. The game tracks its menus as a chain
		//! -- each one knows its parent, and "is the pause menu open?", "close
		//! the pause menu" and "close every menu" all walk that chain down from
		//! whatever is on top. This window is put up with no parent, so it ends
		//! the chain: anything already on screen underneath it stops counting
		//! as open.
		//!
		//! With the pause menu under it the game stops believing it is paused,
		//! so Escape never closes the pause screen, every press builds another
		//! one on top, and leaving the server leaves them all behind. The only
		//! way out is killing the game. That is the 1.5.0 report of a window
		//! that could not be escaped, and it survived 1.6.0.
		//!
		//! DayZ opens the inventory exactly the way this window opens -- shown
		//! with no parent -- so the inventory goes the same way as the pause
		//! screen: the mission stops finding it, its input restrictions are
		//! never lifted, and it cannot be closed. Reproduced 2026-09-22.
		//!
		//! Expansion applies the same rule to its own quest window, which only
		//! opens when nothing else is on screen. The conversation is dropped
		//! rather than queued: the player is doing something else, and they can
		//! always talk again.
		UIScriptedMenu onScreen = GetGame().GetUIManager().GetMenu();
		if (!TEST_ALLOW_STACKING && onScreen && !DialogueWindowMenu.Cast(onScreen))
		{
			Print("[DialogueFramework] [UI] [WARN] Another menu is on screen -- not opening the conversation on top of it.");
			return;
		}

	#ifdef EXPANSIONUI
		//! Expansion's own menus -- its quest window, the market, the book --
		//! are script views in Expansion's manager, not DayZ menus, so the
		//! check above cannot see them. They still hold the cursor and the
		//! player's inputs, and a conversation on top of one leaves it
		//! unreachable underneath. Expansion's quest window can put itself on
		//! screen a second after a quest is accepted, which is the moment a
		//! late talk answer tends to arrive.
		//!
		//! Every hand-off of ours closes Expansion's menu before asking for a
		//! conversation, and that close takes effect at once, so this only
		//! catches a menu that opened on its own since.
		if (GetDayZGame() && GetDayZGame().GetExpansionGame())
		{
			ExpansionUIManager expansionUI = GetDayZGame().GetExpansionGame().GetExpansionUIManager();
			if (!TEST_ALLOW_STACKING && expansionUI && expansionUI.GetMenu())
			{
				Print("[DialogueFramework] [UI] [WARN] An Expansion menu is on screen -- not opening the conversation on top of it.");
				return;
			}
		}
	#endif

		//! One conversation at a time. A second window sits behind the first,
		//! holding the player's inputs, and its close button can't be reached
		//! -- the 1.5.0 report of a window that could not be escaped.
		CloseOpenWindow("a new conversation is opening");

		//! Kept so a window that comes up empty can be tried once more. A
		//! fresh request forgets any earlier attempt.
		if (!m_Retrying)
		{
			m_LastTree = tree;
			m_LastNPCID = npcID;
			m_LastNPCName = npcName;
			m_RetryUsed = false;
		}

		m_ActiveWindow = new DialogueWindowMenu(tree, npcID, npcName);
		m_RetryOwner = m_ActiveWindow;
		m_RetryMode = OPENED_BY_LAUNCHER;

		if (m_Retrying)
			m_ActiveWindow.DialogueFW_UsePlainLayout();

		GetGame().GetUIManager().ShowScriptedMenu(m_ActiveWindow, null);
	}

	//! A window whose layout came back with nothing inside it says so here
	//! rather than simply giving up. Returns true when another attempt is on
	//! its way, so the window knows whether to tell the player anything.
	//!
	//! The second attempt is built from the plain layout instead of the
	//! player's font variant: nothing in the engine resets whatever is broken,
	//! and a different file is the only thing we can change. A conversation in
	//! the default font beats no conversation at all. If that one comes up
	//! empty too, the client is past saving until the game is restarted, and
	//! the player is told so.
	bool DialogueFW_BuildFailed(DialogueWindowMenu window)
	{
		//! Only a window this launcher opened may be reopened from here. The
		//! AI and P2P conversations are built by the mission from their own
		//! sessions, and the tree remembered here is not theirs -- retrying
		//! one of those would put a different character's conversation on
		//! screen. They get the message instead.
		bool ours = false;
		if (window && window == m_RetryOwner)
			ours = true;

		//! The window is not let go of here either -- it is still closing, and
		//! it has to outlive its own OnHide (see CloseDeferred). The retry
		//! clears it on the way past.
		m_RetryOwner = null;

		if (!ours || m_RetryUsed)
			return false;

		//! The launcher's own conversations rebuild from the tree it kept; the
		//! mission's two rebuild from their sessions, which still hold theirs.
		if (m_RetryMode == OPENED_BY_LAUNCHER && !m_LastTree)
			return false;

		m_RetryUsed = true;

		Print("[DialogueFramework] [UI] Trying the conversation once more with the plain layout.");

		//! 100 ms, the same wait every other hand-off uses: the empty window
		//! closing gives the player their controls back in its own time, and
		//! the new one must not open underneath that.
		ScriptCallQueue guiQueue = GetGame().GetCallQueue(CALL_CATEGORY_GUI);

		if (m_RetryMode == OPENED_BY_LAUNCHER)
		{
			guiQueue.Remove(RetryDeferred);
			guiQueue.CallLater(RetryDeferred, 100, false);
		}
		else
		{
			guiQueue.Remove(RetryEnterDeferred);
			guiQueue.CallLater(RetryEnterDeferred, 100, false);
		}

		return true;
	}

	//! The AI patrol and P2P trader conversations are entered as the mission's
	//! own menus, so they are asked for again the same way. Their sessions
	//! still hold the conversation, the character and the trader id, which is
	//! what the bridge builds the window from.
	void RetryEnterDeferred()
	{
		//! Same rule as any other open: never on top of someone else's menu.
		UIScriptedMenu onScreen = GetGame().GetUIManager().GetMenu();
		if (!TEST_ALLOW_STACKING && onScreen && !DialogueWindowMenu.Cast(onScreen))
		{
			Print("[DialogueFramework] [UI] [WARN] Another menu is on screen -- not trying the conversation again on top of it.");
			return;
		}

		m_Retrying = true;
		s_PlainLayoutNext = true;

	#ifdef EXPANSIONMODAI
		if (m_RetryMode == OPENED_AS_AI)
			GetGame().GetUIManager().EnterScriptedMenu(MENU_DIALOGUEFW_AI, NULL);
	#endif

	#ifdef EXPANSIONMODP2PMARKET
		if (m_RetryMode == OPENED_AS_P2P)
			GetGame().GetUIManager().EnterScriptedMenu(MENU_DIALOGUEFW_P2P, NULL);
	#endif

		//! Cleared whatever happened: if the menu never reached the bridge,
		//! the flag must not sit here waiting for the next conversation.
		s_PlainLayoutNext = false;
		m_Retrying = false;
	}

	//! Straight back through OpenDeferred, so the retry obeys every rule a
	//! first attempt does -- above all, never opening on top of another menu.
	void RetryDeferred()
	{
		m_Retrying = true;
		OpenDeferred(m_LastTree, m_LastNPCID, m_LastNPCName);
		m_Retrying = false;
	}

	//! The AI and P2P windows are built by the mission rather than here, but
	//! they are still the one conversation on screen, so they are tracked the
	//! same way. Those two enter their menu rather than showing it on top, so
	//! the engine is already replacing whatever was open -- this only stops
	//! the launcher pointing at the old one.
	void TrackWindow(DialogueWindowMenu window, int openedAs = 0)
	{
		if (!window || m_ActiveWindow == window)
			return;

		m_ActiveWindow = window;
		m_RetryOwner = window;
		m_RetryMode = openedAs;

		//! A conversation the player asked for starts with its retry unspent.
		//! The one the retry itself puts up must not hand out another.
		if (!m_Retrying)
			m_RetryUsed = false;
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

	//! Nothing lets go of a window on its way out any more -- a closed window
	//! has to outlive its own OnHide, or the engine runs it against an object
	//! that has already been collected. CloseOpenWindow and the next
	//! conversation are what clear this. See DialogueWindowMenu.CloseDeferred.
}
