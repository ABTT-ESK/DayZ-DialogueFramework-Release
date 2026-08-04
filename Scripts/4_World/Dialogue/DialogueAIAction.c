#ifdef EXPANSIONMODAI
class DialogueAISession
{
	protected static ref DialogueAISession s_Instance;

	ref DialogueTree m_PendingTree;
	eAIBase m_PendingAI;
	string m_PendingName;

	static DialogueAISession GetInstance()
	{
		if (!s_Instance)
			s_Instance = new DialogueAISession();
		return s_Instance;
	}

	void Set(DialogueTree tree, eAIBase ai, string name)
	{
		m_PendingTree = tree;
		m_PendingAI = ai;
		m_PendingName = name;
	}
}

class DialogueFW_ActionTalkToAI : ActionInteractBase
{
	void DialogueFW_ActionTalkToAI()
	{
		m_CommandUID = DayZPlayerConstants.CMD_GESTUREMOD_COME;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_ALL;
		m_HUDCursorIcon = CursorIcons.Cursor;
		m_Text = "Talk";
	}

	override void CreateConditionComponents()
	{
		m_ConditionItem = new CCINone;
		m_ConditionTarget = new CCTCursor;
	}

	override bool CanBeUsedSwimming()
	{
		return true;
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		eAIBase tAI;
		if (!Class.CastTo(tAI, target.GetObject()))
			return false;

		if (!tAI.IsAlive() || tAI.IsUnconscious())
			return false;

		return DialogueFW_ResolveTree(tAI) != null;
	}

	override void Start(ActionData action_data)
	{
		super.Start(action_data);

		if (!IsMissionClient())
			return;

		eAIBase tAI;
		if (!Class.CastTo(tAI, action_data.m_Target.GetObject()))
			return;

		DialogueTree tree = DialogueFW_ResolveTree(tAI);
		if (!tree)
			return;

		DialogueAISession.GetInstance().Set(tree, tAI, DialogueFW_SpeakerName(tAI));
		g_Game.GetUIManager().EnterScriptedMenu(MENU_DIALOGUEFW_AI, NULL);
	}

	static DialogueTree DialogueFW_ResolveTree(eAIBase tAI)
	{
		int patrolID = tAI.DialogueFW_GetPatrolID();
		if (patrolID <= 0)
			return null;

		return DialogueManager.GetInstance().GetTreeForAIPatrol(patrolID, tAI.DialogueFW_GetPatrolSubID());
	}

	static string DialogueFW_SpeakerName(eAIBase tAI)
	{
		eAIGroup group = tAI.GetGroup();
		if (group && group.GetFaction())
		{
			string name = group.GetFaction().GetName();
			if (name.IndexOf(DialogueFW_FactionRegistry.SLOT_PREFIX) == 0)
				return "";
			return name;
		}
		return "";
	}
}

modded class ActionConstructor
{
	override void RegisterActions(TTypenameArray actions)
	{
		super.RegisterActions(actions);

		actions.Insert(DialogueFW_ActionTalkToAI);
	}
}

modded class PlayerBase
{
	override void SetActions(out TInputActionMap InputActionMap)
	{
		super.SetActions(InputActionMap);

		AddAction(DialogueFW_ActionTalkToAI, InputActionMap);
	}
}
#endif
