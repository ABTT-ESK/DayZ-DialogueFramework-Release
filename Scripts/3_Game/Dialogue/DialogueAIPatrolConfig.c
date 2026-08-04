#ifdef EXPANSIONMODAI
class DialogueFW_PatrolConfig : ExpansionAIPatrol
{
	int DialogueID = 0;

	int PersistentAggroThreshold = -1;
	string PersistenceMode = "";
}

class DialogueFW_PatrolFile
{
	ref array<ref DialogueFW_PatrolConfig> Patrols;

	void DialogueFW_PatrolFile()
	{
		Patrols = new array<ref DialogueFW_PatrolConfig>;
	}
}
#endif
