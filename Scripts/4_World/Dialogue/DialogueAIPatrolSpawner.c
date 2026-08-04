#ifdef EXPANSIONMODAI
class DialogueFW_Patrol : eAIDynamicPatrol
{
	protected int m_DFW_NextSubID = 1;

	override void Spawn()
	{
		m_DFW_NextSubID = 1;
		super.Spawn();
	}

	override void SetupAI(eAIBase ai)
	{
		super.SetupAI(ai);

		DialogueFW_PatrolConfig cfg = DialogueFW_PatrolConfig.Cast(m_Config);
		if (cfg && ai)
		{
			ai.DialogueFW_SetPatrolID(cfg.DialogueID, m_DFW_NextSubID);
			m_DFW_NextSubID++;
		}
	}
}

class DialogueFW_AIPatrolSpawner
{
	protected static const string PATROL_FILE = "$profile:\\DialogFramework\\AIPatrol\\AIPatrols.json";

	static void SpawnAll()
	{
		if (!GetGame().IsServer())
			return;

		if (!FileExist(PATROL_FILE))
		{
			Print("[DialogueFramework] [AI] No AIPatrols.json at " + PATROL_FILE + " -- no talkable patrols spawned (optional).");
			return;
		}

		if (!eAIDynamicPatrol.InitSettings())
		{
			Print("[DialogueFramework] [AI] Expansion AI patrol settings not ready -- cannot spawn talkable patrols.");
			return;
		}

		DialogueFW_PatrolFile file = new DialogueFW_PatrolFile();
		JsonFileLoader<DialogueFW_PatrolFile>.JsonLoadFile(PATROL_FILE, file);

		if (!file || !file.Patrols)
		{
			Print("[DialogueFramework] [AI] AIPatrols.json has no Patrols array (or failed to parse).");
			return;
		}

		int spawned = 0;
		foreach (DialogueFW_PatrolConfig cfg : file.Patrols)
		{
			if (!cfg)
				continue;

			if (cfg.DialogueID <= 0)
			{
				Print("[DialogueFramework] [AI] A patrol has no DialogueID (or <= 0) -- skipping.");
				continue;
			}

			if (!cfg.Waypoints || cfg.Waypoints.Count() == 0)
			{
				Print("[DialogueFramework] [AI] Patrol DialogueID=" + cfg.DialogueID + " has no waypoints -- skipping.");
				continue;
			}

			DialogueAggro.RegisterPatrolOverride(cfg.DialogueID, cfg.PersistentAggroThreshold, cfg.PersistenceMode);

			int factionSlot = DialogueFW_FactionRegistry.SlotForName(cfg.Faction);
			if (factionSlot >= 0)
				cfg.Faction = DialogueFW_FactionRegistry.SlotClassSuffix(factionSlot);

			eAIDynamicPatrol patrol = eAIDynamicPatrolT<DialogueFW_Patrol>.CreateEx(cfg, vector.Zero);
			if (patrol)
			{
				spawned++;
				Print("[DialogueFramework] [AI] Registered talkable patrol DialogueID=" + cfg.DialogueID + " ('" + cfg.Name + "').");
			}
		}

		Print("[DialogueFramework] [AI] " + spawned + " talkable patrol(s) registered from AIPatrols.json.");
	}
}
#endif
