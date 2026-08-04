class DialogueVars
{
	protected static const string STATE_FOLDER = "$profile:\\DialogFramework\\PlayerState\\";

	protected static ref DialogueVars s_Instance;

	protected ref map<string, ref DialoguePlayerState> m_ServerStates;
	protected ref DialoguePlayerState m_ClientState;

	void DialogueVars()
	{
		m_ServerStates = new map<string, ref DialoguePlayerState>;
		m_ClientState = new DialoguePlayerState();
	}

	static DialogueVars GetInstance()
	{
		if (!s_Instance)
			s_Instance = new DialogueVars();
		return s_Instance;
	}

	DialoguePlayerState GetServerState(string uid)
	{
		DialoguePlayerState state;
		if (m_ServerStates.Find(uid, state))
			return state;

		state = new DialoguePlayerState();

		string path = STATE_FOLDER + uid + ".json";
		if (FileExist(path))
			JsonFileLoader<DialoguePlayerState>.JsonLoadFile(path, state);

		state.Sanitize();
		m_ServerStates.Insert(uid, state);
		return state;
	}

	void SaveServerState(string uid)
	{
		DialoguePlayerState state;
		if (!m_ServerStates.Find(uid, state))
			return;

		if (!FileExist(STATE_FOLDER))
			ExpansionStatic.MakeDirectoryRecursive(STATE_FOLDER);

		JsonFileLoader<DialoguePlayerState>.JsonSaveFile(STATE_FOLDER + uid + ".json", state);
	}

	void ApplyServer(string uid, array<ref DialogueVarOp> ops)
	{
		if (uid == "" || !ops || ops.Count() == 0)
			return;

		DialoguePlayerState state = GetServerState(uid);
		DialogueVarOpList.Apply(ops, state);
		SaveServerState(uid);
	}

	void SetClientState(DialoguePlayerState state)
	{
		if (state)
			m_ClientState = state;
	}

	DialoguePlayerState GetClientState()
	{
		if (!m_ClientState)
			m_ClientState = new DialoguePlayerState();
		return m_ClientState;
	}
}

modded class PlayerBase
{
	override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		super.OnRPC(sender, rpc_type, ctx);

		if (sender && rpc_type == DialogueFrameworkRPC.CLIENT_APPLY_VARS && GetGame().IsServer())
		{
			array<ref DialogueVarOp> ops = new array<ref DialogueVarOp>;
			if (!DialogueVarOpList.Read(ctx, ops))
				return;

			DialogueVars.GetInstance().ApplyServer(sender.GetId(), ops);
			DialogueFrameworkSyncModule.DialogueFW_SendVars(sender);
		}
	}
}
