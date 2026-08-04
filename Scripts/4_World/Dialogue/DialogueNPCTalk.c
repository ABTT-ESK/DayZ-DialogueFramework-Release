modded class ExpansionNPCBase
{
	protected float m_DialogueFW_TalkDuration;
	protected float m_DialogueFW_TalkDelta;
	protected bool m_DialogueFW_WasTalking;

	override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		super.OnRPC(sender, rpc_type, ctx);

		if (sender && rpc_type == DialogueFrameworkRPC.CLIENT_REQUEST_NPC_TALK)
		{
			Param1<float> talkData;
			if (!ctx.Read(talkData))
				return;

			DialogueFW_SetTalkingServer(talkData.param1);
		}
	}

	void DialogueFW_SetTalkingServer(float duration)
	{
		if (duration <= 0)
		{
			m_DialogueFW_TalkDuration = 0;
			m_DialogueFW_TalkDelta = 0;
			return;
		}

		m_DialogueFW_TalkDuration = duration - 0.5;
		m_DialogueFW_TalkDelta = 0;
	}

	void DialogueFW_SetTalkingClient(float duration)
	{
		Param1<float> params = new Param1<float>(duration);
		RPCSingleParam(DialogueFrameworkRPC.CLIENT_REQUEST_NPC_TALK, params, true, NULL);
	}

	override void CommandHandler(float pDt, int pCurrentCommandID, bool pCurrentCommandFinished)
	{
		super.CommandHandler(pDt, pCurrentCommandID, pCurrentCommandFinished);

		if (m_DialogueFW_TalkDuration <= 0)
		{
			if (!m_DialogueFW_WasTalking)
				return;

			HumanCommandAdditives stopAdditives = GetCommandModifier_Additives();
			if (stopAdditives)
				stopAdditives.SetTalking(false);

			m_DialogueFW_WasTalking = false;
			return;
		}

		HumanCommandAdditives additives = GetCommandModifier_Additives();
		if (!additives)
			return;

		additives.SetTalking(true);
		m_DialogueFW_WasTalking = true;

		m_DialogueFW_TalkDelta += pDt;
		if (m_DialogueFW_TalkDelta >= m_DialogueFW_TalkDuration)
		{
			m_DialogueFW_TalkDuration = 0;
			m_DialogueFW_TalkDelta = 0;
		}
	}
}

#ifdef EXPANSIONMODAI
modded class eAIBase
{
	protected float m_DialogueFW_AITalkDuration;
	protected float m_DialogueFW_AITalkDelta;
	protected bool m_DialogueFW_AIWasTalking;

	int m_DialogueFW_PatrolID = 0;
	int m_DialogueFW_PatrolSubID = 0;

	float m_DialogueFW_AggroCheckDelta;

	override void Init()
	{
		super.Init();

		RegisterNetSyncVariableInt("m_DialogueFW_PatrolID");
		RegisterNetSyncVariableInt("m_DialogueFW_PatrolSubID");
	}

	string DialogueFW_FactionName()
	{
		eAIGroup group = GetGroup();
		if (group && group.GetFaction())
			return group.GetFaction().GetName();
		return "";
	}

	void DialogueFW_SetPatrolID(int patrolID, int subID)
	{
		m_DialogueFW_PatrolID = patrolID;
		m_DialogueFW_PatrolSubID = subID;
		SetSynchDirty();
	}

	int DialogueFW_GetPatrolID()
	{
		return m_DialogueFW_PatrolID;
	}

	int DialogueFW_GetPatrolSubID()
	{
		return m_DialogueFW_PatrolSubID;
	}

	override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		super.OnRPC(sender, rpc_type, ctx);

		if (sender && rpc_type == DialogueFrameworkRPC.CLIENT_REQUEST_NPC_TALK)
		{
			Param1<float> aiTalkData;
			if (!ctx.Read(aiTalkData))
				return;

			DialogueFW_SetTalkingServer(aiTalkData.param1);
		}

		if (sender && rpc_type == DialogueFrameworkRPC.CLIENT_REQUEST_RECRUIT_AI)
		{
			Param1<int> recruitData;
			if (!ctx.Read(recruitData))
				return;

			PlayerBase recruiter = PlayerBase.GetPlayerByUID(sender.GetId());
			if (recruiter)
				DialogueFW_TryRecruit(recruiter, sender, recruitData.param1);
		}

		if (sender && rpc_type == DialogueFrameworkRPC.CLIENT_REQUEST_AI_HOLD)
		{
			Param1<bool> holdData;
			if (!ctx.Read(holdData))
				return;

			if (GetGame().IsServer())
				OverrideMovementSpeed(holdData.param1, 0);
		}

		if (sender && rpc_type == DialogueFrameworkRPC.CLIENT_REQUEST_AI_HOSTILE)
		{
			Param1<bool> hostileData;
			if (!ctx.Read(hostileData))
				return;

			PlayerBase aggressor = PlayerBase.GetPlayerByUID(sender.GetId());
			if (aggressor)
				DialogueFW_GoHostile(aggressor);
		}
	}

	void DialogueFW_RequestHoldClient(bool hold)
	{
		Param1<bool> holdParams = new Param1<bool>(hold);
		RPCSingleParam(DialogueFrameworkRPC.CLIENT_REQUEST_AI_HOLD, holdParams, true, NULL);
	}

	void DialogueFW_RequestHostileClient()
	{
		Param1<bool> hostileParams = new Param1<bool>(true);
		RPCSingleParam(DialogueFrameworkRPC.CLIENT_REQUEST_AI_HOSTILE, hostileParams, true, NULL);
	}

	void DialogueFW_GoHostile(PlayerBase player)
	{
		if (!GetGame().IsServer() || !player)
			return;

		eAIGroup group = GetGroup();
		if (!group)
			return;

		OverrideMovementSpeed(false, 0);
		group.AddTarget(player, player.GetTargetInformation(), 120000, true, 1.0);
		DialogueAggro.RegisterAggro(this, player);
	}

	void DialogueFW_CheckAggroReset()
	{
		if (!GetGame().IsServer() || m_DialogueFW_PatrolID <= 0)
			return;

		array<ref eAITarget> targets = GetTargets();
		if (!targets || targets.Count() == 0)
			return;

		string faction = DialogueFW_FactionName();

		array<ref eAITarget> snapshot = new array<ref eAITarget>;
		foreach (eAITarget t : targets)
			snapshot.Insert(t);

		foreach (eAITarget target : snapshot)
		{
			if (!target)
				continue;

			PlayerBase player;
			if (!Class.CastTo(player, target.GetEntity()))
				continue;

			if (!player.IsAlive())
			{
				if (DialogueAggro.Settings().ResetOnDeath)
					eAI_RemoveTarget(target);
				continue;
			}

			if (!player.GetIdentity())
				continue;

			string uid = player.GetIdentity().GetId();
			if (DialogueAggro.AnyAggroCount(uid, faction, m_DialogueFW_PatrolID) <= 0)
				continue;

			if (DialogueAggro.IsPermanent(uid, faction, m_DialogueFW_PatrolID))
				continue;

			if (DialogueAggro.PlayerIsCalm(this, player))
				eAI_RemoveTarget(target);
		}
	}

	void DialogueFW_RequestRecruitClient(int requiredQuestID)
	{
		Param1<int> recruitParams = new Param1<int>(requiredQuestID);
		RPCSingleParam(DialogueFrameworkRPC.CLIENT_REQUEST_RECRUIT_AI, recruitParams, true, NULL);
	}

	void DialogueFW_TryRecruit(PlayerBase player, PlayerIdentity identity, int requiredQuestID)
	{
		if (!GetGame().IsServer())
			return;

		if (!player || !IsAlive() || IsUnconscious())
			return;

		eAIGroup aiGroup = GetGroup();
		if (!aiGroup)
			return;

		eAIGroup playerGroup = player.GetGroup();
		if (aiGroup == playerGroup)
			return;

		if (aiGroup.GetLeader() && !aiGroup.GetLeader().IsAI())
			return;

		eAIFaction faction = aiGroup.GetFaction();
		if (!faction || eAI_IsPassive() || faction.IsInvincible())
			return;

		auto settings = GetExpansionSettings().GetAI(false);
		if (!settings || !settings.IsLoaded())
			return;

		if (requiredQuestID > 0)
		{
			ExpansionQuestModule questModule = ExpansionQuestModule.GetModuleInstance();
			if (!questModule || !questModule.HasCompletedQuest(requiredQuestID, identity.GetId()))
				return;
		}

		bool isPlayerMoving;
		bool friendly;

		if (faction.IsGuard())
		{
			if (!settings.CanRecruitGuards || (playerGroup && playerGroup.Count() - 1 >= settings.MaxRecruitableAI))
				return;

			if (eAI_GetCachedThreat(player.GetTargetInformation()) > 0.2)
			{
				ExpansionNotification("STR_EXPANSION_HOSTILE", "STR_EXPANSION_AI_CANNOT_RECRUIT_HOSTILE_TEMP").Error(identity);
				return;
			}
		}
		else if (!settings.CanRecruitFriendly || (playerGroup && playerGroup.Count() - 1 >= settings.MaxRecruitableAI))
		{
			return;
		}
		else if (PlayerIsEnemy(player, false, isPlayerMoving, friendly))
		{
			if (friendly)
				ExpansionNotification("STR_EXPANSION_HOSTILE", "STR_EXPANSION_AI_CANNOT_RECRUIT_HOSTILE_TEMP").Error(identity);

			return;
		}

		eAIGroup targetGroup = player.GetGroup();
		if (!targetGroup)
			targetGroup = eAIGroup.GetGroupByLeader(player, true, faction);
		else
			player.Expansion_SetFormerGroup(targetGroup);

		SetGroup(targetGroup, false);
	}

	void DialogueFW_SetTalkingServer(float duration)
	{
		if (duration <= 0)
		{
			m_DialogueFW_AITalkDuration = 0;
			m_DialogueFW_AITalkDelta = 0;
			return;
		}

		m_DialogueFW_AITalkDuration = duration - 0.5;
		m_DialogueFW_AITalkDelta = 0;
	}

	void DialogueFW_SetTalkingClient(float duration)
	{
		Param1<float> aiParams = new Param1<float>(duration);
		RPCSingleParam(DialogueFrameworkRPC.CLIENT_REQUEST_NPC_TALK, aiParams, true, NULL);
	}

	override void CommandHandler(float pDt, int pCurrentCommandID, bool pCurrentCommandFinished)
	{
		super.CommandHandler(pDt, pCurrentCommandID, pCurrentCommandFinished);

		if (GetGame().IsServer() && m_DialogueFW_PatrolID > 0)
		{
			m_DialogueFW_AggroCheckDelta += pDt;
			if (m_DialogueFW_AggroCheckDelta >= DialogueAggro.Settings().CheckInterval)
			{
				m_DialogueFW_AggroCheckDelta = 0;
				DialogueFW_CheckAggroReset();
			}
		}

		if (m_DialogueFW_AITalkDuration <= 0)
		{
			if (!m_DialogueFW_AIWasTalking)
				return;

			HumanCommandAdditives aiStopAdditives = GetCommandModifier_Additives();
			if (aiStopAdditives)
				aiStopAdditives.SetTalking(false);

			m_DialogueFW_AIWasTalking = false;
			return;
		}

		HumanCommandAdditives aiAdditives = GetCommandModifier_Additives();
		if (!aiAdditives)
			return;

		aiAdditives.SetTalking(true);
		m_DialogueFW_AIWasTalking = true;

		m_DialogueFW_AITalkDelta += pDt;
		if (m_DialogueFW_AITalkDelta >= m_DialogueFW_AITalkDuration)
		{
			m_DialogueFW_AITalkDuration = 0;
			m_DialogueFW_AITalkDelta = 0;
		}
	}
}
#endif
