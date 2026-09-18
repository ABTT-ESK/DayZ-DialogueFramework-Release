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

			//! NPC ID -1, the same as a market trader: a P2P trader has no
			//! quest-giver identity either, so SHOW_QUEST_LIST is guarded and
			//! quests are given by ID.
			DialogueWindowMenu menu = new DialogueWindowMenu(session.m_PendingTree, -1, session.m_PendingName);
			menu.DialogueFW_SetP2PTrader(true, session.m_PendingTraderID);
			return menu;
		}

		return super.CreateScriptedMenu(id);
	}
}
#endif
