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
		Expansion_RegisterClientRPC("RPC_SyncPlayerVars");
		Expansion_RegisterClientRPC("RPC_SyncDialogueLoc");
		Expansion_RegisterServerRPC("RPC_RequestDialogueLoc");

		Print("[DialogueFramework] [DIAG] RPC_SyncDialogueTrees registered.");
	}

	static void DialogueFW_SendVars(PlayerIdentity identity)
	{
		if (!identity || !g_Game.IsServer())
			return;

		DialogueFrameworkSyncModule mod = DialogueFrameworkSyncModule.Cast(
			CF_ModuleCoreManager.Get(DialogueFrameworkSyncModule));
		if (mod)
			mod.SendVarsToClient(identity);
	}

	void SendVarsToClient(PlayerIdentity identity)
	{
		DialoguePlayerState state = DialogueVars.GetInstance().GetServerState(identity.GetId());

		auto rpc = Expansion_CreateRPC("RPC_SyncPlayerVars");
		state.OnSend(rpc);
		rpc.Expansion_Send(true, identity);
	}

	void RPC_SyncPlayerVars(PlayerIdentity sender, Object target, ParamsReadContext ctx)
	{
		DialoguePlayerState state = new DialoguePlayerState();
		if (state.OnRecieve(ctx))
		{
			DialogueVars.GetInstance().SetClientState(state);
			Print("[DialogueFramework] Received " + state.Names.Count() + " dialogue variable(s) from server.");
		}
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

		DialogueVars.GetInstance().GetServerState(identity.GetId()).OnSend(rpc);

		array<string> languages = DialogueManager.GetInstance().GetLocalizationLanguages();
		rpc.Write(languages.Count());
		foreach (string language : languages)
			rpc.Write(language);

		rpc.Expansion_Send(true, identity);

		Print("[DialogueFramework] Sent " + trees.Count() + " dialogue tree(s) and " + languages.Count() + " translation language(s) to client UID=" + identity.GetId());
	}

	static void DialogueFW_RequestLanguage(string language)
	{
		if (!g_Game.IsClient())
			return;

		DialogueFrameworkSyncModule mod = DialogueFrameworkSyncModule.Cast(
			CF_ModuleCoreManager.Get(DialogueFrameworkSyncModule));
		if (mod)
			mod.RequestLanguageFromServer(language);
	}

	void RequestLanguageFromServer(string language)
	{
		string wanted = DialogueFWLanguages.Normalize(language);

		DialogueLoc loc = DialogueLoc.GetInstance();

		if (!loc.ServerHasLanguage(wanted))
		{
			loc.ClearBundle();
			loc.SetRequestedLanguage("");
			Print("[DialogueFramework] [LOC] No translation on this server for '" + wanted + "' -- showing the dialogue as written.");
			return;
		}

		if (loc.ActiveLanguage() == wanted)
			return;

		if (loc.UseCachedBundle(wanted))
		{
			Print("[DialogueFramework] [LOC] Switched to the already-downloaded '" + wanted + "' translation.");
			return;
		}

		loc.SetRequestedLanguage(wanted);

		auto rpc = Expansion_CreateRPC("RPC_RequestDialogueLoc");
		rpc.Write(wanted);
		rpc.Expansion_Send(true);

		Print("[DialogueFramework] [LOC] Requested the '" + wanted + "' translation from the server.");
	}

	void RPC_RequestDialogueLoc(PlayerIdentity sender, Object target, ParamsReadContext ctx)
	{
		if (!g_Game.IsServer() || !sender)
			return;

		string wanted;
		if (!ctx.Read(wanted))
			return;

		DialogueLocBundle bundle = DialogueManager.GetInstance().GetLocBundle(wanted);
		if (!bundle)
		{
			Print("[DialogueFramework] [LOC] Client UID=" + sender.GetId() + " asked for '" + wanted + "' but the server has no such translation.");
			return;
		}

		auto rpc = Expansion_CreateRPC("RPC_SyncDialogueLoc");
		bundle.OnSend(rpc);
		rpc.Expansion_Send(true, sender);

		Print("[DialogueFramework] [LOC] Sent the '" + bundle.Language + "' translation (" + bundle.EntryCount() + " line(s)) to UID=" + sender.GetId());
	}

	void RPC_SyncDialogueLoc(PlayerIdentity sender, Object target, ParamsReadContext ctx)
	{
		DialogueLocBundle bundle = new DialogueLocBundle();
		if (!bundle.OnRecieve(ctx))
		{
			Print("[DialogueFramework] [LOC] [ERROR] Could not read the translation sent by the server.");
			return;
		}

		DialogueLoc.GetInstance().SetBundle(bundle);
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

		DialoguePlayerState varState = new DialoguePlayerState();
		if (varState.OnRecieve(ctx))
		{
			DialogueVars.GetInstance().SetClientState(varState);
			Print("[DialogueFramework] Received " + varState.Names.Count() + " dialogue variable(s) from server.");
		}

		int languageCount;
		if (ctx.Read(languageCount))
		{
			array<string> languages = new array<string>;
			for (int l = 0; l < languageCount; l++)
			{
				string language;
				if (!ctx.Read(language))
					break;
				languages.Insert(language);
			}

			DialogueLoc.GetInstance().SetServerLanguages(languages);
			Print("[DialogueFramework] [LOC] Server offers " + languages.Count() + " translation language(s).");

			RequestLanguageFromServer(DialogueLoc.GetInstance().EffectiveLanguage());
		}

		Print("[DialogueFramework] Received " + treeCount + " dialogue tree(s) from server.");

		DialogueManager.GetInstance().AuditVoiceLines();
	}
}
