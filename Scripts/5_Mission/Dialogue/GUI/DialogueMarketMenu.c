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

	protected bool DialogueFW_TryOpenDialogue(ExpansionTraderObjectBase trader)
	{
		ExpansionMarketTrader traderMarket = trader.GetTraderMarket();
		if (!traderMarket)
			return false;

		EntityAI traderEntity = trader.GetTraderEntity();
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
			return false;
		}

		DialogueTraderSession.s_Trader = trader;

		string traderName = trader.GetDisplayName();
		if (traderName == "")
			traderName = traderMarket.DisplayName;

		DialogueManager.GetInstance().DumpTreeDiagnostic(tree, "TRADER " + traderMarket.m_FileName);

		CloseMenu();

		GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(DialogueWindowLauncher.GetInstance().OpenDeferred, 100, false, tree, -1, traderName);

		return true;
	}
}
#endif
