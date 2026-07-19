modded class MissionServer
{
	override void OnInit()
	{
		super.OnInit();

		Print("[DialogueFramework] MissionServer OnInit -- loading dialogue trees.");
		DialogueManager.GetInstance().Init();
	}
}
