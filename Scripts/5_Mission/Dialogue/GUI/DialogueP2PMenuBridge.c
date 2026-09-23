#ifdef EXPANSIONMODP2PMARKET
//! OPEN_TRADER on a P2P conversation. The market must not open in the frame
//! the conversation closes: DialogueWindowMenu.OnHide gives the player their
//! controls back and hides the cursor, and it runs after Close() returns. A
//! market opened before that had its own lock undone -- no cursor, and the
//! player could walk around with it open. The ordinary trader path waits
//! 100 ms for the same reason (DialogueTraderSession.OpenMarketForCurrentTrader).
class DialogueP2PMarketOpener
{
	protected static ref DialogueP2PMarketOpener s_Instance;

	int m_TraderID = -1;

	static DialogueP2PMarketOpener GetInstance()
	{
		if (!s_Instance)
			s_Instance = new DialogueP2PMarketOpener();
		return s_Instance;
	}

	static void OpenAfterClose(int traderID)
	{
		GetInstance().m_TraderID = traderID;
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(GetInstance().OpenMarketDeferred, 100, false);
	}

	void OpenMarketDeferred()
	{
		//! Open the market first so it is listening, then ask the server for
		//! its opening data: the first copy was sent when the trader was used,
		//! while the conversation stood in front of the menu.
		GetDayZGame().GetExpansionGame().GetExpansionUIManager().CreateSVMenu("ExpansionP2PMarketMenu");
		DialogueP2PSession.ClientRequestMarket(m_TraderID);
	}
}

modded class MissionBase
{
	override UIScriptedMenu CreateScriptedMenu(int id)
	{
		if (id == MENU_DIALOGUEFW_P2P)
		{
			DialogueP2PSession session = DialogueP2PSession.GetInstance();

			//! No conversation waiting means this menu was entered without a
			//! trader behind it -- a stale or already-used session. Opening
			//! anyway gives an empty box that says nothing and offers nothing,
			//! while holding the player's controls.
			if (!session.m_PendingTree)
			{
				Print("[DialogueFramework] [P2P] [WARN] The P2P conversation window was asked for with no conversation waiting -- not opening it.");
				return super.CreateScriptedMenu(id);
			}

			//! NPC ID -1, the same as a market trader: a P2P trader has no
			//! quest-giver identity either, so SHOW_QUEST_LIST is guarded and
			//! quests are given by ID.
			DialogueWindowMenu menu = new DialogueWindowMenu(session.m_PendingTree, -1, session.m_PendingName);
			menu.DialogueFW_SetP2PTrader(true, session.m_PendingTraderID);

			//! A second attempt after a window came up empty is built from the
			//! plain layout. See DialogueWindowLauncher.RetryEnterDeferred.
			if (DialogueWindowLauncher.TakePlainLayout())
				menu.DialogueFW_UsePlainLayout();

			DialogueWindowLauncher.GetInstance().TrackWindow(menu, DialogueWindowLauncher.OPENED_AS_P2P);
			return menu;
		}

		return super.CreateScriptedMenu(id);
	}
}
#endif
