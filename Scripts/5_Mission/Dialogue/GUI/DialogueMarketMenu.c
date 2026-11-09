#ifdef EXPANSIONMODMARKET

class DialogueTraderSession
{
	static bool s_SkipDialogue;

	static ExpansionTraderObjectBase s_Trader;

	protected static ref DialogueTraderSession s_Instance;

	static DialogueTraderSession GetInstance()
	{
		if (!s_Instance)
			s_Instance = new DialogueTraderSession();
		return s_Instance;
	}

	//! 100 ms, not the next frame: the dialogue window's OnHide gives the
	//! player their controls back and hides the cursor after Close() returns,
	//! and a market opened before that loses both.
	static void OpenMarketForCurrentTrader()
	{
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(GetInstance().OpenMarketDeferred, 100, false);
	}

	void OpenMarketDeferred()
	{
		if (!s_Trader)
		{
			Print("[DialogueFramework] [TRADER] [ERROR] No trader in session -- cannot open the market.");
			return;
		}

		ExpansionMarketModule marketModule = ExpansionMarketModule.Cast(CF_ModuleCoreManager.Get(ExpansionMarketModule));
		if (!marketModule)
		{
			Print("[DialogueFramework] [TRADER] [ERROR] Could not get ExpansionMarketModule.");
			return;
		}

		s_SkipDialogue = true;

		if (!marketModule.OpenTraderMenu())
		{
			s_SkipDialogue = false;
			return;
		}

		ExpansionMarketTrader traderMarket = s_Trader.GetTraderMarket();
		if (!traderMarket)
			return;

		marketModule.RequestTraderItems(s_Trader, 0, traderMarket.m_StockOnly);
	}
}

modded class ExpansionMarketMenu
{
	protected bool m_DialogueFW_Checked;

	//! A conversation chosen while the market was being built, held for the
	//! next frame, when the market is closed and the conversation opened.
	protected ref DialogueTree m_DialogueFW_PendingTree;
	protected string m_DialogueFW_PendingName;
	protected Object m_DialogueFW_PendingSpeaker;

	//! Decided as the market is built, before it is ever drawn.
	//!
	//! The trader is already known on the client here:
	//! ExpansionMarketModule.RPC_LoadTraderData sets it just before
	//! OpenTraderMenu builds this menu, and CreateSVMenu shows it straight
	//! after. Waiting for SetTraderObject -- which only comes once the server
	//! has answered with the trader's items -- left the market on screen for
	//! about 40 ms before the conversation took over.
	//!
	//! A trader not known yet falls through to the old check in
	//! SetTraderObject below, which still works, flash and all.
	override void Show()
	{
		if (!m_DialogueFW_Checked && m_MarketModule)
		{
			ExpansionTraderObjectBase trader = m_MarketModule.GetTrader();
			if (trader)
			{
				m_DialogueFW_Checked = true;

				bool skip = DialogueTraderSession.s_SkipDialogue;
				DialogueTraderSession.s_SkipDialogue = false;

				if (!skip && DialogueFW_TakeOverBeforeShown(trader))
					return;
			}
		}

		super.Show();
	}

	override void SetTraderObject(ExpansionTraderObjectBase trader, bool complete)
	{
		if (!m_DialogueFW_Checked)
		{
			m_DialogueFW_Checked = true;

			bool skip = DialogueTraderSession.s_SkipDialogue;
			DialogueTraderSession.s_SkipDialogue = false;

			if (!skip && trader && DialogueFW_TryOpenDialogue(trader))
				return;
		}

		super.SetTraderObject(trader, complete);
	}

	//! The conversation for this trader, or null to open the market as
	//! normal. Fills in the name and body the conversation needs.
	protected DialogueTree DialogueFW_FindTree(ExpansionTraderObjectBase trader, out string traderName, out EntityAI traderEntity)
	{
		ExpansionMarketTrader traderMarket = trader.GetTraderMarket();
		if (!traderMarket)
			return null;

		traderEntity = trader.GetTraderEntity();
		string traderClass = "";
		vector traderPos = vector.Zero;
		if (traderEntity)
		{
			traderClass = traderEntity.GetType();
			traderPos = traderEntity.GetPosition();
		}

		Print("[DialogueFramework] [TRADER] Trader opened -- name='" + traderMarket.m_FileName + "' class='" + traderClass + "' position='" + traderPos[0] + " " + traderPos[1] + " " + traderPos[2] + "'");

		DialogueTree tree = DialogueManager.GetInstance().GetTreeForTraderEntity(traderMarket.m_FileName, traderClass, traderPos);
		if (!tree)
			tree = DialogueManager.GetInstance().GetTreeForTrader(traderMarket.DisplayName);

		if (!tree)
		{
			Print("[DialogueFramework] [TRADER] No dialogue tree for this trader -- opening the market normally.");
			return null;
		}

		traderName = trader.GetDisplayName();
		if (traderName == "")
			traderName = traderMarket.DisplayName;

		DialogueManager.GetInstance().DumpTreeDiagnostic(tree, "TRADER " + traderMarket.m_FileName);
		return tree;
	}

	//! The old check, from SetTraderObject: only reached when the trader was
	//! not known yet as the market was built. The market is on screen by
	//! now, so it is closed straight away.
	protected bool DialogueFW_TryOpenDialogue(ExpansionTraderObjectBase trader)
	{
		string traderName;
		EntityAI traderEntity;
		DialogueTree tree = DialogueFW_FindTree(trader, traderName, traderEntity);
		if (!tree)
			return false;

		DialogueTraderSession.s_Trader = trader;

		CloseMenu();

		//! At most one conversation waiting to open, whoever asked for it.
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(DialogueWindowLauncher.GetInstance().OpenDeferred);
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(DialogueWindowLauncher.GetInstance().OpenDeferred, 100, false, tree, -1, traderName, traderEntity);

		return true;
	}

	//! The conversation wins before the market is drawn: hide it now, close
	//! it next frame.
	protected bool DialogueFW_TakeOverBeforeShown(ExpansionTraderObjectBase trader)
	{
		string traderName;
		EntityAI traderEntity;
		DialogueTree tree = DialogueFW_FindTree(trader, traderName, traderEntity);
		if (!tree)
			return false;

		Print("[DialogueFramework] [TRADER] Conversation chosen before the market was drawn.");

		DialogueTraderSession.s_Trader = trader;
		m_DialogueFW_PendingTree = tree;
		m_DialogueFW_PendingName = traderName;
		m_DialogueFW_PendingSpeaker = traderEntity;

		//! Never shown: no controls locked, no blur, no focus. The root can
		//! be visible from the moment its layout is built, so it is hidden
		//! outright rather than just never shown.
		GetLayoutRoot().Show(false);

		//! Not closed here. This runs inside CreateSVMenu, and closing
		//! destroys the menu -- ScriptView.Destroy deletes it -- while
		//! CreateSVMenu still has it in hand.
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(DialogueFW_CloseUnshown, 0, false);
		return true;
	}

	//! Close the market that was never drawn, then open the conversation
	//! 100 ms later -- close first, then wait, the same order every other
	//! hand-off uses.
	protected void DialogueFW_CloseUnshown()
	{
		DialogueTree tree = m_DialogueFW_PendingTree;
		string traderName = m_DialogueFW_PendingName;
		Object traderEntity = m_DialogueFW_PendingSpeaker;

		//! Hide first. The manager's CloseMenu only calls Hide -- and with it
		//! the market's OnHide: ExitTrader, its server listeners taken off --
		//! for a menu that says it is visible, and this one never was.
		Hide();

		ExpansionUIManager expansionUI = GetDayZGame().GetExpansionGame().GetExpansionUIManager();
		if (expansionUI && expansionUI.GetMenu() == this)
			CloseMenu();
		else
			Print("[DialogueFramework] [TRADER] [WARN] Another menu took over before the market could be closed -- left hidden for Expansion to clear.");

		//! The menu is gone from here on; only the locals above are used.
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(DialogueWindowLauncher.GetInstance().OpenDeferred);
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(DialogueWindowLauncher.GetInstance().OpenDeferred, 100, false, tree, -1, traderName, traderEntity);
	}
}
#endif
