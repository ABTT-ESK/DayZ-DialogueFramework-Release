modded class MissionServer
{
	override void OnInit()
	{
		super.OnInit();

		Print("[DialogueFramework] MissionServer OnInit -- loading dialogue trees.");
		DialogueManager.GetInstance().Init();

	#ifdef EXPANSIONMODAI
		DialogueFW_FactionRegistry.Load();
		DialogueFW_AIPatrolSpawner.SpawnAll();
		DialogueAggro.Init();
	#endif
	}
}
