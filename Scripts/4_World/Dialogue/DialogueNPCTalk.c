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
