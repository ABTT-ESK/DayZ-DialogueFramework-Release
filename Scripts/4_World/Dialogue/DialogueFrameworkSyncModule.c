[CF_RegisterModule(DialogueFrameworkSyncModule)]
class DialogueFrameworkSyncModule : CF_ModuleWorld
{
	override bool IsServer()
	{
		return true;
	}

	override bool IsClient()
	{
		return true;
	}

	override void OnInit()
	{
		super.OnInit();

		Print("[DialogueFramework] [DIAG] DialogueFrameworkSyncModule.OnInit() fired. IsServer=" + GetGame().IsServer() + " IsClient=" + GetGame().IsClient() + " IsMultiplayer=" + GetGame().IsMultiplayer());

		EnableInvokeConnect();
		Expansion_EnableRPCManager();
		Expansion_RegisterClientRPC("RPC_SyncDialogueTrees");

		Print("[DialogueFramework] [DIAG] RPC_SyncDialogueTrees registered.");
	}

	override void OnInvokeConnect(Class sender, CF_EventArgs args)
	{
		super.OnInvokeConnect(sender, args);

		Print("[DialogueFramework] [DIAG] OnInvokeConnect() fired. IsServer=" + GetGame().IsServer() + " IsMultiplayer=" + GetGame().IsMultiplayer());

		if (!g_Game.IsServer() || !g_Game.IsMultiplayer())
		{
			Print("[DialogueFramework] [DIAG] OnInvokeConnect() -- not server/not multiplayer, skipping send.");
			return;
		}

		auto cArgs = CF_EventPlayerArgs.Cast(args);
		if (!cArgs || !cArgs.Identity)
		{
			Print("[DialogueFramework] [DIAG] OnInvokeConnect() -- could not cast args or no Identity, skipping send.");
			return;
		}

		SendTreesToClient(cArgs.Identity);
	}

	void SendTreesToClient(PlayerIdentity identity)
	{
		array<ref DialogueTree> trees = DialogueManager.GetInstance().GetAllTrees();

		auto rpc = Expansion_CreateRPC("RPC_SyncDialogueTrees");
		rpc.Write(trees.Count());
		foreach (DialogueTree tree : trees)
			tree.OnSend(rpc);

		DialogueManager.GetInstance().GetMenuConfig().OnSend(rpc);

		map<int, ref DialogueQuestText> questTexts = DialogueManager.GetInstance().GetAllQuestTexts();
		rpc.Write(questTexts.Count());
		for (int q = 0; q < questTexts.Count(); q++)
			questTexts.GetElement(q).OnSend(rpc);

		rpc.Expansion_Send(true, identity);

		Print("[DialogueFramework] Sent " + trees.Count() + " dialogue tree(s) to client UID=" + identity.GetId());
	}

	void RPC_SyncDialogueTrees(PlayerIdentity sender, Object target, ParamsReadContext ctx)
	{
		Print("[DialogueFramework] [DIAG] RPC_SyncDialogueTrees() handler invoked on client.");

		int treeCount;
		if (!ctx.Read(treeCount))
		{
			Print("[DialogueFramework] [ERROR] Could not read dialogue tree count from server.");
			return;
		}

		DialogueManager.GetInstance().ClearClientTrees();

		for (int i = 0; i < treeCount; i++)
		{
			DialogueTree tree = new DialogueTree();
			if (!tree.OnRecieve(ctx))
			{
				Print("[DialogueFramework] [ERROR] Failed to receive dialogue tree #" + i + " from server.");
				return;
			}

			tree.Sanitize();
			DialogueManager.GetInstance().DumpTreeDiagnostic(tree, "CLIENT-RECEIVE");
			DialogueManager.GetInstance().RegisterTree(tree);
		}

		DialogueMenuConfig menuConfig = new DialogueMenuConfig();
		if (menuConfig.OnRecieve(ctx))
		{
			menuConfig.Sanitize();
			DialogueManager.GetInstance().SetMenuConfig(menuConfig);
			Print("[DialogueFramework] Received menu config: position=" + menuConfig.Position);
		}
		else
		{
			Print("[DialogueFramework] [ERROR] Could not read menu config -- falling back to defaults.");
		}

		int questTextCount;
		if (ctx.Read(questTextCount))
		{
			DialogueManager.GetInstance().ClearQuestTexts();
			for (int q = 0; q < questTextCount; q++)
			{
				DialogueQuestText questText = new DialogueQuestText();
				if (!questText.OnRecieve(ctx))
				{
					Print("[DialogueFramework] [ERROR] Could not read quest text #" + q + ".");
					break;
				}
				DialogueManager.GetInstance().RegisterQuestText(questText);
			}
			Print("[DialogueFramework] Received per-quest text for " + questTextCount + " quest(s).");
		}

		Print("[DialogueFramework] Received " + treeCount + " dialogue tree(s) from server.");

		DialogueManager.GetInstance().AuditVoiceLines();
	}
}
