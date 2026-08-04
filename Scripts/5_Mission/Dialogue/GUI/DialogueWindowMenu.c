class DialogueWindowMenu : UIScriptedMenu
{
	protected TextWidget m_SpeakerName;
	protected RichTextWidget m_SpeakerLine;
	protected WrapSpacerWidget m_ResponseList;
	protected Widget m_CloseButton;

	protected ref DialogueTree m_ActiveTree;
	protected ref DialogueNode m_ActiveNode;
	protected int m_ActiveQuestID = -1;
	protected int m_NPCID = -1;
	protected string m_NPCName = "";
	protected EffectSound m_DialogueVoiceSound;

	protected ref array<Widget> m_ResponseButtons;
	protected ref array<ref DialogueResponse> m_CurrentResponses;
	protected ref array<ExpansionQuestConfig> m_CurrentQuests;
	protected bool m_ShowingQuestList;

	protected ref DialogueNode m_NoQuestsNode;
	protected static const int NO_QUESTS_NODE_ID = -1000;

	protected static const string ICON_FOLDER = "DialogueFramework/GUI/images/";

	protected static const string ICON_EXT = ".paa";
	protected static const string ICON_EXIT = "icon_exit_ca";
	protected static const string ICON_CHAT = "icon_chat_ca";
	protected static const string ICON_CART = "icon_cart_ca";

	protected bool m_IconDiagLogged;

	protected ref array<ExpansionQuestRewardConfig> m_CurrentRewards;
	protected bool m_ShowingRewardList;

	protected ref map<string, bool> m_VisitedResponses;

	protected DialogueMenuConfig m_MenuConfig;
	protected string m_LayoutSuffix = "";
	protected Widget m_DialoguePanel;
	protected ScrollWidget m_ResponseScroll;

	protected ScrollWidget m_SpeakerLineScroll;
	protected Widget m_ConfirmPanel;
	protected RichTextWidget m_ConfirmText;
	protected Widget m_ConfirmYesButton;
	protected Widget m_ConfirmNoButton;

	protected ref array<EntityAI> m_RewardPreviewObjects;

	protected ref array<Widget> m_RewardDisplayWidgets;
	protected Widget m_RewardStrip;
	protected Widget m_RewardStripLabel;
	protected Widget m_RequiredStrip;
	protected Widget m_RequiredStripLabel;

	protected ref array<Widget> m_RequiredTiles;
	protected ref array<Widget> m_RewardTiles;

	protected float m_ScrollX;
	protected float m_ScrollY;
	protected float m_ScrollW;
	protected float m_ScrollH;

	protected static const float TILE_WIDTH = 300;
	protected static const float TILE_HEIGHT = 72;
	protected static const float TILE_MARGIN = 4;

	protected static const float STRIP_MAX_FRACTION = 0.30;

	protected static const float RESPONSE_MIN_FRACTION = 0.28;

	protected static const float ITEM_AREA_TOP = 0.28;
	protected static const float ITEM_AREA_MAX_FRACTION = 0.34;

	protected static const float GROUP_LABEL_FRACTION = 0.06;

	protected static const float TILE_MAX_SCALE = 1.5;
	protected static const float TILE_MIN_SCALE = 0.8;

	protected static const float SPEAKER_FONT_PX = 18;

	protected static const float SPEAKER_CHAR_RATIO = 0.5;
	protected static const float SPEAKER_LINE_SPACING = 1.35;

	protected static const int TILE_NAME_MAX = 46;

	protected bool m_StripHasGiven;
	protected bool m_StripHasNeeded;
	protected bool m_StripHasReward;
	protected int m_SelectedRewardIndex = -1;

	protected bool m_ShowingObjItemList;
	protected int m_SelectedObjItemIndex = -1;
	protected ref array<ref ExpansionQuestObjectiveDelivery> m_ObjItemChoices;
	protected ref array<int> m_ObjItemChoiceIndices;

	protected bool m_OpeningTrader;

	protected ExpansionNPCBase m_TalkingNPC;
#ifdef EXPANSIONMODAI
	protected eAIBase m_TalkingNPCAI;
	protected eAIBase m_TargetAI;
#endif

	void DialogueWindowMenu(DialogueTree tree, int npcID, string npcName)
	{
		m_ActiveTree = tree;
		m_NPCID = npcID;
		m_NPCName = npcName;

		m_ResponseButtons = new array<Widget>;
		m_CurrentResponses = new array<ref DialogueResponse>;
		m_CurrentQuests = new array<ExpansionQuestConfig>;
		m_CurrentRewards = new array<ExpansionQuestRewardConfig>;
		m_RewardPreviewObjects = new array<EntityAI>;
		m_VisitedResponses = new map<string, bool>;
		m_RewardDisplayWidgets = new array<Widget>;
		m_ObjItemChoices = new array<ref ExpansionQuestObjectiveDelivery>;
		m_ObjItemChoiceIndices = new array<int>;
	}

#ifdef EXPANSIONMODAI
	void DialogueFW_SetTargetAI(eAIBase ai)
	{
		m_TargetAI = ai;
	}
#endif

	override Widget Init()
	{
		m_MenuConfig = DialogueManager.GetInstance().GetMenuConfig();

		if (m_MenuConfig)
			m_LayoutSuffix = m_MenuConfig.GetLayoutSuffix();

		string layoutPath = LayoutPath("dialogue_menu");
		if (m_MenuConfig && m_MenuConfig.LayoutOverride != "")
		{
			Print("[DialogueFramework] [UI] Using layout override: " + m_MenuConfig.LayoutOverride);
			layoutPath = m_MenuConfig.LayoutOverride;
		}
		else
		{
			Print("[DialogueFramework] [UI] Font style: " + m_MenuConfig.FontStyle + " -> " + layoutPath);
		}

		layoutRoot = GetGame().GetWorkspace().CreateWidgets(layoutPath);

		if (!layoutRoot)
		{
			Print("[DialogueFramework] [UI] [ERROR] Could not load " + layoutPath + " -- falling back to the default layout.");
			m_LayoutSuffix = "";
			layoutRoot = GetGame().GetWorkspace().CreateWidgets(LayoutPath("dialogue_menu"));
		}

		m_SpeakerName = TextWidget.Cast(layoutRoot.FindAnyWidget("SpeakerName"));
		m_SpeakerLine = RichTextWidget.Cast(layoutRoot.FindAnyWidget("SpeakerLine"));
		m_SpeakerLineScroll = ScrollWidget.Cast(layoutRoot.FindAnyWidget("SpeakerLineScroll"));
		m_ResponseList = WrapSpacerWidget.Cast(layoutRoot.FindAnyWidget("ResponseList"));
		m_CloseButton = layoutRoot.FindAnyWidget("CloseButton");
		m_ResponseScroll = ScrollWidget.Cast(layoutRoot.FindAnyWidget("ResponseScroll"));
		m_ConfirmPanel = layoutRoot.FindAnyWidget("ConfirmPanel");
		m_ConfirmText = RichTextWidget.Cast(layoutRoot.FindAnyWidget("ConfirmText"));
		m_RewardStrip = layoutRoot.FindAnyWidget("RewardStrip");
		m_RewardStripLabel = layoutRoot.FindAnyWidget("RewardStripLabel");
		m_RequiredStrip = layoutRoot.FindAnyWidget("RequiredStrip");
		m_RequiredStripLabel = layoutRoot.FindAnyWidget("RequiredStripLabel");

		if (!m_RequiredTiles)
			m_RequiredTiles = new array<Widget>;
		if (!m_RewardTiles)
			m_RewardTiles = new array<Widget>;

		if (m_ResponseScroll)
		{
			m_ResponseScroll.GetPos(m_ScrollX, m_ScrollY);
			m_ResponseScroll.GetSize(m_ScrollW, m_ScrollH);
		}

		m_ConfirmYesButton = layoutRoot.FindAnyWidget("ConfirmYesButton");
		m_ConfirmNoButton = layoutRoot.FindAnyWidget("ConfirmNoButton");

		if (m_ConfirmPanel)
			m_ConfirmPanel.Show(false);

		ApplyMenuConfig();

		string diagTreeID = "NULL";
		if (m_ActiveTree)
			diagTreeID = "" + m_ActiveTree.ID;
		Print("[DialogueFramework] [DIAG] DialogueWindowMenu.Init() for NPC ID=" + m_NPCID + ", tree ID=" + diagTreeID);

		return layoutRoot;
	}

	protected void ApplyMenuConfig()
	{
		if (!m_MenuConfig || !layoutRoot)
			return;

		m_DialoguePanel = layoutRoot.FindAnyWidget("DialoguePanel");
		if (m_DialoguePanel)
		{
			float posX;
			float posY;
			m_MenuConfig.GetResolvedPosition(posX, posY);

			m_DialoguePanel.SetSize(m_MenuConfig.PanelWidth, m_MenuConfig.PanelHeight);
			m_DialoguePanel.SetPos(posX, posY);

			Print("[DialogueFramework] [UI] Panel placed at " + posX + ", " + posY + " (" + m_MenuConfig.Position + ")");
		}

		Widget background = layoutRoot.FindAnyWidget("DialoguePanelBackground");
		if (background)
			background.SetColor(m_MenuConfig.GetColor(m_MenuConfig.BackgroundColor));

		if (m_SpeakerName)
			m_SpeakerName.SetColor(m_MenuConfig.GetColor(m_MenuConfig.SpeakerNameColor));

		if (m_SpeakerLine)
			m_SpeakerLine.SetColor(m_MenuConfig.GetColor(m_MenuConfig.SpeakerTextColor));

		ApplyWindowBorder();
	}

	protected void ApplyWindowBorder()
	{
		Widget border = layoutRoot.FindAnyWidget("WindowBorder");
		if (!border)
			return;

		if (m_MenuConfig.WindowBorderThickness <= 0)
		{
			border.Show(false);
			return;
		}

		border.Show(true);

		int borderColor = m_MenuConfig.GetColor(m_MenuConfig.WindowBorderColor);
		float thickness = m_MenuConfig.WindowBorderThickness;

		SetBorderEdge(border, "WindowBorderTop", borderColor, 1.0, thickness);
		SetBorderEdge(border, "WindowBorderBottom", borderColor, 1.0, thickness);
		SetBorderEdge(border, "WindowBorderLeft", borderColor, thickness, 1.0);
		SetBorderEdge(border, "WindowBorderRight", borderColor, thickness, 1.0);
	}

	protected void SetBorderEdge(Widget parent, string name, int color, float w, float h)
	{
		Widget edge = parent.FindAnyWidget(name);
		if (!edge)
			return;

		edge.SetColor(color);
		edge.SetSize(w, h);
	}

	protected void ApplyBorderColor(Widget button)
	{
		if (!button || !m_MenuConfig)
			return;

		int borderColor = m_MenuConfig.GetColor(m_MenuConfig.HoverBorderColor);

		Widget top = button.FindAnyWidget("BorderTop");
		if (top)
			top.SetColor(borderColor);

		Widget bottom = button.FindAnyWidget("BorderBottom");
		if (bottom)
			bottom.SetColor(borderColor);

		Widget left = button.FindAnyWidget("BorderLeft");
		if (left)
			left.SetColor(borderColor);

		Widget right = button.FindAnyWidget("BorderRight");
		if (right)
			right.SetColor(borderColor);
	}

	protected void OpenRootNode()
	{
		if (!m_ActiveTree)
			return;

		DialogueNode root = FindStageNode(GetEffectiveRootNodeID());
		RenderNode(root);

		Print("[DialogueFramework] [DIAG] OpenRootNode() completed -- buttons created=" + m_ResponseButtons.Count());

		array<string> greetingPool = m_ActiveTree.GreetingVoiceLineIDs;
		if (greetingPool)
			PlayRandomVoiceLine(greetingPool);

		Print("[DialogueFramework] [DIAG] OpenRootNode() greeting voice attempted, done.");
	}

	protected bool m_StageResolved;
	protected ref array<ref DialogueNode> m_StageNodes;
	protected int m_StageRootNodeID = 1;

	protected void ResolveStage()
	{
		if (m_StageResolved)
			return;
		m_StageResolved = true;

		if (!m_ActiveTree)
			return;

		m_StageNodes = m_ActiveTree.Nodes;
		m_StageRootNodeID = m_ActiveTree.RootNodeID;

		if (!m_ActiveTree.Stages || m_ActiveTree.Stages.Count() == 0)
			return;

		ExpansionQuestPersistentData questData = ExpansionQuestModule.GetModuleInstance().GetClientQuestData();

		DialogueTreeStage best = null;
		int bestPriority = 0;
		int bestQuestID = -1;

		foreach (DialogueTreeStage stage : m_ActiveTree.Stages)
		{
			if (!stage || !stage.Nodes || stage.Nodes.Count() == 0)
				continue;

			bool hasQuestReq = stage.RequiredQuestID > 0;
			bool hasVarReq = stage.RequiredVars && stage.RequiredVars.Count() > 0;
			if (!hasQuestReq && !hasVarReq)
				continue;

			if (hasQuestReq)
			{
				if (!questData)
					continue;
				if (questData.GetQuestStateByQuestID(stage.RequiredQuestID) != ExpansionQuestState.COMPLETED)
					continue;
			}

			if (hasVarReq && !VarGatePasses(stage.RequiredVars))
				continue;

			bool better = false;
			if (!best)
				better = true;
			else if (stage.Priority > bestPriority)
				better = true;
			else if (stage.Priority == bestPriority && stage.RequiredQuestID > bestQuestID)
				better = true;

			if (better)
			{
				best = stage;
				bestPriority = stage.Priority;
				bestQuestID = stage.RequiredQuestID;
			}
		}

		if (best)
		{
			m_StageNodes = best.Nodes;
			m_StageRootNodeID = best.RootNodeID;
		}

		Print("[DialogueFramework] [DIAG] Active tree resolved -- stage priority=" + bestPriority + " quest=" + bestQuestID + " root=" + m_StageRootNodeID);
	}

	protected int GetEffectiveRootNodeID()
	{
		ResolveStage();
		return m_StageRootNodeID;
	}

	protected void ApplySpeakerName()
	{
		if (!m_SpeakerName)
			return;

		string display = m_NPCName;
		string rep = GetReputationDisplay();
		if (rep != "")
			display = display + "   -   " + rep;

		m_SpeakerName.SetText(display);
	}

	protected string GetReputationDisplay()
	{
		if (!m_ActiveTree || m_ActiveTree.ReputationVar == "")
			return "";

		int value = DialogueVars.GetInstance().GetClientState().Get(m_ActiveTree.ReputationVar);

		string label = DialogueRepTierList.LabelFor(m_ActiveTree.ReputationTiers, value);
		if (label != "")
			return label;

		return "Reputation: " + value;
	}

	protected DialogueNode FindStageNode(int nodeID)
	{
		ResolveStage();

		if (!m_StageNodes)
			return null;

		foreach (DialogueNode node : m_StageNodes)
		{
			if (node && node.ID == nodeID)
				return node;
		}

		return null;
	}

	protected void RenderNode(DialogueNode node)
	{
		if (!node)
		{
			Print("[DialogueFramework] [DIAG] RenderNode() called with NULL node -- bailing.");
			return;
		}

		Print("[DialogueFramework] [DIAG] RenderNode() entered, node ID=" + node.ID + " Type=" + node.Type);

		if (node.Type == DialogueNodeType.QUEST_LIST)
		{
			ShowLiveQuestList();
			return;
		}

		m_ActiveNode = node;
		m_ShowingQuestList = false;
		m_ShowingRewardList = false;
		m_ShowingObjItemList = false;
		m_SelectedRewardIndex = -1;
		HideRewardConfirm();
		HideRewardDisplay();

		array<ref DialogueResponse> visible = GetVisibleResponses(node);

		string spokenText = node.SpeakerText;
		array<string> spokenVoicePool = node.VoiceLineIDs;
		DialogueSpeakerLine chosenLine = PickSpeakerLine(node);
		if (chosenLine)
		{
			spokenText = chosenLine.Text;
			if (chosenLine.VoiceLineIDs && chosenLine.VoiceLineIDs.Count() > 0)
				spokenVoicePool = chosenLine.VoiceLineIDs;
		}

		if (m_SpeakerName)
			ApplySpeakerName();
		if (m_SpeakerLine)
			SetSpeakerLine(spokenText);

		ClearButtons();

		m_CurrentResponses = visible;
		for (int i = 0; i < visible.Count(); i++)
		{
			bool wasVisited = m_VisitedResponses.Contains(node.ID.ToString() + ":" + i);
			m_ResponseButtons.Insert(CreateResponseButton(visible[i].Text, wasVisited, IconForResponse(visible[i])));
		}

		StopDialogueVoice();

		if (spokenVoicePool)
			PlayRandomVoiceLine(spokenVoicePool);
	}

	protected array<ref DialogueResponse> GetVisibleResponses(DialogueNode node)
	{
		array<ref DialogueResponse> visible = new array<ref DialogueResponse>;

		if (!node.Responses)
			return visible;

		foreach (DialogueResponse response : node.Responses)
		{
			if (PassesGating(response))
				visible.Insert(response);
		}

		return visible;
	}

	protected bool PassesGating(DialogueResponse response)
	{
		if (!QuestGatePasses(response.RequiredQuestID))
			return false;
		if (!VarGatePasses(response.RequiredVars))
			return false;
		if (response.MaxUses > 0 && response.UsesKey != "")
		{
			int used = DialogueVars.GetInstance().GetClientState().Get(response.UsesKey);
			if (used >= response.MaxUses)
				return false;
		}
		return true;
	}

	protected bool QuestGatePasses(int requiredQuestID)
	{
		if (requiredQuestID <= 0)
			return true;

		ExpansionQuestPersistentData questData = ExpansionQuestModule.GetModuleInstance().GetClientQuestData();
		if (!questData)
			return false;

		return questData.GetQuestStateByQuestID(requiredQuestID) == ExpansionQuestState.COMPLETED;
	}

	protected bool VarGatePasses(array<ref DialogueVarOp> conditions)
	{
		return DialogueVarOpList.Evaluate(conditions, DialogueVars.GetInstance().GetClientState());
	}

	protected DialogueSpeakerLine PickSpeakerLine(DialogueNode node)
	{
		if (!node.SpeakerLines || node.SpeakerLines.Count() == 0)
			return null;

		DialogueSpeakerLine overrideLine = null;
		int overrideBest = -1;
		foreach (DialogueSpeakerLine candidate : node.SpeakerLines)
		{
			if (!candidate)
				continue;
			if (candidate.OverrideQuestID <= 0)
				continue;
			if (candidate.OverrideQuestID <= overrideBest)
				continue;
			if (!QuestGatePasses(candidate.OverrideQuestID))
				continue;
			if (!QuestGatePasses(candidate.RequiredQuestID))
				continue;
			if (!VarGatePasses(candidate.RequiredVars))
				continue;

			overrideBest = candidate.OverrideQuestID;
			overrideLine = candidate;
		}

		if (overrideLine)
			return overrideLine;

		array<ref DialogueSpeakerLine> pool = new array<ref DialogueSpeakerLine>;
		foreach (DialogueSpeakerLine line : node.SpeakerLines)
		{
			if (line && QuestGatePasses(line.RequiredQuestID) && VarGatePasses(line.RequiredVars))
				pool.Insert(line);
		}

		if (pool.Count() == 0)
			return null;

		int baseWeight = 0;
		if (node.SpeakerText != "")
			baseWeight = 1;

		int pick = Math.RandomInt(0, pool.Count() + baseWeight);
		if (pick >= pool.Count())
			return null;

		return pool.Get(pick);
	}

	protected void OnDialogueResponseSelected(DialogueResponse response)
	{
		if (!response)
			return;

		m_PendingResponse = response;
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(ExecutePendingResponse, 0, false);
	}

	protected ref DialogueResponse m_PendingResponse;

	protected void ApplySetVars(DialogueResponse response)
	{
		bool hasSet = response.SetVars && response.SetVars.Count() > 0;
		bool hasUses = response.MaxUses > 0 && response.UsesKey != "";
		if (!hasSet && !hasUses)
			return;

		array<ref DialogueVarOp> ops = new array<ref DialogueVarOp>;
		if (hasSet)
		{
			foreach (DialogueVarOp op : response.SetVars)
				ops.Insert(op);
		}
		if (hasUses)
		{
			DialogueVarOp useOp = new DialogueVarOp();
			useOp.Name = response.UsesKey;
			useOp.Op = "INCREASE";
			useOp.Value = 1;
			ops.Insert(useOp);
		}

		DialogueVarOpList.Apply(ops, DialogueVars.GetInstance().GetClientState());

		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		if (!player)
			return;

		ScriptRPC rpc = new ScriptRPC();
		DialogueVarOpList.Write(rpc, ops);
		rpc.Send(player, DialogueFrameworkRPC.CLIENT_APPLY_VARS, true, null);
	}

	protected void ExecutePendingResponse()
	{
		DialogueResponse response = m_PendingResponse;
		m_PendingResponse = null;
		if (!response)
			return;

		Print("[DialogueFramework] [DIAG] ExecutePendingResponse() actionType=" + response.ActionType);

		ApplySetVars(response);

		switch (response.ActionType)
		{
			case DialogueActionType.SHOW_QUEST_LIST:
				ShowLiveQuestList();
				break;

			case DialogueActionType.OPEN_TRADER:
			#ifdef EXPANSIONMODMARKET
				Print("[DialogueFramework] [DIAG] OPEN_TRADER -- closing dialogue and opening the market.");
				m_OpeningTrader = true;
				EndConversation();
			#else
				Print("[DialogueFramework] [DIAG] OPEN_TRADER used but the Market module isn't loaded -- ending conversation.");
				EndConversation();
			#endif
				break;

			case DialogueActionType.END_CONVERSATION:
				EndConversation();
				break;

			case DialogueActionType.ACCEPT_QUEST:
				AcceptActiveQuest();
				break;

			case DialogueActionType.DECLINE_QUEST:
				EndConversation();
				break;

			case DialogueActionType.TURN_IN_QUEST:
				TurnInActiveQuest();
				break;

			case DialogueActionType.RECRUIT_AI:
			#ifdef EXPANSIONMODAI
				if (m_TargetAI)
					m_TargetAI.DialogueFW_RequestRecruitClient(response.RequiredQuestID);
			#endif
				EndConversation();
				break;

			case DialogueActionType.GO_HOSTILE:
			#ifdef EXPANSIONMODAI
				if (m_TargetAI)
					m_TargetAI.DialogueFW_RequestHostileClient();
			#endif
				EndConversation();
				break;

			case DialogueActionType.NONE:
			default:
				if (response.NextNodeID == -1)
				{
					EndConversation();
					return;
				}
				RenderNode(FindStageNode(response.NextNodeID));
				break;
		}
	}

	protected void ShowLiveQuestList()
	{
		Print("[DialogueFramework] [DIAG] ShowLiveQuestList() entered.");
		StopDialogueVoice();

		array<ExpansionQuestConfig> validQuests = GetAvailableQuestsForNPC();
		Print("[DialogueFramework] [DIAG] ShowLiveQuestList() got " + validQuests.Count() + " valid quests.");

		if (validQuests.Count() == 0)
		{
			ShowNoQuestsStep();
			return;
		}

		m_ShowingQuestList = true;
		m_ShowingRewardList = false;
		m_ShowingObjItemList = false;

		if (m_SpeakerName)
			ApplySpeakerName();
		if (m_SpeakerLine)
			SetSpeakerLine(GetQuestListPrompt());

		ClearButtons();

		m_CurrentQuests = validQuests;
		foreach (ExpansionQuestConfig quest : validQuests)
		{
			int remaining;
			string title = quest.GetTitle();
			if (IsQuestOnCooldown(quest, remaining))
				title = title + "  (" + ExpansionStatic.GetTimeString(remaining) + ")";

			m_ResponseButtons.Insert(CreateResponseButton(title, IsQuestOnCooldown(quest, remaining), ICON_CHAT));
		}

		array<string> backTexts = ResolveQuestListBackTexts();
		if (backTexts)
		{
			foreach (string backText : backTexts)
				m_ResponseButtons.Insert(CreateResponseButton(backText, false, ICON_CHAT));
		}
	}

	protected void SetSpeakerLine(string text)
	{
		if (!m_SpeakerLine)
			return;

		m_SpeakerLine.SetText(text);

		if (!m_SpeakerLineScroll)
			return;

		float scrollW;
		float scrollH;
		m_SpeakerLineScroll.GetScreenSize(scrollW, scrollH);

		if (scrollW > 0 && scrollH > 0)
		{
			float charWidth = SPEAKER_FONT_PX * SPEAKER_CHAR_RATIO;
			float perLine = scrollW / charWidth;
			if (perLine < 8)
				perLine = 8;

			int lines = 1;
			float consumed = perLine;
			while (consumed < text.Length())
			{
				consumed = consumed + perLine;
				lines = lines + 1;
			}

			lines = lines + 1;

			float neededPx = lines * SPEAKER_FONT_PX * SPEAKER_LINE_SPACING;
			if (neededPx < scrollH)
				neededPx = scrollH;

			m_SpeakerLine.SetSize(0.965, neededPx);

			Print("[DialogueFramework] [UI] Speaker line: " + text.Length() + " chars, ~" + perLine + " per line, " + lines + " line(s), " + neededPx + "px in a " + scrollH + "px view");
		}
		else
		{
			Print("[DialogueFramework] [UI] Speaker line: scroll not measured yet, leaving the layout default.");
		}

		m_SpeakerLineScroll.VScrollToPos01(0);
	}

	protected string GetQuestListPrompt()
	{
		DialogueQuestText questText = FindProgressQuestText(PROGRESS_QUEST_LIST);
		if (questText)
		{
			string questLine = PickRandomLine(questText.QuestListTexts);
			if (questLine != "")
				return questLine;
		}

		if (m_ActiveTree)
		{
			string treeLine = PickRandomLine(m_ActiveTree.QuestListTexts);
			if (treeLine != "")
				return treeLine;
		}

		return "What do you need done?";
	}

	protected void ShowNoQuestsStep()
	{
		m_ShowingQuestList = false;
		m_ShowingRewardList = false;
		m_CurrentQuests.Clear();

		string spoken = "";
		array<string> backTexts = null;
		array<string> leaveTexts = null;
		array<string> voiceLines = null;

		DialogueQuestText questText = FindProgressQuestText(PROGRESS_NO_QUESTS);
		if (questText)
		{
			spoken = PickRandomLine(questText.NoQuestsTexts);
			backTexts = questText.NoQuestsBackTexts;
			leaveTexts = questText.NoQuestsLeaveTexts;
			Print("[DialogueFramework] [DIAG] ShowNoQuestsStep() using QuestText for quest " + questText.QuestID);
		}

		if (m_ActiveTree)
		{
			if (spoken == "")
				spoken = PickRandomLine(m_ActiveTree.NoQuestsTexts);

			if (!backTexts || backTexts.Count() == 0)
				backTexts = m_ActiveTree.NoQuestsBackTexts;

			if (!leaveTexts || leaveTexts.Count() == 0)
				leaveTexts = m_ActiveTree.NoQuestsLeaveTexts;

			voiceLines = m_ActiveTree.NoQuestsVoiceLineIDs;
		}

		if (spoken == "")
			spoken = "Nothing for you right now.";

		m_NoQuestsNode = new DialogueNode();
		m_NoQuestsNode.ID = NO_QUESTS_NODE_ID;
		m_NoQuestsNode.Type = DialogueNodeType.STANDARD;
		m_NoQuestsNode.SpeakerText = spoken;

		if (voiceLines)
		{
			foreach (string voiceLine : voiceLines)
				m_NoQuestsNode.VoiceLineIDs.Insert(voiceLine);
		}

		int rootID = GetEffectiveRootNodeID();

		if (backTexts)
		{
			foreach (string backText : backTexts)
				m_NoQuestsNode.Responses.Insert(BuildNoQuestsResponse(backText, rootID, DialogueActionType.NONE));
		}

		if (leaveTexts)
		{
			foreach (string leaveText : leaveTexts)
				m_NoQuestsNode.Responses.Insert(BuildNoQuestsResponse(leaveText, -1, DialogueActionType.END_CONVERSATION));
		}

		if (m_NoQuestsNode.Responses.Count() == 0)
			m_NoQuestsNode.Responses.Insert(BuildNoQuestsResponse("Back", rootID, DialogueActionType.NONE));

		Print("[DialogueFramework] [DIAG] ShowNoQuestsStep() built " + m_NoQuestsNode.Responses.Count() + " response(s).");

		RenderNode(m_NoQuestsNode);
	}

	protected DialogueResponse BuildNoQuestsResponse(string text, int nextNodeID, string actionType)
	{
		DialogueResponse response = new DialogueResponse();
		response.Text = text;
		response.NextNodeID = nextNodeID;
		response.RequiredQuestID = -1;
		response.ActionType = actionType;
		return response;
	}

	protected array<string> ResolveBackTexts(array<string> perQuest, array<string> treeLevel)
	{
		if (perQuest && perQuest.Count() > 0)
			return perQuest;

		if (treeLevel && treeLevel.Count() > 0)
			return treeLevel;

		return null;
	}

	protected static const int BACK_SCREEN_OFFER = 0;
	protected static const int BACK_SCREEN_INPROGRESS = 1;
	protected static const int BACK_SCREEN_TURNIN = 2;

	protected array<string> ResolveDetailBackTexts(DialogueQuestText questText, int screen)
	{
		array<string> perQuest = null;
		array<string> treeLevel = null;

		if (screen == BACK_SCREEN_INPROGRESS)
		{
			if (questText) perQuest = questText.InProgressBackTexts;
			if (m_ActiveTree) treeLevel = m_ActiveTree.InProgressBackTexts;
		}
		else if (screen == BACK_SCREEN_TURNIN)
		{
			if (questText) perQuest = questText.TurnInBackTexts;
			if (m_ActiveTree) treeLevel = m_ActiveTree.TurnInBackTexts;
		}
		else
		{
			if (questText) perQuest = questText.OfferBackTexts;
			if (m_ActiveTree) treeLevel = m_ActiveTree.OfferBackTexts;
		}

		return ResolveBackTexts(perQuest, treeLevel);
	}

	protected array<string> ResolveQuestListBackTexts()
	{
		array<string> treeLevel = null;
		if (m_ActiveTree)
			treeLevel = m_ActiveTree.QuestListBackTexts;

		return ResolveBackTexts(FindProgressBackTexts(), treeLevel);
	}

	protected array<string> FindProgressBackTexts()
	{
		if (m_NPCID == -1)
			return null;

		ExpansionQuestPersistentData questData = ExpansionQuestModule.GetModuleInstance().GetClientQuestData();
		if (!questData)
			return null;

		map<int, ref ExpansionQuestConfig> allConfigs = ExpansionQuestModule.GetModuleInstance().GetQuestConfigs();
		if (!allConfigs)
			return null;

		array<string> best = null;
		int bestID = -1;

		foreach (int questID, ExpansionQuestConfig questConfig : allConfigs)
		{
			if (questID <= bestID)
				continue;

			if (questData.GetQuestStateByQuestID(questID) != ExpansionQuestState.COMPLETED)
				continue;

			if (!QuestBelongsToThisNPC(questConfig))
				continue;

			DialogueQuestText candidate = DialogueManager.GetInstance().GetQuestText(questID);
			if (!candidate || !candidate.QuestListBackTexts || candidate.QuestListBackTexts.Count() == 0)
				continue;

			bestID = questID;
			best = candidate.QuestListBackTexts;
		}

		return best;
	}

	protected void AddBackToConversationResponses(DialogueNode node, array<string> backTexts)
	{
		if (!node || !m_ActiveTree)
			return;

		if (!backTexts || backTexts.Count() == 0)
			return;

		foreach (string backText : backTexts)
			node.Responses.Insert(BuildNoQuestsResponse(backText, GetEffectiveRootNodeID(), DialogueActionType.NONE));
	}

	protected void ReturnToRootDeferred()
	{
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(ExecuteReturnToRoot, 0, false);
	}

	protected void ExecuteReturnToRoot()
	{
		if (!m_ActiveTree)
			return;

		RenderNode(FindStageNode(GetEffectiveRootNodeID()));
	}

	protected static const int PROGRESS_NO_QUESTS = 0;
	protected static const int PROGRESS_QUEST_LIST = 1;

	protected array<string> GetProgressPool(DialogueQuestText questText, int mode)
	{
		if (!questText)
			return null;

		if (mode == PROGRESS_QUEST_LIST)
			return questText.QuestListTexts;

		return questText.NoQuestsTexts;
	}

	protected DialogueQuestText FindProgressQuestText(int mode)
	{
		if (m_NPCID == -1)
			return null;

		ExpansionQuestPersistentData questData = ExpansionQuestModule.GetModuleInstance().GetClientQuestData();
		if (!questData)
			return null;

		map<int, ref ExpansionQuestConfig> allConfigs = ExpansionQuestModule.GetModuleInstance().GetQuestConfigs();
		if (!allConfigs)
			return null;

		DialogueQuestText best = null;
		int bestID = -1;

		foreach (int questID, ExpansionQuestConfig questConfig : allConfigs)
		{
			if (questID <= bestID)
				continue;

			if (questData.GetQuestStateByQuestID(questID) != ExpansionQuestState.COMPLETED)
				continue;

			if (!QuestBelongsToThisNPC(questConfig))
				continue;

			DialogueQuestText candidate = DialogueManager.GetInstance().GetQuestText(questID);
			if (!candidate)
				continue;

			array<string> pool = GetProgressPool(candidate, mode);
			if (!pool)
				continue;

			if (pool.Count() == 0)
				continue;

			bestID = questID;
			best = candidate;
		}

		return best;
	}

	protected bool QuestBelongsToThisNPC(ExpansionQuestConfig questConfig)
	{
		if (!questConfig)
			return false;

		array<int> givers = questConfig.GetQuestGiverIDs();
		if (givers && givers.Find(m_NPCID) > -1)
			return true;

		array<int> turnIns = questConfig.GetQuestTurnInIDs();
		if (turnIns && turnIns.Find(m_NPCID) > -1)
			return true;

		return false;
	}

	protected string PickRandomLine(array<string> pool)
	{
		if (!pool)
			return "";

		if (pool.Count() == 0)
			return "";

		return pool.Get(Math.RandomInt(0, pool.Count()));
	}

	protected array<ExpansionQuestConfig> GetAvailableQuestsForNPC()
	{
		Print("[DialogueFramework] [DIAG] GetAvailableQuestsForNPC() entered, NPC ID=" + m_NPCID);
		array<ExpansionQuestConfig> result = new array<ExpansionQuestConfig>;

		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		if (!player)
			return result;

		ExpansionQuestPersistentData questData = ExpansionQuestModule.GetModuleInstance().GetClientQuestData();

		map<int, ref ExpansionQuestConfig> allConfigs = ExpansionQuestModule.GetModuleInstance().GetQuestConfigs();
		if (!allConfigs)
		{
			Print("[DialogueFramework] [DIAG] GetAvailableQuestsForNPC() -- GetQuestConfigs() returned NULL.");
			return result;
		}

		Print("[DialogueFramework] [DIAG] GetAvailableQuestsForNPC() iterating " + allConfigs.Count() + " configs.");

		foreach (int questID, ExpansionQuestConfig questConfig : allConfigs)
		{
			if (ExpansionQuestModule.GetModuleInstance().QuestDisplayConditions(questConfig, player, questData, m_NPCID, true))
				result.Insert(questConfig);
		}

		Print("[DialogueFramework] [DIAG] GetAvailableQuestsForNPC() done, " + result.Count() + " matched.");
		return result;
	}

	protected void OnQuestSelectedFromList(ExpansionQuestConfig quest)
	{
		if (!quest)
			return;

		m_PendingQuest = quest;
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(ExecutePendingQuestSelection, 0, false);
	}

	protected ExpansionQuestConfig m_PendingQuest;

	protected void ExecutePendingQuestSelection()
	{
		ExpansionQuestConfig quest = m_PendingQuest;
		m_PendingQuest = null;
		if (!quest)
			return;

		ShowQuestDetail(quest);
	}

	protected void ShowQuestDetail(ExpansionQuestConfig quest)
	{
		DialogueQuestText questText = DialogueManager.GetInstance().GetQuestText(quest.GetID());
		if (!questText)
			questText = new DialogueQuestText();

		m_ActiveQuestID = quest.GetID();

		ExpansionQuestPersistentData questData = ExpansionQuestModule.GetModuleInstance().GetClientQuestData();
		ExpansionQuestState state = ExpansionQuestState.NONE;
		if (questData)
			state = questData.GetQuestStateByQuestID(quest.GetID());

		array<string> descriptions = quest.GetDescriptions();

		DialogueNode detail = new DialogueNode();
		detail.Type = DialogueNodeType.QUEST_DETAIL;

		if (state == ExpansionQuestState.CAN_TURNIN)
		{
			detail.SpeakerText = GetDescriptionSafe(descriptions, 2);
			detail.VoiceLineIDs.Insert("Quest_" + quest.GetID() + "_Complete");

			AddQuestResponses(detail, questText.TurnInTexts, "Here you go.", DialogueActionType.TURN_IN_QUEST);
			AddQuestResponses(detail, questText.NotYetTexts, "Not yet.", DialogueActionType.END_CONVERSATION);
			AddBackToConversationResponses(detail, ResolveDetailBackTexts(questText, BACK_SCREEN_TURNIN));
		}
		else if (state == ExpansionQuestState.STARTED)
		{
			detail.SpeakerText = GetDescriptionSafe(descriptions, 1);
			detail.VoiceLineIDs.Insert("Quest_" + quest.GetID() + "_InProgress");

			AddQuestResponses(detail, questText.InProgressTexts, "Still working on it.", DialogueActionType.END_CONVERSATION);
			AddBackToConversationResponses(detail, ResolveDetailBackTexts(questText, BACK_SCREEN_INPROGRESS));
		}
		else
		{
			int remaining;
			if (IsQuestOnCooldown(quest, remaining))
			{
				detail.SpeakerText = GetDescriptionSafe(descriptions, 0) + "\n\nNot again yet. Come back in " + ExpansionStatic.GetTimeString(remaining) + ".";
				AddQuestResponses(detail, questText.NotYetTexts, "Another time, then.", DialogueActionType.END_CONVERSATION);
				AddBackToConversationResponses(detail, ResolveDetailBackTexts(questText, BACK_SCREEN_OFFER));

				Print("[DialogueFramework] [DIAG] Quest " + quest.GetID() + " is on cooldown, " + remaining + "s remaining -- accept suppressed.");

				RenderNode(detail);
				ShowItemDisplay(quest);
				return;
			}

			detail.SpeakerText = GetDescriptionSafe(descriptions, 0);
			detail.VoiceLineIDs.Insert("Quest_" + quest.GetID() + "_Start");

			AddQuestResponses(detail, questText.AcceptTexts, "I'll take it.", DialogueActionType.ACCEPT_QUEST);
			AddQuestResponses(detail, questText.DeclineTexts, "Not interested.", DialogueActionType.DECLINE_QUEST);
			AddBackToConversationResponses(detail, ResolveDetailBackTexts(questText, BACK_SCREEN_OFFER));
		}

		RenderNode(detail);

		ShowItemDisplay(quest);
	}

	protected string LayoutPath(string baseName)
	{
		return "DialogueFramework/GUI/layouts/" + baseName + m_LayoutSuffix + ".layout";
	}

	protected bool IsQuestOnCooldown(ExpansionQuestConfig quest, out int remaining)
	{
		remaining = 0;

		if (!quest)
			return false;

		if (!quest.IsDailyQuest() && !quest.IsWeeklyQuest())
			return false;

		ExpansionQuestModule questModule = ExpansionQuestModule.GetModuleInstance();
		if (!questModule)
			return false;

		ExpansionQuestPersistentData questData = questModule.GetClientQuestData();
		if (!questData)
			return false;

		return questData.HasCooldownOnQuest(quest.GetID(), remaining);
	}

	protected void ShowItemDisplay(ExpansionQuestConfig quest)
	{
		if (!m_RewardStrip || !quest)
			return;

		m_StripHasGiven = false;
		m_StripHasNeeded = false;
		m_StripHasReward = false;
		m_RequiredTiles.Clear();
		m_RewardTiles.Clear();

		array<ref ExpansionQuestItemConfig> questItems = quest.GetQuestItems();
		if (questItems)
		{
			foreach (ExpansionQuestItemConfig questItem : questItems)
			{
				if (questItem)
					AddItemTile(questItem.GetClassName(), questItem.GetAmount(), "given");
			}
		}

		AddObjectiveTiles(quest);

		array<ref ExpansionQuestRewardConfig> rewards = quest.GetRewards();
		if (rewards)
		{
			foreach (ExpansionQuestRewardConfig reward : rewards)
			{
				if (reward)
					AddItemTile(reward.GetClassName(), reward.GetAmount(), "reward");
			}
		}

		if (m_RewardDisplayWidgets.Count() == 0)
		{
			LayoutResponseArea(0);
			return;
		}

		ApplyRequiredHeading();
		LayoutItemGroups();

		Print("[DialogueFramework] [DIAG] ShowItemDisplay -- " + m_RequiredTiles.Count() + " required, " + m_RewardTiles.Count() + " reward tile(s).");
	}

	protected void AddObjectiveTiles(ExpansionQuestConfig quest)
	{
		array<ref ExpansionQuestObjectiveConfigBase> objectives = quest.GetObjectives();
		if (!objectives)
			return;

		foreach (ExpansionQuestObjectiveConfigBase objective : objectives)
		{
			if (!objective)
				continue;

			int objectiveType = objective.GetObjectiveType();

			if (objectiveType == ExpansionQuestObjectiveType.COLLECT)
			{
				ExpansionQuestObjectiveCollectionConfig collectConfig = ExpansionQuestObjectiveCollectionConfig.Cast(objective);
				if (collectConfig)
					AddDeliveryTiles(collectConfig.GetCollections());
			}
			else if (objectiveType == ExpansionQuestObjectiveType.DELIVERY)
			{
				ExpansionQuestObjectiveDeliveryConfig deliveryConfig = ExpansionQuestObjectiveDeliveryConfig.Cast(objective);
				if (deliveryConfig)
					AddDeliveryTiles(deliveryConfig.GetCollections());
			}
			else if (objectiveType == ExpansionQuestObjectiveType.CRAFTING)
			{
				ExpansionQuestObjectiveCraftingConfig craftingConfig = ExpansionQuestObjectiveCraftingConfig.Cast(objective);
				if (!craftingConfig)
					continue;

				array<string> craftNames = craftingConfig.GetItemNames();
				if (!craftNames)
					continue;

				foreach (string craftName : craftNames)
					AddItemTile(craftName, 1, "needed");
			}
		}
	}

	protected void AddDeliveryTiles(array<ref ExpansionQuestObjectiveDelivery> deliveries)
	{
		if (!deliveries)
			return;

		foreach (ExpansionQuestObjectiveDelivery delivery : deliveries)
		{
			if (delivery)
				AddItemTile(delivery.GetClassName(), delivery.GetAmount(), "needed");
		}
	}

	protected void AddItemTile(string className, int amount, string suffix)
	{
		if (className == "")
			return;

		if (suffix == "given")
			m_StripHasGiven = true;
		if (suffix == "needed")
			m_StripHasNeeded = true;
		if (suffix == "reward")
			m_StripHasReward = true;

		Widget parentStrip = m_RequiredStrip;
		if (suffix == "reward")
			parentStrip = m_RewardStrip;

		if (!parentStrip)
			return;

		Widget tile = GetGame().GetWorkspace().CreateWidgets(LayoutPath("dialogue_reward_display"), parentStrip);
		if (!tile)
			return;

		TextWidget tileName = TextWidget.Cast(tile.FindAnyWidget("DialogueRewardDisplayName"));
		if (tileName)
		{
			tileName.SetText(GetItemDisplayName(className));
			if (m_MenuConfig)
				tileName.SetColor(m_MenuConfig.GetColor(m_MenuConfig.ResponseTextColor));
		}

		TextWidget tileAmount = TextWidget.Cast(tile.FindAnyWidget("DialogueRewardDisplayAmount"));
		if (tileAmount)
			tileAmount.SetText("x" + amount);

		Widget tileBackground = tile.FindAnyWidget("DialogueRewardDisplayBackground");
		if (tileBackground && m_MenuConfig)
			tileBackground.SetColor(m_MenuConfig.GetColor(m_MenuConfig.ResponseBackgroundColor));

		ItemPreviewWidget tilePreview = ItemPreviewWidget.Cast(tile.FindAnyWidget("DialogueRewardDisplayPreview"));
		if (tilePreview)
		{
			EntityAI tileEntity = EntityAI.Cast(GetGame().CreateObjectEx(className, vector.Zero, ECE_LOCAL|ECE_NOLIFETIME));
			if (tileEntity)
			{
				m_RewardPreviewObjects.Insert(tileEntity);
				tilePreview.SetItem(tileEntity);
			}
		}

		m_RewardDisplayWidgets.Insert(tile);

		if (suffix == "reward")
			m_RewardTiles.Insert(tile);
		else
			m_RequiredTiles.Insert(tile);
	}

	protected int TilesPerRow(float tileW, float rowPx, int count)
	{
		float slotW = tileW + TILE_MARGIN * 2;
		float used = 0;
		int fit = 0;

		while (fit < count)
		{
			if (used + slotW > rowPx)
				break;

			used = used + slotW;
			fit = fit + 1;
		}

		if (fit < 1)
			fit = 1;

		return fit;
	}

	protected int RowsNeeded(int count, int perRow)
	{
		if (perRow < 1)
			perRow = 1;

		int rows = 0;
		int remaining = count;

		while (remaining > 0)
		{
			remaining = remaining - perRow;
			rows = rows + 1;
		}

		if (rows < 1)
			rows = 1;

		return rows;
	}

	protected void ApplyRequiredHeading()
	{
		if (!m_RequiredStripLabel)
			return;

		TextWidget heading = TextWidget.Cast(m_RequiredStripLabel.FindAnyWidget("RequiredStripLabelText"));
		if (!heading)
			return;

		string text = "Required";
		if (m_StripHasGiven && !m_StripHasNeeded)
			text = "Given to you";
		if (m_StripHasGiven && m_StripHasNeeded)
			text = "Required and given";

		heading.SetText(text);
	}

	protected void LayoutItemGroups()
	{
		if (!m_DialoguePanel)
			return;

		float panelW;
		float panelH;
		m_DialoguePanel.GetScreenSize(panelW, panelH);

		if (panelW <= 0 || panelH <= 0)
		{
			Print("[DialogueFramework] [UI] Item area: panel not measured yet, keeping the layout defaults.");
			return;
		}

		int required = m_RequiredTiles.Count();
		int rewards = m_RewardTiles.Count();

		if (required == 0 && rewards == 0)
		{
			LayoutResponseArea(0);
			return;
		}

		float areaTop = ITEM_AREA_TOP;
		float areaBottom = m_ScrollY + m_ScrollH - RESPONSE_MIN_FRACTION;
		float areaH = areaBottom - areaTop;

		if (areaH > ITEM_AREA_MAX_FRACTION)
			areaH = ITEM_AREA_MAX_FRACTION;

		if (areaH <= 0.02)
		{
			LayoutResponseArea(0);
			return;
		}

		float gap = 0.02;
		float halfW = (m_ScrollW - gap) / 2.0;

		if (required == 0 || rewards == 0)
		{
			if (required > 0)
				LayoutOneGroup(m_RequiredStripLabel, m_RequiredStrip, m_RequiredTiles,
					panelW, panelH, m_ScrollX, areaTop, m_ScrollW, areaH);
			else
				HideGroup(m_RequiredStripLabel, m_RequiredStrip);

			if (rewards > 0)
				LayoutOneGroup(m_RewardStripLabel, m_RewardStrip, m_RewardTiles,
					panelW, panelH, m_ScrollX, areaTop, m_ScrollW, areaH);
			else
				HideGroup(m_RewardStripLabel, m_RewardStrip);
		}
		else
		{
			LayoutOneGroup(m_RequiredStripLabel, m_RequiredStrip, m_RequiredTiles,
				panelW, panelH, m_ScrollX, areaTop, halfW, areaH);

			LayoutOneGroup(m_RewardStripLabel, m_RewardStrip, m_RewardTiles,
				panelW, panelH, m_ScrollX + halfW + gap, areaTop, halfW, areaH);
		}

		LayoutResponseArea(areaTop + areaH + 0.015);
	}

	protected void ApplyGroupLabelColor(Widget label)
	{
		if (!label || !m_MenuConfig)
			return;

		TextWidget text = TextWidget.Cast(label.FindAnyWidget("RequiredStripLabelText"));
		if (!text)
			text = TextWidget.Cast(label.FindAnyWidget("RewardStripLabelText"));

		if (text)
			text.SetColor(m_MenuConfig.GetColor(m_MenuConfig.SpeakerTextColor));
	}

	protected void HideGroup(Widget label, Widget strip)
	{
		if (label)
			label.Show(false);
		if (strip)
			strip.Show(false);
	}

	protected void LayoutOneGroup(Widget label, Widget strip, array<Widget> tiles, float panelW, float panelH, float boxX, float boxY, float boxW, float boxH)
	{
		if (!strip)
			return;

		int count = tiles.Count();
		if (count == 0)
		{
			HideGroup(label, strip);
			return;
		}

		float labelH = GROUP_LABEL_FRACTION;
		if (labelH > boxH * 0.3)
			labelH = boxH * 0.3;

		float tilesY = boxY + labelH;
		float tilesH = boxH - labelH;

		float boxWpx = boxW * panelW;
		float boxHpx = tilesH * panelH;

		float bestScale = 0;
		int bestPerRow = 1;
		int bestRows = count;

		int perRow = 1;
		while (perRow <= count)
		{
			int rows = RowsNeeded(count, perRow);

			float byWidth = (boxWpx / perRow - TILE_MARGIN * 2) / TILE_WIDTH;
			float byHeight = (boxHpx / rows - TILE_MARGIN * 2) / TILE_HEIGHT;

			float fit = byWidth;
			if (byHeight < fit)
				fit = byHeight;

			if (fit > bestScale)
			{
				bestScale = fit;
				bestPerRow = perRow;
				bestRows = rows;
			}

			perRow = perRow + 1;
		}

		if (bestScale > TILE_MAX_SCALE)
			bestScale = TILE_MAX_SCALE;
		if (bestScale < TILE_MIN_SCALE)
			bestScale = TILE_MIN_SCALE;

		float tileW = TILE_WIDTH * bestScale;
		float tileH = TILE_HEIGHT * bestScale;

		foreach (Widget tile : tiles)
		{
			if (tile)
				tile.SetSize(tileW, tileH);
		}

		float slotW = tileW + TILE_MARGIN * 2;
		float usedPx = bestPerRow * slotW;
		float stripPx = bestRows * (tileH + TILE_MARGIN * 2);

		float stripW = usedPx / panelW;
		if (stripW > boxW)
			stripW = boxW;

		float stripH = stripPx / panelH;
		if (stripH > tilesH)
			stripH = tilesH;

		float stripX = boxX + (boxW - stripW) / 2.0;

		if (label)
		{
			label.Show(true);
			label.SetPos(boxX, boxY);
			label.SetSize(boxW, labelH);
			ApplyGroupLabelColor(label);
		}

		strip.Show(true);
		strip.SetPos(stripX, tilesY);
		strip.SetSize(stripW, stripH);

		Print("[DialogueFramework] [UI] Item group: " + count + " tile(s), " + bestPerRow + " per row, " + bestRows + " row(s), scale " + bestScale + ", box " + boxWpx + "x" + boxHpx + "px, tile " + tileW + "x" + tileH + "px");
	}

	protected void LayoutResponseArea(float topY)
	{
		if (!m_ResponseScroll)
			return;

		if (topY <= 0)
		{
			m_ResponseScroll.SetPos(m_ScrollX, m_ScrollY);
			m_ResponseScroll.SetSize(m_ScrollW, m_ScrollH);
			return;
		}

		float bottom = m_ScrollY + m_ScrollH;
		float height = bottom - topY;

		if (height < RESPONSE_MIN_FRACTION)
		{
			height = RESPONSE_MIN_FRACTION;
			topY = bottom - height;
		}

		m_ResponseScroll.SetPos(m_ScrollX, topY);
		m_ResponseScroll.SetSize(m_ScrollW, height);
	}

	protected void HideRewardDisplay()
	{
		foreach (Widget tile : m_RewardDisplayWidgets)
		{
			if (tile)
				tile.Unlink();
		}
		m_RewardDisplayWidgets.Clear();

		if (m_RewardStrip)
			m_RewardStrip.Show(false);
		if (m_RewardStripLabel)
			m_RewardStripLabel.Show(false);
		if (m_RequiredStrip)
			m_RequiredStrip.Show(false);
		if (m_RequiredStripLabel)
			m_RequiredStripLabel.Show(false);

		m_RequiredTiles.Clear();
		m_RewardTiles.Clear();

		m_StripHasGiven = false;
		m_StripHasNeeded = false;
		m_StripHasReward = false;

		LayoutResponseArea(0);
	}

	protected void AddQuestResponses(DialogueNode node, array<string> texts, string defaultText, string actionType)
	{
		if (texts && texts.Count() > 0)
		{
			foreach (string text : texts)
			{
				DialogueResponse poolResponse = new DialogueResponse();
				poolResponse.Text = text;
				poolResponse.ActionType = actionType;
				node.Responses.Insert(poolResponse);
			}
		}
		else
		{
			DialogueResponse defaultResponse = new DialogueResponse();
			defaultResponse.Text = defaultText;
			defaultResponse.ActionType = actionType;
			node.Responses.Insert(defaultResponse);
		}
	}

	protected string GetDescriptionSafe(array<string> descriptions, int index)
	{
		if (!descriptions || descriptions.Count() == 0)
			return "";

		if (index < descriptions.Count() && descriptions[index] != "")
			return descriptions[index];

		return descriptions[0];
	}

	protected void AcceptActiveQuest()
	{
		if (m_ActiveQuestID != -1)
			ExpansionQuestModule.GetModuleInstance().RequestCreateQuestInstance(m_ActiveQuestID);

		m_ActiveQuestID = -1;
		EndConversation();
	}

	protected void TurnInActiveQuest()
	{
		if (m_ActiveQuestID == -1)
		{
			EndConversation();
			return;
		}

		m_SelectedObjItemIndex = -1;

		ExpansionQuestConfig quest = ExpansionQuestModule.GetModuleInstance().GetQuestConfigByID(m_ActiveQuestID);
		if (!quest)
		{
			EndConversation();
			return;
		}

		if (PrepareObjectiveItemSelection(quest))
		{
			Print("[DialogueFramework] [DIAG] TurnInActiveQuest() -- quest " + m_ActiveQuestID + " needs objective-item selection, showing picker.");
			return;
		}

		ContinueTurnIn(quest);
	}

	protected void ContinueTurnIn(ExpansionQuestConfig quest)
	{
		if (quest && quest.NeedToSelectReward() && quest.GetRewards() && quest.GetRewards().Count() > 1)
		{
			Print("[DialogueFramework] [DIAG] ContinueTurnIn() -- quest " + m_ActiveQuestID + " needs reward selection, showing picker. objItemIndex=" + m_SelectedObjItemIndex);
			ShowRewardSelection(quest);
			return;
		}

		Print("[DialogueFramework] [DIAG] ContinueTurnIn() -- turning in quest " + m_ActiveQuestID + " objItemIndex=" + m_SelectedObjItemIndex);
		ExpansionQuestModule.GetModuleInstance().RequestTurnInQuestClient(m_ActiveQuestID, false, null, m_SelectedObjItemIndex);

		m_ActiveQuestID = -1;
		EndConversation();
	}

	protected bool PrepareObjectiveItemSelection(ExpansionQuestConfig quest)
	{
		m_ObjItemChoices.Clear();
		m_ObjItemChoiceIndices.Clear();

		array<ref ExpansionQuestObjectiveConfigBase> objectives = quest.GetObjectives();
		if (!objectives)
			return false;

		ExpansionQuestPersistentData questData = ExpansionQuestModule.GetModuleInstance().GetClientQuestData();
		ExpansionQuestPersistentQuestData perQuest;
		if (questData)
			perQuest = questData.GetQuestDataByQuestID(m_ActiveQuestID);

		for (int i = 0; i < objectives.Count(); i++)
		{
			ExpansionQuestObjectiveConfigBase objective = objectives[i];
			if (!objective || objective.GetObjectiveType() != ExpansionQuestObjectiveType.COLLECT)
				continue;

			ExpansionQuestObjectiveCollectionConfig collectConfig = ExpansionQuestObjectiveCollectionConfig.Cast(objective);
			if (!collectConfig || !collectConfig.NeedAnyCollection())
				continue;

			array<ref ExpansionQuestObjectiveDelivery> collections = collectConfig.GetCollections();
			if (!collections || collections.Count() == 0)
				continue;

			if (collections.Count() == 1)
			{
				m_SelectedObjItemIndex = 0;
				return false;
			}

			ExpansionQuestObjectiveData objData;
			if (perQuest)
				objData = perQuest.GetObjectiveByIndex(i);

			for (int j = 0; j < collections.Count(); j++)
			{
				ExpansionQuestObjectiveDelivery collection = collections[j];
				if (!collection)
					continue;

				int have = 0;
				if (objData)
					have = objData.GetDeliveryCountByIndex(j);

				if (have >= collection.GetAmount())
				{
					m_ObjItemChoices.Insert(collection);
					m_ObjItemChoiceIndices.Insert(j);
				}
			}

			break;
		}

		if (m_ObjItemChoiceIndices.Count() == 0)
			return false;

		if (m_ObjItemChoiceIndices.Count() == 1)
		{
			m_SelectedObjItemIndex = m_ObjItemChoiceIndices[0];
			return false;
		}

		ShowObjectiveItemSelection();
		return true;
	}

	protected void ShowObjectiveItemSelection()
	{
		StopDialogueVoice();

		m_ShowingQuestList = false;
		m_ShowingRewardList = false;
		m_ShowingObjItemList = true;
		HideRewardDisplay();

		Print("[DialogueFramework] [DIAG] ShowObjectiveItemSelection() -- " + m_ObjItemChoices.Count() + " collection option(s).");

		if (m_SpeakerName)
			ApplySpeakerName();
		if (m_SpeakerLine)
			SetSpeakerLine(PickObjItemPrompt());

		ClearButtons();

		for (int c = 0; c < m_ObjItemChoices.Count(); c++)
		{
			ExpansionQuestObjectiveDelivery choice = m_ObjItemChoices[c];
			if (choice)
				m_ResponseButtons.Insert(CreateItemChoiceButton(choice.GetClassName(), choice.GetAmount()));
		}
	}

	protected string PickObjItemPrompt()
	{
		return "Which will you hand over?";
	}

	protected int m_PendingObjItemChoice = -1;

	protected void ExecutePendingObjItemSelection()
	{
		int choice = m_PendingObjItemChoice;
		m_PendingObjItemChoice = -1;
		if (choice < 0 || choice >= m_ObjItemChoiceIndices.Count())
			return;

		m_SelectedObjItemIndex = m_ObjItemChoiceIndices[choice];
		m_ShowingObjItemList = false;

		Print("[DialogueFramework] [DIAG] ExecutePendingObjItemSelection() -- chose collection index " + m_SelectedObjItemIndex);

		ExpansionQuestConfig quest = ExpansionQuestModule.GetModuleInstance().GetQuestConfigByID(m_ActiveQuestID);
		ContinueTurnIn(quest);
	}

	protected void ShowRewardSelection(ExpansionQuestConfig quest)
	{
		StopDialogueVoice();

		m_ShowingQuestList = false;
		m_ShowingRewardList = true;
		m_ShowingObjItemList = false;
		HideRewardDisplay();

		array<ref ExpansionQuestRewardConfig> configuredRewards = quest.GetRewards();

		m_CurrentRewards.Clear();
		foreach (ExpansionQuestRewardConfig configuredReward : configuredRewards)
		{
			if (configuredReward)
				m_CurrentRewards.Insert(configuredReward);
		}

		Print("[DialogueFramework] [DIAG] ShowRewardSelection() -- " + m_CurrentRewards.Count() + " reward option(s).");

		if (m_SpeakerName)
			ApplySpeakerName();
		if (m_SpeakerLine)
			SetSpeakerLine(PickRewardPrompt());

		ClearButtons();
		m_SelectedRewardIndex = -1;

		for (int r = 0; r < m_CurrentRewards.Count(); r++)
			m_ResponseButtons.Insert(CreateRewardButton(m_CurrentRewards[r]));
	}

	protected Widget CreateRewardButton(ExpansionQuestRewardConfig reward)
	{
		if (!reward)
			return null;

		return CreateItemChoiceButton(reward.GetClassName(), reward.GetAmount());
	}

	protected Widget CreateItemChoiceButton(string className, int amount)
	{
		if (!m_ResponseList)
			return null;

		Widget button = GetGame().GetWorkspace().CreateWidgets(LayoutPath("dialogue_reward_button"), m_ResponseList);
		if (!button)
			return null;

		TextWidget nameLabel = TextWidget.Cast(button.FindAnyWidget("DialogueRewardName"));
		if (nameLabel)
			nameLabel.SetText(GetItemDisplayName(className));

		TextWidget amountLabel = TextWidget.Cast(button.FindAnyWidget("DialogueRewardAmount"));
		if (amountLabel)
			amountLabel.SetText("x" + amount);

		ItemPreviewWidget preview = ItemPreviewWidget.Cast(button.FindAnyWidget("DialogueRewardPreview"));
		if (preview)
		{
			EntityAI previewEntity = EntityAI.Cast(GetGame().CreateObjectEx(className, vector.Zero, ECE_LOCAL|ECE_NOLIFETIME));
			if (previewEntity)
			{
				m_RewardPreviewObjects.Insert(previewEntity);
				preview.SetItem(previewEntity);
			}
			else
			{
				Print("[DialogueFramework] [REWARD] Could not create preview entity for '" + className + "' -- thumbnail will be blank.");
			}
		}

		Widget rewardBackground = button.FindAnyWidget("DialogueRewardButtonBackground");
		if (rewardBackground && m_MenuConfig)
			rewardBackground.SetColor(m_MenuConfig.GetColor(m_MenuConfig.ResponseBackgroundColor));

		if (nameLabel && m_MenuConfig)
			nameLabel.SetColor(m_MenuConfig.GetColor(m_MenuConfig.ResponseTextColor));

		ApplyBorderColor(button);

		button.SetUserID(m_ResponseButtons.Count());
		return button;
	}

	protected string GetItemDisplayName(string className)
	{
		string displayName = "";

		if (GetGame().ConfigIsExisting("CfgVehicles " + className))
			displayName = GetGame().ConfigGetTextOut("CfgVehicles " + className + " displayName");

		if (displayName == "" && GetGame().ConfigIsExisting("CfgMagazines " + className))
			displayName = GetGame().ConfigGetTextOut("CfgMagazines " + className + " displayName");

		if (displayName == "" && GetGame().ConfigIsExisting("CfgWeapons " + className))
			displayName = GetGame().ConfigGetTextOut("CfgWeapons " + className + " displayName");

		if (displayName == "")
		{
			Print("[DialogueFramework] [ITEM] No displayName in config for '" + className + "' -- falling back to the class name.");
			displayName = className;
		}

		return ShortenForTile(displayName);
	}

	protected string ShortenForTile(string text)
	{
		if (text.Length() <= TILE_NAME_MAX)
			return text;

		return text.Substring(0, TILE_NAME_MAX - 1) + ".";
	}

	protected void SelectReward(int index)
	{
		m_SelectedRewardIndex = index;

		for (int i = 0; i < m_ResponseButtons.Count(); i++)
		{
			Widget entry = m_ResponseButtons[i];
			if (!entry)
				continue;

			Widget background = entry.FindAnyWidget("DialogueRewardButtonBackground");
			if (!background)
				continue;

			if (i == index)
				background.SetColor(m_MenuConfig.GetColor(m_MenuConfig.RewardSelectedColor));
			else
				background.SetColor(m_MenuConfig.GetColor(m_MenuConfig.ResponseBackgroundColor));
		}
	}

	protected void ShowRewardConfirm(int index)
	{
		if (index < 0 || index >= m_CurrentRewards.Count())
			return;

		ExpansionQuestRewardConfig reward = m_CurrentRewards[index];
		if (!reward)
			return;

		m_SelectedRewardIndex = index;

		if (m_ConfirmText)
			m_ConfirmText.SetText("Take " + GetItemDisplayName(reward.GetClassName()) + " x" + reward.GetAmount() + " and hand the job in?");

		if (m_ConfirmPanel)
			m_ConfirmPanel.Show(true);
	}

	protected void HideRewardConfirm()
	{
		if (m_ConfirmPanel)
			m_ConfirmPanel.Show(false);
	}

	protected string PickRewardPrompt()
	{
		if (m_ActiveQuestID != -1)
		{
			DialogueQuestText questText = DialogueManager.GetInstance().GetQuestText(m_ActiveQuestID);
			if (questText && questText.RewardSelectText != "")
				return questText.RewardSelectText;
		}

		return "Take your pick.";
	}

	protected string GetRewardDisplayText(ExpansionQuestRewardConfig reward)
	{
		if (!reward)
			return "";

		string className = reward.GetClassName();
		string displayName = "";

		if (GetGame().ConfigIsExisting("CfgVehicles " + className))
			displayName = GetGame().ConfigGetTextOut("CfgVehicles " + className + " displayName");

		if (displayName == "" && GetGame().ConfigIsExisting("CfgMagazines " + className))
			displayName = GetGame().ConfigGetTextOut("CfgMagazines " + className + " displayName");

		if (displayName == "" && GetGame().ConfigIsExisting("CfgWeapons " + className))
			displayName = GetGame().ConfigGetTextOut("CfgWeapons " + className + " displayName");

		if (displayName == "")
			displayName = className;

		int amount = reward.GetAmount();
		if (amount > 1)
			return displayName + " x" + amount;

		return displayName;
	}

	protected void OnRewardSelected(ExpansionQuestRewardConfig reward)
	{
		if (!reward)
			return;

		m_PendingReward = reward;
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(ExecutePendingRewardSelection, 0, false);
	}

	protected ExpansionQuestRewardConfig m_PendingReward;

	protected void ExecutePendingRewardSelection()
	{
		ExpansionQuestRewardConfig reward = m_PendingReward;
		m_PendingReward = null;
		if (!reward)
			return;

		Print("[DialogueFramework] [DIAG] ExecutePendingRewardSelection() -- turning in quest " + m_ActiveQuestID + " with reward " + reward.GetClassName() + " objItemIndex=" + m_SelectedObjItemIndex);

		if (m_ActiveQuestID != -1)
			ExpansionQuestModule.GetModuleInstance().RequestTurnInQuestClient(m_ActiveQuestID, true, reward, m_SelectedObjItemIndex);

		m_ActiveQuestID = -1;
		m_ShowingRewardList = false;
		EndConversation();
	}

	protected void EndConversation()
	{
		Object npc = GetTalkingNPC();

		if (m_ActiveTree && npc)
		{
			array<string> farewellPool = m_ActiveTree.FarewellVoiceLineIDs;
			if (farewellPool)
				PlayRandomVoiceLineOnObject(farewellPool, npc);
		}

		GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(CloseDeferred, 0, false, this);
	}

	protected void CloseDeferred(DialogueWindowMenu window)
	{
		if (!window)
			return;

		bool openTrader = window.IsOpeningTrader();

		window.Close();
		DialogueWindowLauncher.GetInstance().ReleaseWindow(window);

	#ifdef EXPANSIONMODMARKET
		if (openTrader)
		{
			DialogueTraderSession.OpenMarketForCurrentTrader();
		}
	#endif
	}

	bool IsOpeningTrader()
	{
		return m_OpeningTrader;
	}

	protected void PlayRandomVoiceLine(array<string> pool)
	{
		Object npc = GetTalkingNPC();
		if (!npc)
			return;

		PlayRandomVoiceLineOnObject(pool, npc);
	}

	protected void PlayRandomVoiceLineOnObject(array<string> pool, Object npc)
	{
		if (!pool || pool.Count() == 0)
			return;

		string voiceLineID = pool[Math.RandomInt(0, pool.Count())];
		PlayVoiceLineID(voiceLineID, npc);
	}

	protected void PlayVoiceLineID(string voiceLineID, Object npc)
	{
		if (voiceLineID == "" || !npc)
			return;

		string soundSet = "DialogueFW_" + voiceLineID + "_SoundSet";
		if (!GetGame().ConfigIsExisting("CfgSoundSets " + soundSet))
		{
			Print("[DialogueFramework] [VOICE] Missing SoundSet '" + soundSet + "' -- no audio for voice line ID '" + voiceLineID + "'.");
			return;
		}

		m_DialogueVoiceSound = SEffectManager.PlaySoundOnObject(soundSet, npc);
		if (!m_DialogueVoiceSound)
			return;

		m_DialogueVoiceSound.SetSoundAutodestroy(true);
		m_DialogueVoiceSound.SetSoundFadeOut(0.5);

		StartTalkAnimation(npc);
	}

	protected void StartTalkAnimation(Object npc)
	{
		if (!npc || !m_DialogueVoiceSound)
			return;

		AbstractWave wave = m_DialogueVoiceSound.DialogueFW_GetAbstractWave();
		if (!wave)
			return;

		float clipLength = wave.GetLength();
		if (clipLength <= 0)
			return;

		m_TalkingNPC = ExpansionNPCBase.Cast(npc);
		if (m_TalkingNPC)
		{
			m_TalkingNPC.DialogueFW_SetTalkingClient(clipLength);
			return;
		}

	#ifdef EXPANSIONMODAI

		m_TalkingNPCAI = eAIBase.Cast(npc);
		if (m_TalkingNPCAI)
			m_TalkingNPCAI.DialogueFW_SetTalkingClient(clipLength);
	#endif
	}

	protected void StopTalkAnimation()
	{
		if (m_TalkingNPC)
		{
			m_TalkingNPC.DialogueFW_SetTalkingClient(-1);
			m_TalkingNPC = null;
		}

	#ifdef EXPANSIONMODAI
		if (m_TalkingNPCAI)
		{
			m_TalkingNPCAI.DialogueFW_SetTalkingClient(-1);
			m_TalkingNPCAI = null;
		}
	#endif
	}

	protected void StopDialogueVoice()
	{
		if (m_DialogueVoiceSound)
			m_DialogueVoiceSound.Stop();

		StopTalkAnimation();
	}

	protected Object GetTalkingNPC()
	{
	#ifdef EXPANSIONMODAI
		if (m_TargetAI)
			return m_TargetAI;
	#endif

		if (m_NPCID == -1)
			return null;

		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		if (!player)
			return null;

		array<Object> nearbyObjects = new array<Object>;
		GetGame().GetObjectsAtPosition3D(player.GetPosition(), 5.0, nearbyObjects, NULL);

		ExpansionQuestNPCBase npc;
		ExpansionQuestStaticObject npcObject;
	#ifdef EXPANSIONMODAI
		ExpansionQuestNPCAIBase npcAI;
	#endif

		foreach (Object obj : nearbyObjects)
		{
			npc = ExpansionQuestNPCBase.Cast(obj);
			if (npc && npc.GetQuestNPCID() == m_NPCID)
				return obj;

		#ifdef EXPANSIONMODAI
			npcAI = ExpansionQuestNPCAIBase.Cast(obj);
			if (npcAI && npcAI.GetQuestNPCID() == m_NPCID)
				return obj;
		#endif

			npcObject = ExpansionQuestStaticObject.Cast(obj);
			if (npcObject && npcObject.GetQuestNPCID() == m_NPCID)
				return obj;
		}

		return null;
	}

	protected string IconForResponse(DialogueResponse response)
	{
		if (!response)
			return ICON_CHAT;

		if (response.ActionType == DialogueActionType.OPEN_TRADER)
			return ICON_CART;

		if (response.ActionType == DialogueActionType.END_CONVERSATION)
			return ICON_EXIT;

		if (response.ActionType == DialogueActionType.RECRUIT_AI)
			return ICON_EXIT;

		if (response.ActionType == DialogueActionType.GO_HOSTILE)
			return ICON_EXIT;

		if (response.ActionType == DialogueActionType.NONE && response.NextNodeID == -1)
			return ICON_EXIT;

		return ICON_CHAT;
	}

	protected void ApplyResponseIcon(Widget button, string iconName)
	{
		ImageWidget icon = ImageWidget.Cast(button.FindAnyWidget("DialogueResponseButtonIcon"));

		if (!m_IconDiagLogged)
		{
			m_IconDiagLogged = true;
			string haveConfig = "NO";
			string configWants = "false";
			if (m_MenuConfig)
			{
				haveConfig = "YES";
				if (m_MenuConfig.ShowResponseIcons)
					configWants = "true";
			}

			string widgetFound = "NO";
			if (icon)
				widgetFound = "YES";

			Print("[DialogueFramework] [ICONS] menuConfigReceived=" + haveConfig + " ShowResponseIcons=" + configWants + " iconWidgetFound=" + widgetFound + " firstIcon=" + iconName + " path=" + ICON_FOLDER + iconName + ICON_EXT);

			if (!icon)
				Print("[DialogueFramework] [ICONS] DialogueResponseButtonIcon not found in the layout -- the .layout in the built PBO is older than the scripts. Rebuild.");
			if (haveConfig == "YES" && configWants == "false")
				Print("[DialogueFramework] [ICONS] The client received ShowResponseIcons=false. Check MenuConfig.json on the SERVER, then restart the server AND fully restart the client.");
		}

		if (!icon)
			return;

		bool wanted = false;
		if (m_MenuConfig && m_MenuConfig.ShowResponseIcons)
			wanted = true;

		if (!wanted || iconName == "")
		{
			icon.Show(false);
			return;
		}

		icon.LoadImageFile(0, ICON_FOLDER + iconName + ICON_EXT);
		icon.SetImage(0);
		if (m_MenuConfig)
			icon.SetColor(m_MenuConfig.GetColor(m_MenuConfig.ResponseTextColor));
		icon.Show(true);

		TextWidget label = TextWidget.Cast(button.FindAnyWidget("DialogueResponseButtonText"));
		if (label)
			label.SetSize(0.85, 1.0);
	}

	protected Widget CreateResponseButton(string text, bool visited = false, string iconName = "")
	{
		if (!m_ResponseList)
			return null;

		Widget button = GetGame().GetWorkspace().CreateWidgets(LayoutPath("dialogue_response_button"), m_ResponseList);
		if (!button)
			return null;

		TextWidget label = TextWidget.Cast(button.FindAnyWidget("DialogueResponseButtonText"));
		if (label)
		{
			label.SetText(text);
			if (m_MenuConfig)
			{
				if (visited)
					label.SetColor(m_MenuConfig.GetFadedColor(m_MenuConfig.ResponseTextColor, m_MenuConfig.VisitedResponseOpacity));
				else
					label.SetColor(m_MenuConfig.GetColor(m_MenuConfig.ResponseTextColor));
			}
		}

		Widget buttonBackground = button.FindAnyWidget("DialogueResponseButtonBackground");
		if (buttonBackground && m_MenuConfig)
			buttonBackground.SetColor(m_MenuConfig.GetColor(m_MenuConfig.ResponseBackgroundColor));

		ApplyResponseIcon(button, iconName);
		ApplyBorderColor(button);

		button.SetUserID(m_ResponseButtons.Count());

		return button;
	}

	protected void ResetIconDiagnostics()
	{
		m_IconDiagLogged = false;
	}

	protected void ClearButtons()
	{
		foreach (Widget w : m_ResponseButtons)
		{
			if (w)
				w.Unlink();
		}
		m_ResponseButtons.Clear();

		foreach (Widget rewardTile : m_RewardDisplayWidgets)
		{
			if (rewardTile)
				rewardTile.Unlink();
		}
		m_RewardDisplayWidgets.Clear();

		DestroyRewardPreviews();

		if (m_ResponseScroll)
			m_ResponseScroll.VScrollToPos01(0);
	}

	protected void DestroyRewardPreviews()
	{
		foreach (EntityAI previewEntity : m_RewardPreviewObjects)
		{
			if (previewEntity)
				GetGame().ObjectDelete(previewEntity);
		}
		m_RewardPreviewObjects.Clear();
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (!w)
			return false;

		Print("[DialogueFramework] [DIAG] OnClick fired. userID=" + w.GetUserID() + " showingQuestList=" + m_ShowingQuestList);

		if (w == m_CloseButton)
		{
			Print("[DialogueFramework] [DIAG] OnClick -> Close button.");
			EndConversation();
			return true;
		}

		if (m_ConfirmPanel && m_ConfirmPanel.IsVisible())
		{
			if (w == m_ConfirmYesButton)
			{
				Print("[DialogueFramework] [DIAG] OnClick -> reward confirmed, idx=" + m_SelectedRewardIndex);
				HideRewardConfirm();
				if (m_SelectedRewardIndex >= 0 && m_SelectedRewardIndex < m_CurrentRewards.Count())
					OnRewardSelected(m_CurrentRewards[m_SelectedRewardIndex]);
				return true;
			}

			if (w == m_ConfirmNoButton)
			{
				Print("[DialogueFramework] [DIAG] OnClick -> reward confirm cancelled.");
				HideRewardConfirm();
				return true;
			}

			return true;
		}

		int idx = w.GetUserID();
		if (idx < 0)
			return false;

		if (m_ShowingObjItemList)
		{
			if (idx >= m_ObjItemChoiceIndices.Count())
				return true;

			Print("[DialogueFramework] [DIAG] OnClick -> objective item chosen idx=" + idx);
			m_PendingObjItemChoice = idx;
			GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(ExecutePendingObjItemSelection, 0, false);
		}
		else if (m_ShowingRewardList)
		{
			if (idx >= m_CurrentRewards.Count())
				return true;

			Print("[DialogueFramework] [DIAG] OnClick -> reward highlighted idx=" + idx);
			SelectReward(idx);
		}
		else if (m_ShowingQuestList)
		{
			if (idx >= m_CurrentQuests.Count())
			{
				Print("[DialogueFramework] [DIAG] OnClick -> quest list back to conversation.");
				ReturnToRootDeferred();
				return true;
			}

			Print("[DialogueFramework] [DIAG] OnClick -> quest selected idx=" + idx);
			OnQuestSelectedFromList(m_CurrentQuests[idx]);
		}
		else
		{
			if (idx >= m_CurrentResponses.Count())
				return true;

			if (m_ActiveNode)
				m_VisitedResponses.Set(m_ActiveNode.ID.ToString() + ":" + idx, true);

			Print("[DialogueFramework] [DIAG] OnClick -> response idx=" + idx + " actionType=" + m_CurrentResponses[idx].ActionType + " nextNode=" + m_CurrentResponses[idx].NextNodeID);
			OnDialogueResponseSelected(m_CurrentResponses[idx]);
		}

		Print("[DialogueFramework] [DIAG] OnClick handling returned.");
		return true;
	}

	override bool OnDoubleClick(Widget w, int x, int y, int button)
	{
		if (!w || !m_ShowingRewardList)
			return false;

		if (m_ConfirmPanel && m_ConfirmPanel.IsVisible())
			return false;

		int idx = w.GetUserID();
		if (idx < 0 || idx >= m_CurrentRewards.Count())
			return false;

		Print("[DialogueFramework] [DIAG] OnDoubleClick -> reward confirm idx=" + idx);
		SelectReward(idx);
		ShowRewardConfirm(idx);
		return true;
	}

	override bool OnMouseEnter(Widget w, int x, int y)
	{
		SetHoverBorder(w, true);
		return false;
	}

	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		SetHoverBorder(w, false);
		return false;
	}

	protected void SetHoverBorder(Widget w, bool state)
	{
		if (!w)
			return;

		Widget border = w.FindAnyWidget("DialogueResponseButtonBorder");
		if (!border)
			border = w.FindAnyWidget("DialogueRewardBorder");

		if (border)
			border.Show(state);
	}

	protected bool m_ContentInitialized = false;

	override void OnShow()
	{
		super.OnShow();
		LockPlayerMovement();
		ResetIconDiagnostics();

	#ifdef EXPANSIONMODAI
		if (m_TargetAI)
			m_TargetAI.DialogueFW_RequestHoldClient(true);
	#endif

		SetFocus(layoutRoot);

		Print("[DialogueFramework] [DIAG] DialogueWindowMenu.OnShow() fired. ContentInitialized=" + m_ContentInitialized);

		if (!m_ContentInitialized)
		{
			m_ContentInitialized = true;
			OpenRootNode();
		}
	}

	override void OnHide()
	{
		Print("[DialogueFramework] [DIAG] DialogueWindowMenu.OnHide() fired.");
		StopDialogueVoice();

		ClearButtons();

	#ifdef EXPANSIONMODAI
		if (m_TargetAI)
			m_TargetAI.DialogueFW_RequestHoldClient(false);
	#endif

		UnlockPlayerMovement();
		super.OnHide();
	}

	void ~DialogueWindowMenu()
	{
		Print("[DialogueFramework] [DIAG] ~DialogueWindowMenu() destructor fired.");

		DestroyRewardPreviews();
	}

	protected void LockPlayerMovement()
	{
		GetGame().GetUIManager().ShowUICursor(true);

		IngameHud hud = IngameHud.Cast(GetGame().GetMission().GetHud());
		if (hud && hud.GetHudPanelWidget())
		{
			hud.ShowQuickbarUI(false);
			hud.ShowHudUI(false);
			hud.ShowCursor();
		}

		TIntArray inputIDs = new TIntArray;
		GetUApi().GetActiveInputs(inputIDs);

		int skipID = GetUApi().GetInputByName("UAUIBack").ID();

		foreach (int inputID : inputIDs)
		{
			if (inputID != skipID)
				GetUApi().GetInputByID(inputID).ForceDisable(true);
		}

		GetUApi().UpdateControls();
	}

	protected void UnlockPlayerMovement()
	{
		GetGame().GetUIManager().ShowUICursor(false);

		IngameHud hud = IngameHud.Cast(GetGame().GetMission().GetHud());
		if (hud && hud.GetHudPanelWidget())
		{
			hud.ShowQuickbarUI(true);
			hud.ShowHudUI(true);
			hud.HideCursor();
		}

		TIntArray inputIDs = new TIntArray;
		GetUApi().GetActiveInputs(inputIDs);
		foreach (int inputID : inputIDs)
			GetUApi().GetInputByID(inputID).ForceDisable(false);

		GetUApi().UpdateControls();
	}
}
