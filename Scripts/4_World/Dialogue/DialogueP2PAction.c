#ifdef EXPANSIONMODP2PMARKET
//! Player-to-player traders never touch ExpansionMarketMenu -- they open
//! ExpansionP2PMarketMenu, a script-view menu created by name, so the trader
//! hook in DialogueMarketMenu.c cannot see them. The interception point is the
//! action itself: if the trader we are looking at has a conversation, open that
//! instead of the market, exactly as a normal trader does.
class DialogueP2PSession
{
	//! How far the player may be from the trader's spot, or any stop on its
	//! route, when asking for its market. Generous on purpose: an AI trader can
	//! be mid-route. It only has to stop a market being opened from across the
	//! map.
	static const float MARKET_REACH = 100.0;

	protected static ref DialogueP2PSession s_Instance;

	ref DialogueTree m_PendingTree;
	string m_PendingName;
	int m_PendingTraderID = -1;

	static DialogueP2PSession GetInstance()
	{
		if (!s_Instance)
			s_Instance = new DialogueP2PSession();
		return s_Instance;
	}

	void Set(DialogueTree tree, string name, int traderID)
	{
		m_PendingTree = tree;
		m_PendingName = name;
		m_PendingTraderID = traderID;
	}

	//! Client. Expansion's P2P market menu fills itself from data the server
	//! sends the moment the trader is used -- and that arrived while this
	//! conversation was in front of it, with no menu there to take it. Once
	//! OPEN_TRADER has opened the menu, ask for that data again.
	static void ClientRequestMarket(int traderID)
	{
		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		if (!player)
			return;

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(traderID);
		rpc.Send(player, DialogueFrameworkRPC.CLIENT_REQUEST_P2P_MARKET, true, null);
	}

	//! Server. Sends exactly what Expansion's own ExpansionActionOpenP2PMarketMenu
	//! sends when a P2P trader is used, after the same can-use check the action
	//! makes. The distance check stands in for the action's "looking at it".
	static void ServerOpenMarket(PlayerBase player, PlayerIdentity sender, int traderID)
	{
		ExpansionP2PMarketModule module = ExpansionP2PMarketModule.GetModuleInstance();
		if (!module)
			return;

		ExpansionP2PMarketTraderConfig traderConfig = module.GetP2PTraderConfigByID(traderID);
		if (!traderConfig)
		{
			Print("[DialogueFramework] [P2P] Market asked for P2P trader ID=" + traderID + ", which does not exist -- ignored.");
			return;
		}

		if (!module.CheckCanUseTrader(player, traderConfig))
		{
			Print("[DialogueFramework] [P2P] Market asked for P2P trader ID=" + traderID + ", which this player may not use -- ignored.");
			return;
		}

		if (!IsNearTrader(player.GetPosition(), traderConfig))
		{
			Print("[DialogueFramework] [P2P] Market asked for P2P trader ID=" + traderID + " from too far away -- ignored.");
			return;
		}

		ExpansionP2PMarketRequestData data = new ExpansionP2PMarketRequestData();
		data.m_Init = true;
		data.m_TraderID = traderID;

		module.AddTradingPlayer(traderID, sender.GetId());
		module.SendCategoryListingsData(traderID, sender);
		module.SendBasicListingData(data, sender);
	}

	protected static bool IsNearTrader(vector playerPos, ExpansionP2PMarketTraderConfig traderConfig)
	{
		if (vector.Distance(playerPos, traderConfig.m_Position) <= MARKET_REACH)
			return true;

		//! The field, not GetWaypoints(): that getter only exists when
		//! Expansion AI is loaded, and the P2P market runs without it.
		array<vector> waypoints = traderConfig.m_Waypoints;
		if (!waypoints)
			return false;

		foreach (vector waypoint : waypoints)
		{
			if (vector.Distance(playerPos, waypoint) <= MARKET_REACH)
				return true;
		}

		return false;
	}
}

modded class ExpansionActionOpenP2PMarketMenu
{
	override void OnExecuteClient(ActionData action_data)
	{
		int traderID = DialogueFW_P2PTraderID(action_data);
		if (traderID < 0)
		{
			super.OnExecuteClient(action_data);
			return;
		}

		DialogueTree tree = DialogueManager.GetInstance().GetTreeForP2PTrader(traderID);
		if (!tree)
		{
			Print("[DialogueFramework] [P2P] Trader opened -- P2P trader ID=" + traderID + ", no dialogue tree for it, opening the market normally.");
			super.OnExecuteClient(action_data);
			return;
		}

		Print("[DialogueFramework] [P2P] Trader opened -- P2P trader ID=" + traderID + ", using tree ID=" + tree.ID + ".");

		DialogueManager.GetInstance().DumpTreeDiagnostic(tree, "P2P " + traderID);
		DialogueP2PSession.GetInstance().Set(tree, DialogueFW_P2PTraderName(action_data), traderID);
		g_Game.GetUIManager().EnterScriptedMenu(MENU_DIALOGUEFW_P2P, NULL);
	}

	//! The synced P2P trader id of whatever is under the cursor, or -1.
	//! m_P2PTraderID is a registered net-sync variable, so this is valid on the
	//! client as well as the server.
	protected int DialogueFW_P2PTraderID(ActionData action_data)
	{
		Object targetObject;
		if (!Class.CastTo(targetObject, action_data.m_Target.GetParentOrObject()))
			return -1;

		ExpansionP2PMarketTraderStatic staticTrader = ExpansionP2PMarketTraderStatic.Cast(targetObject);
		if (staticTrader)
			return staticTrader.GetP2PTraderID();

		ExpansionP2PMarketTraderNPC npc = ExpansionP2PMarketTraderNPC.Cast(targetObject);
		if (npc)
			return npc.GetP2PTraderID();

	#ifdef ENFUSION_AI_PROJECT
		ExpansionP2PMarketTraderNPCAI npcAI = ExpansionP2PMarketTraderNPCAI.Cast(targetObject);
		if (npcAI)
			return npcAI.GetP2PTraderID();
	#endif

		return -1;
	}

	//! Expansion syncs the trader's display name into slot 0 of its netsync
	//! data, which is what the P2P menu itself shows.
	protected string DialogueFW_P2PTraderName(ActionData action_data)
	{
		Object targetObject;
		if (!Class.CastTo(targetObject, action_data.m_Target.GetParentOrObject()))
			return "";

		string traderName = "";

		ExpansionP2PMarketTraderStatic staticTrader = ExpansionP2PMarketTraderStatic.Cast(targetObject);
		if (staticTrader)
		{
			staticTrader.m_Expansion_NetsyncData.Get(0, traderName);
			return traderName;
		}

		ExpansionP2PMarketTraderNPC npc = ExpansionP2PMarketTraderNPC.Cast(targetObject);
		if (npc)
		{
			npc.m_Expansion_NetsyncData.Get(0, traderName);
			return traderName;
		}

	#ifdef ENFUSION_AI_PROJECT
		ExpansionP2PMarketTraderNPCAI npcAI = ExpansionP2PMarketTraderNPCAI.Cast(targetObject);
		if (npcAI)
		{
			npcAI.m_Expansion_NetsyncData.Get(0, traderName);
			return traderName;
		}
	#endif

		return traderName;
	}
}

modded class PlayerBase
{
	override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		super.OnRPC(sender, rpc_type, ctx);

		if (sender && rpc_type == DialogueFrameworkRPC.CLIENT_REQUEST_P2P_MARKET && GetGame().IsServer())
		{
			int traderID;
			if (!ctx.Read(traderID))
				return;

			DialogueP2PSession.ServerOpenMarket(this, sender, traderID);
		}
	}
}
#endif
