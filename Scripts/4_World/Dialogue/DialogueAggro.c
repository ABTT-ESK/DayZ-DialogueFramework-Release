#ifdef EXPANSIONMODAI
class DialogueAISettings
{
	int ResetOnDeath = 1;
	int ResetOnWeaponStowed = 1;
	int ResetOnLeaveArea = 1;
	float LeaveAreaDistance = 60.0;
	int ResetOnSurrender = 1;
	int PersistentAggroThreshold = 0;
	string PersistenceMode = "FACTION";
	float CheckInterval = 2.0;
}

class DialogueAggroOverride
{
	int Threshold = -1;
	string Mode = "";
}

class DialogueAggro
{
	protected static const string FOLDER = "$profile:\\DialogFramework\\";
	protected static const string SETTINGS_PATH = "$profile:\\DialogFramework\\AISettings.json";

	protected static ref DialogueAISettings s_Settings;
	protected static ref map<int, ref DialogueAggroOverride> s_Overrides;

	static void Init()
	{
		Load();
	}

	static void RegisterPatrolOverride(int patrolID, int threshold, string mode)
	{
		if (patrolID <= 0)
			return;
		if (threshold < 0 && mode == "")
			return;

		if (!s_Overrides)
			s_Overrides = new map<int, ref DialogueAggroOverride>;

		DialogueAggroOverride ov = new DialogueAggroOverride();
		ov.Threshold = threshold;
		ov.Mode = mode;
		s_Overrides.Insert(patrolID, ov);
	}

	protected static int EffectiveThreshold(int patrolID)
	{
		DialogueAggroOverride ov;
		if (s_Overrides && s_Overrides.Find(patrolID, ov) && ov.Threshold >= 0)
			return ov.Threshold;
		return Settings().PersistentAggroThreshold;
	}

	protected static string EffectiveMode(int patrolID)
	{
		DialogueAggroOverride ov;
		if (s_Overrides && s_Overrides.Find(patrolID, ov) && ov.Mode != "")
			return ov.Mode;
		return Settings().PersistenceMode;
	}

	static DialogueAISettings Settings()
	{
		if (!s_Settings)
			Load();
		return s_Settings;
	}

	protected static void Load()
	{
		s_Settings = new DialogueAISettings();

		if (FileExist(SETTINGS_PATH))
		{
			JsonFileLoader<DialogueAISettings>.JsonLoadFile(SETTINGS_PATH, s_Settings);
			return;
		}

		if (GetGame().IsServer())
		{
			if (!FileExist(FOLDER))
				ExpansionStatic.MakeDirectoryRecursive(FOLDER);
			JsonFileLoader<DialogueAISettings>.JsonSaveFile(SETTINGS_PATH, s_Settings);
			Print("[DialogueFramework] [AI] Wrote default AISettings.json");
		}
	}

	static void RegisterAggro(eAIBase ai, PlayerBase player)
	{
		if (!ai || !player || !player.GetIdentity())
			return;

		string uid = player.GetIdentity().GetId();
		DialoguePlayerState state = DialogueVars.GetInstance().GetServerState(uid);

		string factionKey = "__aggro_f_" + ai.DialogueFW_FactionName();
		string patrolKey = "__aggro_p_" + ai.DialogueFW_GetPatrolID();

		state.Set(factionKey, state.Get(factionKey) + 1);
		state.Set(patrolKey, state.Get(patrolKey) + 1);
		DialogueVars.GetInstance().SaveServerState(uid);
	}

	static int AnyAggroCount(string uid, string faction, int patrolID)
	{
		DialoguePlayerState state = DialogueVars.GetInstance().GetServerState(uid);
		int factionCount = state.Get("__aggro_f_" + faction);
		int patrolCount = state.Get("__aggro_p_" + patrolID);
		if (patrolCount > factionCount)
			return patrolCount;
		return factionCount;
	}

	static bool IsPermanent(string uid, string faction, int patrolID)
	{
		int threshold = EffectiveThreshold(patrolID);
		if (threshold <= 0)
			return false;

		DialoguePlayerState state = DialogueVars.GetInstance().GetServerState(uid);
		int factionCount = state.Get("__aggro_f_" + faction);
		int patrolCount = state.Get("__aggro_p_" + patrolID);

		string mode = EffectiveMode(patrolID);
		if (mode == "PATROL")
			return patrolCount >= threshold;
		if (mode == "BOTH")
			return factionCount >= threshold || patrolCount >= threshold;
		return factionCount >= threshold;
	}

	static bool PlayerIsCalm(eAIBase ai, PlayerBase player)
	{
		DialogueAISettings s = Settings();

		if (s.ResetOnWeaponStowed)
		{
			Weapon_Base weapon;
			if (!Class.CastTo(weapon, player.GetItemInHands()))
				return true;
		}

		if (s.ResetOnSurrender && DialogueFW_IsSurrendering(player))
			return true;

		if (s.ResetOnLeaveArea)
		{
			if (vector.Distance(ai.GetPosition(), player.GetPosition()) > s.LeaveAreaDistance)
				return true;
		}

		return false;
	}

	static bool DialogueFW_IsSurrendering(PlayerBase player)
	{
		EmoteManager emotes = player.GetEmoteManager();
		if (!emotes)
			return false;
		return emotes.Expansion_GetCurrentGesture() == EmoteConstants.ID_EMOTE_SURRENDER;
	}
}
#endif
