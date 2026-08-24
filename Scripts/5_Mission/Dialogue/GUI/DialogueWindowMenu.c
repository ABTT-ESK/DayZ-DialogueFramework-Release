class DialogueWindowMenu : UIScriptedMenu
{
	protected TextWidget m_SpeakerName;
	protected RichTextWidget m_SpeakerLine;
	protected WrapSpacerWidget m_ResponseList;
	protected Widget m_CloseButton;
	protected Widget m_SettingsButton;

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
	protected static const int LANGUAGE_NODE_ID = -1001;

	protected int m_StageIndex = -1;

	//! Scroll position is tracked here because the engine exposes a setter
	//! (VScrollToPos01) but no getter.
	protected float m_ScrollPos01;
	protected float m_ContentHeightPx;

	//! Buttons that carry wrapping text, with what they say, so the sizing
	//! can be re-applied a frame later once the spacer has had its own pass.
	protected ref array<Widget> m_SizedButtons;
	protected ref array<string> m_SizedTexts;
	protected ref array<bool> m_SizedIcons;


	//! The player's own settings screen. Rows are identified by these, so the
	//! order they appear in can change without touching the click handling.
	protected static const string SETTING_LANGUAGE = "LANGUAGE";
	protected static const string SETTING_POSITION = "POSITION";
	protected static const string SETTING_TEXTSIZE = "TEXTSIZE";
	protected static const string SETTING_ICONS = "ICONS";
	protected static const string SETTING_RESET = "RESET";

	protected bool m_ShowingSettings;
	protected ref array<string> m_SettingRows;

	protected static const string ICON_FOLDER = "DialogueFramework/GUI/images/";

	protected static const string ICON_EXT = ".paa";
	protected static const string ICON_EXIT = "icon_exit_ca";
	protected static const string ICON_CHAT = "icon_chat_ca";
	protected static const string ICON_CART = "icon_cart_ca";

	protected bool m_IconDiagLogged;
	protected bool m_LabelCastWarned;

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

	protected static const float RESPONSE_FONT_PX = 18;
	protected static const float RESPONSE_FONT_PX_LARGE = 22;
	protected static const float RESPONSE_FONT_PX_COMPACT = 15;

	protected static const int RESPONSE_GROW_LINES = 3;

	protected static const float RESPONSE_MIN_FONT_PX = 10;

	//! Measured off the real render, not assumed: at font 18 in a 1050px
	//! column, Metron fits ~88 characters, i.e. ~11.9px per character. The
	//! old 0.5 said ~107 and produced buttons a whole line too short, which
	//! is why text spilled over the edges.
	protected static const float RESPONSE_CHAR_RATIO = 0.70;
	protected static const float RESPONSE_LINE_SPACING = 1.35;
	protected static const float RESPONSE_VPAD_PX = 18;
	protected static const float RESPONSE_MIN_HEIGHT_PX = 40;

	protected static const float RESPONSE_WRAP_SAFETY = 0.92;

	//! One wheel notch moves about one short option. The engine's own step is
	//! a fixed fraction of the content, which skips whole options now that
	//! they are not all the same height.
	protected static const float RESPONSE_SCROLL_STEP_PX = 45;

	protected static const float PANEL_REFERENCE_WIDTH = 0.6;
	protected static const float PANEL_SCALE_MIN = 0.6;
	protected static const float PANEL_SCALE_MAX = 1.8;

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
		m_SettingRows = new array<string>;
		m_SizedButtons = new array<Widget>;
		m_SizedTexts = new array<string>;
		m_SizedIcons = new array<bool>;
	}

	//! A WrapSpacer lays its children out once. Buttons that grew taller after
	//! that are drawn overlapping the next one, so the spacer has to be told
	//! to re-flow after the list is built.
	protected void RefreshResponseList()
	{
		if (m_ResponseList)
			m_ResponseList.Update();

		if (m_ResponseScroll)
			m_ResponseScroll.Update();

		//! The spacer lays out after this returns and can stamp its own height
		//! back onto a child. Re-applying a frame later lands after that pass.
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(ReapplyResponseSizes);
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(ReapplyResponseSizes, 1, false);
	}

	protected void ReapplyResponseSizes()
	{
		if (!m_SizedButtons || m_SizedButtons.Count() == 0)
			return;

		float total = 0;

		for (int i = 0; i < m_SizedButtons.Count(); i++)
		{
			Widget sized = m_SizedButtons[i];
			if (!sized)
				continue;

			TextWidget sizedLabel = TextWidget.Cast(sized.FindAnyWidget("DialogueResponseButtonText"));
			if (!sizedLabel)
				continue;

			total = total + SizeResponseButton(sized, sizedLabel, m_SizedTexts[i], m_SizedIcons[i]);
		}

		m_ContentHeightPx = total;

		if (m_ResponseList)
			m_ResponseList.Update();

		if (m_ResponseScroll)
			m_ResponseScroll.Update();
	}

	//! A short toast so the player isn't left staring at a window that closed
	//! for no visible reason. The full reason always goes to the log as well --
	//! this is the player-facing half, deliberately non-technical.
	protected void NotifyPlayer(string bodyKey, string bodyFallback, bool isError)
	{
		if (isError && m_MenuConfig && !m_MenuConfig.ShowErrorNotifications)
			return;

		//! Built one at a time on purpose: two freshly-returned strings in one
		//! expression alias each other, which is what made every settings row
		//! read "server's choice: server's choice".
		string title = UIText("#STR_DIALOGUEFW_NOTIFY_TITLE", "Dialogue");
		string body = UIText(bodyKey, bodyFallback);

		StringLocaliser titleText = new StringLocaliser(title);
		StringLocaliser bodyText = new StringLocaliser(body);

		int colour = COLOR_EXPANSION_NOTIFICATION_INFO;
		if (isError)
			colour = COLOR_EXPANSION_NOTIFICATION_ERROR;

		ExpansionNotification(titleText, bodyText, ExpansionIcons.GetPath("Exclamationmark"), colour, 7, ExpansionNotificationType.TOAST).Create();
	}

	protected bool m_StringTableWarned;

	protected string UIText(string key, string fallback)
	{
		string text = Widget.TranslateString(key);

		//! With no stringtable in the PBO the engine hands the key straight
		//! back. Showing a player "STR_DIALOGUEFW_HEAD_REWARD" is worse than
		//! showing them plain English, so fall back and say so once in the log.
		if (text == "" || text.IndexOf("STR_DIALOGUEFW_") != -1)
		{
			if (!m_StringTableWarned)
			{
				m_StringTableWarned = true;
				Print("[DialogueFramework] [UI] [ERROR] stringtable.csv is missing from the built PBO -- falling back to English, and automatic language detection cannot work. Add *.csv to your packing tool's copy/include list and repack.");
			}

			return fallback;
		}

		return text;
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
		m_SettingsButton = layoutRoot.FindAnyWidget("SettingsButton");
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

		ApplyConfirmButtonLabels();

		if (m_ConfirmPanel)
			m_ConfirmPanel.Show(false);

		ApplyMenuConfig();

		string diagTreeID = "NULL";
		if (m_ActiveTree)
			diagTreeID = "" + m_ActiveTree.ID;
		Print("[DialogueFramework] [DIAG] DialogueWindowMenu.Init() for NPC ID=" + m_NPCID + ", tree ID=" + diagTreeID);

		return layoutRoot;
	}

	protected void ApplyConfirmButtonLabels()
	{
		if (!layoutRoot)
			return;

		TextWidget yesLabel = TextWidget.Cast(layoutRoot.FindAnyWidget("ConfirmYesLabel"));
		if (yesLabel)
			yesLabel.SetText(UIText("#STR_DIALOGUEFW_CONFIRM_YES", "Confirm"));

		TextWidget noLabel = TextWidget.Cast(layoutRoot.FindAnyWidget("ConfirmNoLabel"));
		if (noLabel)
			noLabel.SetText(UIText("#STR_DIALOGUEFW_CONFIRM_NO", "Cancel"));
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

			//! A player who has moved the window keeps it where they put it,
			//! whatever the server prefers. Size and colours stay the server's.
			string playerPosition = DialogueClientSettings.Get().Position;
			string serverPosition = m_MenuConfig.Position;
			if (playerPosition != "")
				m_MenuConfig.Position = playerPosition;

			m_MenuConfig.GetResolvedPosition(posX, posY);
			m_MenuConfig.Position = serverPosition;

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
		ApplySettingsButtonVisibility();
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
		m_StageIndex = -1;

		if (!m_ActiveTree.Stages || m_ActiveTree.Stages.Count() == 0)
			return;

		ExpansionQuestPersistentData questData = ExpansionQuestModule.GetModuleInstance().GetClientQuestData();

		DialogueTreeStage best = null;
		int bestPriority = 0;
		int bestQuestID = -1;
		int bestIndex = -1;

		for (int stageIdx = 0; stageIdx < m_ActiveTree.Stages.Count(); stageIdx++)
		{
			DialogueTreeStage stage = m_ActiveTree.Stages[stageIdx];
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
				bestIndex = stageIdx;
			}
		}

		if (best)
		{
			m_StageNodes = best.Nodes;
			m_StageRootNodeID = best.RootNodeID;
			m_StageIndex = bestIndex;
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

		int tierIndex = DialogueRepTierList.LabelIndexFor(m_ActiveTree.ReputationTiers, value);
		if (tierIndex >= 0)
		{
			string label = m_ActiveTree.ReputationTiers[tierIndex].Label;
			if (label != "")
				return DialogueLoc.ForTree(m_ActiveTree, DialogueLocKeys.TreeList("ReputationTiers", tierIndex), label);
		}

		return string.Format(UIText("#STR_DIALOGUEFW_REPUTATION", "Reputation: %1"), value.ToString());
	}

	protected bool IsAuthoredNode(DialogueNode node)
	{
		if (!node)
			return false;

		if (node.Type != DialogueNodeType.STANDARD)
			return false;

		return node.ID != NO_QUESTS_NODE_ID && node.ID != LANGUAGE_NODE_ID;
	}

	protected string PickTreeLine(array<string> pool, string field)
	{
		if (!pool || pool.Count() == 0)
			return "";

		int index = Math.RandomInt(0, pool.Count());
		return DialogueLoc.ForTree(m_ActiveTree, DialogueLocKeys.TreeList(field, index), pool[index]);
	}

	protected string PickQuestLine(int questID, array<string> pool, string field)
	{
		if (!pool || pool.Count() == 0)
			return "";

		int index = Math.RandomInt(0, pool.Count());
		return DialogueLoc.ForQuest(questID, DialogueLocKeys.QuestList(field, index), pool[index]);
	}

	protected array<string> LocalizeTreeList(array<string> pool, string field)
	{
		array<string> result = new array<string>;

		if (!pool)
			return result;

		for (int i = 0; i < pool.Count(); i++)
			result.Insert(DialogueLoc.ForTree(m_ActiveTree, DialogueLocKeys.TreeList(field, i), pool[i]));

		return result;
	}

	protected array<string> LocalizeQuestList(int questID, array<string> pool, string field)
	{
		array<string> result = new array<string>;

		if (!pool)
			return result;

		for (int i = 0; i < pool.Count(); i++)
			result.Insert(DialogueLoc.ForQuest(questID, DialogueLocKeys.QuestList(field, i), pool[i]));

		return result;
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

	//! The names here are the ones in docs/SCREENS.md. When someone reports a
	//! problem with "the quest turn-in screen", this is what lets you find it.
	protected void LogScreen(string screenName)
	{
		Print("[DialogueFramework] [SCREEN] " + screenName);
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
		m_ShowingSettings = false;
		m_SelectedRewardIndex = -1;
		HideRewardConfirm();
		HideRewardDisplay();

		bool authored = IsAuthoredNode(node);

		array<string> visibleTexts = new array<string>;
		array<ref DialogueResponse> visible = GetVisibleResponses(node, authored, visibleTexts);

		string spokenText = node.SpeakerText;
		if (authored)
			spokenText = DialogueLoc.ForTree(m_ActiveTree, DialogueLocKeys.NodeSpeaker(m_StageIndex, node.ID), spokenText);

		array<string> spokenVoicePool = node.VoiceLineIDs;

		int chosenLineIndex = -1;
		DialogueSpeakerLine chosenLine = PickSpeakerLine(node, chosenLineIndex);
		if (chosenLine)
		{
			spokenText = chosenLine.Text;
			if (authored)
				spokenText = DialogueLoc.ForTree(m_ActiveTree, DialogueLocKeys.NodeSpeakerLine(m_StageIndex, node.ID, chosenLineIndex), spokenText);

			if (chosenLine.VoiceLineIDs && chosenLine.VoiceLineIDs.Count() > 0)
				spokenVoicePool = chosenLine.VoiceLineIDs;
		}

		if (m_SpeakerName)
			ApplySpeakerName();
		if (m_SpeakerLine)
			SetSpeakerLine(spokenText);

		LogScreen("Conversation screen");
		ClearButtons();

		m_CurrentResponses = visible;
		for (int i = 0; i < visible.Count(); i++)
		{
			bool wasVisited = m_VisitedResponses.Contains(node.ID.ToString() + ":" + i);
			m_ResponseButtons.Insert(CreateResponseButton(visibleTexts[i], wasVisited, IconForResponse(visible[i])));
		}


		RefreshResponseList();

		StopDialogueVoice();

		if (spokenVoicePool)
			PlayRandomVoiceLine(spokenVoicePool);
	}

	protected array<ref DialogueResponse> GetVisibleResponses(DialogueNode node, bool authored, out array<string> texts)
	{
		array<ref DialogueResponse> visible = new array<ref DialogueResponse>;

		if (!texts)
			texts = new array<string>;

		if (!node.Responses)
			return visible;

		for (int i = 0; i < node.Responses.Count(); i++)
		{
			DialogueResponse response = node.Responses[i];
			if (!response || !PassesGating(response))
				continue;

			visible.Insert(response);

			if (authored)
				texts.Insert(DialogueLoc.ForTree(m_ActiveTree, DialogueLocKeys.NodeResponse(m_StageIndex, node.ID, i), response.Text));
			else
				texts.Insert(response.Text);
		}

		return visible;
	}

	protected bool PassesGating(DialogueResponse response)
	{
		if (!QuestGatePasses(response.RequiredQuestID))
			return false;
		if (QuestIsCompleted(response.HideAfterQuestID))
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

	//! True once the player has finished this quest. Unset (-1) is never
	//! "completed", so a response with no HideAfterQuestID is never hidden.
	protected bool QuestIsCompleted(int questID)
	{
		if (questID <= 0)
			return false;

		ExpansionQuestPersistentData questData = ExpansionQuestModule.GetModuleInstance().GetClientQuestData();
		if (!questData)
			return false;

		return questData.GetQuestStateByQuestID(questID) == ExpansionQuestState.COMPLETED;
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

	protected DialogueSpeakerLine PickSpeakerLine(DialogueNode node, out int chosenIndex)
	{
		chosenIndex = -1;

		if (!node.SpeakerLines || node.SpeakerLines.Count() == 0)
			return null;

		DialogueSpeakerLine overrideLine = null;
		int overrideBest = -1;
		int overrideIndex = -1;

		for (int c = 0; c < node.SpeakerLines.Count(); c++)
		{
			DialogueSpeakerLine candidate = node.SpeakerLines[c];
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
			overrideIndex = c;
		}

		if (overrideLine)
		{
			chosenIndex = overrideIndex;
			return overrideLine;
		}

		array<ref DialogueSpeakerLine> pool = new array<ref DialogueSpeakerLine>;
		array<int> poolIndices = new array<int>;

		for (int l = 0; l < node.SpeakerLines.Count(); l++)
		{
			DialogueSpeakerLine line = node.SpeakerLines[l];
			if (line && QuestGatePasses(line.RequiredQuestID) && VarGatePasses(line.RequiredVars))
			{
				pool.Insert(line);
				poolIndices.Insert(l);
			}
		}

		if (pool.Count() == 0)
			return null;

		int baseWeight = 0;
		if (node.SpeakerText != "")
			baseWeight = 1;

		int pick = Math.RandomInt(0, pool.Count() + baseWeight);
		if (pick >= pool.Count())
			return null;

		chosenIndex = poolIndices.Get(pick);
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
				AcceptQuest(response.QuestID);
				break;

			case DialogueActionType.OFFER_QUEST:
				OfferQuest(response.QuestID);
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
		LogScreen("Quest list screen");
		m_ShowingRewardList = false;
		m_ShowingObjItemList = false;
		m_ShowingSettings = false;

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

		RefreshResponseList();
	}

	protected void SetSpeakerLine(string text)
	{
		if (!m_SpeakerLine)
			return;

		m_SpeakerLine.SetText(DialogueFW_FormatText(text));

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
			string questLine = PickQuestLine(questText.QuestID, questText.QuestListTexts, "QuestListTexts");
			if (questLine != "")
				return questLine;
		}

		if (m_ActiveTree)
		{
			string treeLine = PickTreeLine(m_ActiveTree.QuestListTexts, "QuestListTexts");
			if (treeLine != "")
				return treeLine;
		}

		return UIText("#STR_DIALOGUEFW_QUESTLIST_PROMPT", "What do you need done?");
	}

	protected void ShowNoQuestsStep()
	{
		m_ShowingQuestList = false;
		m_ShowingRewardList = false;
		m_ShowingSettings = false;
		m_CurrentQuests.Clear();

		string spoken = "";
		array<string> backTexts = null;
		array<string> leaveTexts = null;
		array<string> voiceLines = null;

		DialogueQuestText questText = FindProgressQuestText(PROGRESS_NO_QUESTS);
		if (questText)
		{
			spoken = PickQuestLine(questText.QuestID, questText.NoQuestsTexts, "NoQuestsTexts");

			if (questText.NoQuestsBackTexts && questText.NoQuestsBackTexts.Count() > 0)
				backTexts = LocalizeQuestList(questText.QuestID, questText.NoQuestsBackTexts, "NoQuestsBackTexts");

			if (questText.NoQuestsLeaveTexts && questText.NoQuestsLeaveTexts.Count() > 0)
				leaveTexts = LocalizeQuestList(questText.QuestID, questText.NoQuestsLeaveTexts, "NoQuestsLeaveTexts");

			Print("[DialogueFramework] [DIAG] ShowNoQuestsStep() using QuestText for quest " + questText.QuestID);
		}

		if (m_ActiveTree)
		{
			if (spoken == "")
				spoken = PickTreeLine(m_ActiveTree.NoQuestsTexts, "NoQuestsTexts");

			if (!backTexts || backTexts.Count() == 0)
				backTexts = LocalizeTreeList(m_ActiveTree.NoQuestsBackTexts, "NoQuestsBackTexts");

			if (!leaveTexts || leaveTexts.Count() == 0)
				leaveTexts = LocalizeTreeList(m_ActiveTree.NoQuestsLeaveTexts, "NoQuestsLeaveTexts");

			voiceLines = m_ActiveTree.NoQuestsVoiceLineIDs;
		}

		if (spoken == "")
			spoken = UIText("#STR_DIALOGUEFW_NOQUESTS", "Nothing for you right now.");

		LogScreen("No-quests screen");
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
			m_NoQuestsNode.Responses.Insert(BuildNoQuestsResponse(UIText("#STR_DIALOGUEFW_BACK", "Back"), rootID, DialogueActionType.NONE));

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

	protected array<string> ResolveBackTexts(int questID, array<string> perQuest, array<string> treeLevel, string field)
	{
		if (perQuest && perQuest.Count() > 0)
			return LocalizeQuestList(questID, perQuest, field);

		if (treeLevel && treeLevel.Count() > 0)
			return LocalizeTreeList(treeLevel, field);

		return null;
	}

	protected static const int BACK_SCREEN_OFFER = 0;
	protected static const int BACK_SCREEN_INPROGRESS = 1;
	protected static const int BACK_SCREEN_TURNIN = 2;

	protected array<string> ResolveDetailBackTexts(DialogueQuestText questText, int screen)
	{
		array<string> perQuest = null;
		array<string> treeLevel = null;
		string field = "OfferBackTexts";

		if (screen == BACK_SCREEN_INPROGRESS)
		{
			field = "InProgressBackTexts";
			if (questText) perQuest = questText.InProgressBackTexts;
			if (m_ActiveTree) treeLevel = m_ActiveTree.InProgressBackTexts;
		}
		else if (screen == BACK_SCREEN_TURNIN)
		{
			field = "TurnInBackTexts";
			if (questText) perQuest = questText.TurnInBackTexts;
			if (m_ActiveTree) treeLevel = m_ActiveTree.TurnInBackTexts;
		}
		else
		{
			if (questText) perQuest = questText.OfferBackTexts;
			if (m_ActiveTree) treeLevel = m_ActiveTree.OfferBackTexts;
		}

		int questID = -1;
		if (questText)
			questID = questText.QuestID;

		return ResolveBackTexts(questID, perQuest, treeLevel, field);
	}

	protected array<string> ResolveQuestListBackTexts()
	{
		array<string> treeLevel = null;
		if (m_ActiveTree)
			treeLevel = m_ActiveTree.QuestListBackTexts;

		int questID = -1;
		array<string> perQuest = FindProgressBackTexts(questID);

		return ResolveBackTexts(questID, perQuest, treeLevel, "QuestListBackTexts");
	}

	protected array<string> FindProgressBackTexts(out int foundQuestID)
	{
		foundQuestID = -1;

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
			foundQuestID = candidate.QuestID;
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

	//! The gear is shown whenever the player has something worth changing.
	//! Language only counts if the server actually has translations.
	protected bool SettingsButtonWanted()
	{
		if (!m_MenuConfig)
			return false;

		return true;
	}

	protected bool LanguageRowWanted()
	{
		if (!m_MenuConfig || !m_MenuConfig.ShowLanguageButton)
			return false;

		return DialogueLoc.GetInstance().ServerLanguages().Count() > 0;
	}

	protected void ApplySettingsButtonVisibility()
	{
		if (!m_SettingsButton)
			return;

		m_SettingsButton.Show(SettingsButtonWanted());

		TextWidget label = TextWidget.Cast(m_SettingsButton.FindAnyWidget("SettingsButtonLabel"));
		if (label && m_MenuConfig)
			label.SetColor(m_MenuConfig.GetColor(m_MenuConfig.SpeakerNameColor));

		Widget background = m_SettingsButton.FindAnyWidget("SettingsButtonBackground");
		if (background && m_MenuConfig)
			background.SetColor(m_MenuConfig.GetColor(m_MenuConfig.ResponseBackgroundColor));
	}

	protected void ShowSettingsScreen()
	{
		StopDialogueVoice();

		m_ShowingQuestList = false;
		m_ShowingRewardList = false;
		m_ShowingObjItemList = false;
		m_ShowingSettings = true;
		LogScreen("Player settings screen");
		HideRewardDisplay();

		if (m_SpeakerName)
			m_SpeakerName.SetText(UIText("#STR_DIALOGUEFW_SETTINGS_TITLE", "Settings"));
		if (m_SpeakerLine)
			SetSpeakerLine(UIText("#STR_DIALOGUEFW_SETTINGS_HINT", "These are yours alone. They follow you to any server running this mod."));

		ClearButtons();

		m_SettingRows.Clear();

		//! Each label and value goes into its own local first. Passing two
		//! freshly-returned strings straight into one call aliases them, and
		//! every row came out reading "server's choice: server's choice".
		string rowLabel;
		string rowValue;

		if (LanguageRowWanted())
		{
			rowValue = SettingValueLanguage();
			rowLabel = UIText("#STR_DIALOGUEFW_LANG_BUTTON", "Language");
			AddSettingRow(SETTING_LANGUAGE, rowLabel, rowValue);
		}

		rowValue = SettingValuePosition();
		rowLabel = UIText("#STR_DIALOGUEFW_SET_POSITION", "Window position");
		AddSettingRow(SETTING_POSITION, rowLabel, rowValue);

		rowValue = SettingValueTextSize();
		rowLabel = UIText("#STR_DIALOGUEFW_SET_TEXTSIZE", "Text size");
		AddSettingRow(SETTING_TEXTSIZE, rowLabel, rowValue);

		rowValue = SettingValueIcons();
		rowLabel = UIText("#STR_DIALOGUEFW_SET_ICONS", "Button icons");
		AddSettingRow(SETTING_ICONS, rowLabel, rowValue);

		if (DialogueClientSettings.Get().HasAnyOverride())
		{
			rowLabel = UIText("#STR_DIALOGUEFW_SET_RESET", "Reset to the server's settings");
			AddSettingRow(SETTING_RESET, rowLabel, "");
		}

		m_ResponseButtons.Insert(CreateResponseButton(UIText("#STR_DIALOGUEFW_SET_BACK", "Back to the conversation"), false, ICON_CHAT));

		RefreshResponseList();
	}

	protected void AddSettingRow(string key, string label, string value)
	{
		m_SettingRows.Insert(key);

		string text = label;
		if (value != "")
			text = label + ":   " + value;

		m_ResponseButtons.Insert(CreateResponseButton(text, false, ICON_CHAT));
	}

	protected string ServerDefaultLabel()
	{
		return UIText("#STR_DIALOGUEFW_SET_SERVER", "server's choice");
	}

	protected string SettingValueLanguage()
	{
		DialogueLoc loc = DialogueLoc.GetInstance();
		string preferred = loc.PreferredLanguage();

		if (preferred == "")
		{
			string autoWord = UIText("#STR_DIALOGUEFW_LANG_AUTO", "Automatic");
			string detected = DialogueFWLanguages.DisplayName(loc.DetectedLanguage());
			return autoWord + " (" + detected + ")";
		}

		return DialogueFWLanguages.DisplayName(preferred);
	}

	protected string SettingValuePosition()
	{
		string position = DialogueClientSettings.Get().Position;
		if (position == "")
			return ServerDefaultLabel();

		return DialogueMenuPosition.Label(position);
	}

	protected string SettingValueTextSize()
	{
		float scale = DialogueClientSettings.Get().TextScale;
		if (scale == DialogueClientSettings.TEXT_SCALE_SERVER)
			return ServerDefaultLabel();

		int percent = (int)(scale * 100);
		return "" + percent + "%";
	}

	protected string SettingValueIcons()
	{
		int icons = DialogueClientSettings.Get().Icons;
		if (icons == DialogueClientSettings.ICONS_ON)
			return UIText("#STR_DIALOGUEFW_SET_ON", "shown");
		if (icons == DialogueClientSettings.ICONS_OFF)
			return UIText("#STR_DIALOGUEFW_SET_OFF", "hidden");

		return ServerDefaultLabel();
	}

	//! Every row cycles on click, so the whole screen needs no widget beyond
	//! the response buttons the window already knows how to draw.
	protected void CycleSetting(string key)
	{
		DialogueClientSettings settings = DialogueClientSettings.Get();

		if (key == SETTING_LANGUAGE)
		{
			CycleLanguage();
			return;
		}

		if (key == SETTING_POSITION)
		{
			array<string> positions = DialogueMenuPosition.All();
			int posIndex = positions.Find(settings.Position);
			posIndex = posIndex + 1;

			if (settings.Position == "")
				posIndex = 0;

			if (posIndex >= positions.Count())
				settings.Position = "";
			else
				settings.Position = positions[posIndex];

			settings.Save();
			ApplyMenuConfig();
		}
		else if (key == SETTING_TEXTSIZE)
		{
			settings.TextScale = NextTextScale(settings.TextScale);
			settings.Save();
		}
		else if (key == SETTING_ICONS)
		{
			if (settings.Icons == DialogueClientSettings.ICONS_SERVER)
				settings.Icons = DialogueClientSettings.ICONS_ON;
			else if (settings.Icons == DialogueClientSettings.ICONS_ON)
				settings.Icons = DialogueClientSettings.ICONS_OFF;
			else
				settings.Icons = DialogueClientSettings.ICONS_SERVER;

			settings.Save();
		}
		else if (key == SETTING_RESET)
		{
			settings.ResetAll();
			DialogueLoc.GetInstance().SetPreferredLanguage("");
			DialogueFrameworkSyncModule.DialogueFW_RequestLanguage(DialogueLoc.GetInstance().EffectiveLanguage());
			ApplyMenuConfig();
		}

		ShowSettingsScreen();
	}

	protected float NextTextScale(float current)
	{
		if (current == DialogueClientSettings.TEXT_SCALE_SERVER)
			return 0.8;
		if (current < 0.85)
			return 0.9;
		if (current < 0.95)
			return 1.1;
		if (current < 1.15)
			return 1.25;
		if (current < 1.3)
			return 1.5;

		return DialogueClientSettings.TEXT_SCALE_SERVER;
	}

	protected void CycleLanguage()
	{
		DialogueLoc loc = DialogueLoc.GetInstance();
		array<string> offered = loc.ServerLanguages();

		string current = loc.PreferredLanguage();
		string next = "";

		if (current == "")
		{
			if (offered.Count() > 0)
				next = offered[0];
		}
		else
		{
			int index = offered.Find(current);
			index = index + 1;
			if (index < offered.Count())
				next = offered[index];
		}

		loc.SetPreferredLanguage(next);
		DialogueClientSettings.Get().Language = next;
		DialogueClientSettings.Get().Save();

		DialogueFrameworkSyncModule.DialogueFW_RequestLanguage(loc.EffectiveLanguage());

		//! The bundle arrives over the network, so give it a moment before the
		//! screen redraws with the new wording.
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(ShowSettingsScreen);
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(ShowSettingsScreen, 350, false);
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
			LogScreen("Quest turn-in screen");

			AddQuestResponses(detail, questText, "TurnInTexts", questText.TurnInTexts, "#STR_DIALOGUEFW_TURNIN", "Here you go.", DialogueActionType.TURN_IN_QUEST);
			AddQuestResponses(detail, questText, "NotYetTexts", questText.NotYetTexts, "#STR_DIALOGUEFW_NOTYET", "Not yet.", DialogueActionType.END_CONVERSATION);
			AddBackToConversationResponses(detail, ResolveDetailBackTexts(questText, BACK_SCREEN_TURNIN));
		}
		else if (state == ExpansionQuestState.STARTED)
		{
			detail.SpeakerText = GetDescriptionSafe(descriptions, 1);
			detail.VoiceLineIDs.Insert("Quest_" + quest.GetID() + "_InProgress");
			LogScreen("Quest in-progress screen");

			AddQuestResponses(detail, questText, "InProgressTexts", questText.InProgressTexts, "#STR_DIALOGUEFW_INPROGRESS", "Still working on it.", DialogueActionType.END_CONVERSATION);
			AddBackToConversationResponses(detail, ResolveDetailBackTexts(questText, BACK_SCREEN_INPROGRESS));
		}
		else
		{
			int remaining;
			if (IsQuestOnCooldown(quest, remaining))
			{
				LogScreen("Cooldown screen");
				string cooldownNote = string.Format(UIText("#STR_DIALOGUEFW_COOLDOWN", "Not again yet. Come back in %1."), ExpansionStatic.GetTimeString(remaining));
				detail.SpeakerText = GetDescriptionSafe(descriptions, 0) + "\n\n" + cooldownNote;
				AddQuestResponses(detail, questText, "NotYetTexts", questText.NotYetTexts, "#STR_DIALOGUEFW_ANOTHERTIME", "Another time, then.", DialogueActionType.END_CONVERSATION);
				AddBackToConversationResponses(detail, ResolveDetailBackTexts(questText, BACK_SCREEN_OFFER));

				Print("[DialogueFramework] [DIAG] Quest " + quest.GetID() + " is on cooldown, " + remaining + "s remaining -- accept suppressed.");

				RenderNode(detail);
				ShowItemDisplay(quest);
				return;
			}

			detail.SpeakerText = GetDescriptionSafe(descriptions, 0);
			detail.VoiceLineIDs.Insert("Quest_" + quest.GetID() + "_Start");
			LogScreen("Quest offer screen");

			//! No accept button unless they could actually take it -- better
			//! than a button that refuses when pressed.
			if (CanPlayerTakeQuest(quest))
				AddQuestResponses(detail, questText, "AcceptTexts", questText.AcceptTexts, "#STR_DIALOGUEFW_ACCEPT", "I'll take it.", DialogueActionType.ACCEPT_QUEST);
			else
				Print("[DialogueFramework] [QUEST] Offer screen for quest " + quest.GetID() + " has no accept button -- this player cannot take it right now.");
			AddQuestResponses(detail, questText, "DeclineTexts", questText.DeclineTexts, "#STR_DIALOGUEFW_DECLINE", "Not interested.", DialogueActionType.DECLINE_QUEST);
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
		ApplyRewardHeading();
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

		string text = UIText("#STR_DIALOGUEFW_HEAD_TURNIN", "Turn in:");
		if (m_StripHasGiven && !m_StripHasNeeded)
			text = UIText("#STR_DIALOGUEFW_HEAD_GIVEN", "Given to you:");
		if (m_StripHasGiven && m_StripHasNeeded)
			text = UIText("#STR_DIALOGUEFW_HEAD_TURNIN_GIVEN", "Turn in & given:");

		heading.SetText(text);
	}

	protected void ApplyRewardHeading()
	{
		if (!m_RewardStripLabel)
			return;

		TextWidget heading = TextWidget.Cast(m_RewardStripLabel.FindAnyWidget("RewardStripLabelText"));
		if (!heading)
			return;

		heading.SetText(UIText("#STR_DIALOGUEFW_HEAD_REWARD", "Reward:"));
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
			label.SetSize(boxW, labelH * panelH);
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

	protected void AddQuestResponses(DialogueNode node, DialogueQuestText questText, string field, array<string> texts, string defaultTextKey, string defaultFallback, string actionType)
	{
		if (texts && texts.Count() > 0)
		{
			int questID = -1;
			if (questText)
				questID = questText.QuestID;

			for (int i = 0; i < texts.Count(); i++)
			{
				DialogueResponse poolResponse = new DialogueResponse();
				poolResponse.Text = DialogueLoc.ForQuest(questID, DialogueLocKeys.QuestList(field, i), texts[i]);
				poolResponse.ActionType = actionType;
				node.Responses.Insert(poolResponse);
			}
		}
		else
		{
			DialogueResponse defaultResponse = new DialogueResponse();
			defaultResponse.Text = UIText(defaultTextKey, defaultFallback);
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

	protected string DialogueFW_FormatText(string text)
	{
		if (text == "")
			return text;

		string playerName = "";
		Man man = GetGame().GetPlayer();
		if (man && man.GetIdentity())
			playerName = man.GetIdentity().GetName();

		StringLocaliser loc = new StringLocaliser(text, playerName);
		return loc.Format();
	}

	//! questID of -1 means "whatever the live quest-detail step is showing",
	//! which is how the mod's own accept button has always worked. An authored
	//! response names its own quest instead.
	//! Whether this player could legitimately take this quest right now.
	//! Passing -1 as the NPC id skips Expansion's "is this NPC the giver"
	//! test, which is the point -- the option can live on any character --
	//! while keeping every other rule: already completed, on cooldown,
	//! prerequisites unmet, achievement quests.
	protected bool CanPlayerTakeQuest(ExpansionQuestConfig quest)
	{
		if (!quest)
			return false;

		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		if (!player)
			return false;

		ExpansionQuestPersistentData questData = ExpansionQuestModule.GetModuleInstance().GetClientQuestData();

		ExpansionQuestState state = ExpansionQuestState.NONE;
		if (questData)
			state = questData.GetQuestStateByQuestID(quest.GetID());

		//! Already on it, or ready to hand it in. Handing it over again would
		//! either do nothing or reset their progress.
		if (state == ExpansionQuestState.STARTED || state == ExpansionQuestState.CAN_TURNIN)
			return false;

		return ExpansionQuestModule.GetModuleInstance().QuestDisplayConditions(quest, player, questData, -1, false);
	}

	protected void AcceptQuest(int questID)
	{
		int target = questID;
		if (target <= 0)
			target = m_ActiveQuestID;

		if (target <= 0)
		{
			Print("[DialogueFramework] [QUEST] [ERROR] ACCEPT_QUEST with no quest to accept. Set the response's QuestID, or only use this action inside the live quest-detail step.");
			NotifyPlayer("#STR_DIALOGUEFW_NOTIFY_BADOPTION", "That option isn't set up correctly. The server owner can find the reason in the log.", true);
			EndConversation();
			return;
		}

		ExpansionQuestConfig targetConfig = ExpansionQuestModule.GetModuleInstance().GetQuestConfigByID(target);
		if (!CanPlayerTakeQuest(targetConfig))
		{
			Print("[DialogueFramework] [QUEST] Refused to start quest " + target + " -- this player cannot take it right now (already on it, already finished it, on cooldown, or its prerequisites are not met). Nothing was changed.");
			NotifyPlayer("#STR_DIALOGUEFW_NOTIFY_CANTTAKE", "You can't take that quest right now.", false);
			EndConversation();
			return;
		}

		Print("[DialogueFramework] [QUEST] Accepting quest " + target);
		ExpansionQuestModule.GetModuleInstance().RequestCreateQuestInstance(target);

		m_ActiveQuestID = -1;
		EndConversation();
	}

	//! Shows the quest's own offer screen -- description, item tiles, the
	//! accept and decline buttons -- exactly as the live quest list does.
	protected void OfferQuest(int questID)
	{
		if (questID <= 0)
		{
			Print("[DialogueFramework] [QUEST] [ERROR] OFFER_QUEST with no QuestID set on the response -- nothing to show.");
			NotifyPlayer("#STR_DIALOGUEFW_NOTIFY_BADOPTION", "That option isn't set up correctly. The server owner can find the reason in the log.", true);
			EndConversation();
			return;
		}

		ExpansionQuestConfig quest = ExpansionQuestModule.GetModuleInstance().GetQuestConfigByID(questID);
		if (!quest)
		{
			Print("[DialogueFramework] [QUEST] [ERROR] OFFER_QUEST points at quest " + questID + ", which no quest config matches. Check the ID against your Expansion quest files.");
			NotifyPlayer("#STR_DIALOGUEFW_NOTIFY_BADOPTION", "That option isn't set up correctly. The server owner can find the reason in the log.", true);
			EndConversation();
			return;
		}

		Print("[DialogueFramework] [QUEST] Offering quest " + questID + " from a response.");
		ShowQuestDetail(quest);
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
		LogScreen("Item choice screen");
		m_ShowingSettings = false;
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

		RefreshResponseList();
	}

	protected string PickObjItemPrompt()
	{
		return UIText("#STR_DIALOGUEFW_OBJITEM_PROMPT", "Which will you hand over?");
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
		LogScreen("Reward choice screen");
		m_ShowingObjItemList = false;
		m_ShowingSettings = false;
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

		RefreshResponseList();
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
		{
			string confirmName = GetItemDisplayName(reward.GetClassName());
			string confirmAmount = reward.GetAmount().ToString();
			string confirmFormat = UIText("#STR_DIALOGUEFW_CONFIRM_REWARD", "Take %1 x%2 and hand the job in?");
			m_ConfirmText.SetText(string.Format(confirmFormat, confirmName, confirmAmount));
		}

		LogScreen("Reward confirm box");

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
				return DialogueLoc.ForQuest(m_ActiveQuestID, DialogueLocKeys.QuestSingle("RewardSelectText"), questText.RewardSelectText);
		}

		return UIText("#STR_DIALOGUEFW_REWARD_PROMPT", "Take your pick.");
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

	protected bool ApplyResponseIcon(Widget button, string iconName)
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
			return false;

		bool wanted = false;
		if (m_MenuConfig && m_MenuConfig.ShowResponseIcons)
			wanted = true;

		int playerIcons = DialogueClientSettings.Get().Icons;
		if (playerIcons == DialogueClientSettings.ICONS_ON)
			wanted = true;
		else if (playerIcons == DialogueClientSettings.ICONS_OFF)
			wanted = false;

		if (!wanted || iconName == "")
		{
			icon.Show(false);
			return false;
		}

		icon.LoadImageFile(0, ICON_FOLDER + iconName + ICON_EXT);
		icon.SetImage(0);
		if (m_MenuConfig)
			icon.SetColor(m_MenuConfig.GetColor(m_MenuConfig.ResponseTextColor));
		icon.Show(true);

		TextWidget label = TextWidget.Cast(button.FindAnyWidget("DialogueResponseButtonText"));
		if (label)
			label.SetSize(0.85, 1.0);

		return true;
	}

	protected float ResponseBaseFontSize()
	{
		float size = RESPONSE_FONT_PX;

		if (m_MenuConfig)
		{
			if (m_MenuConfig.FontStyle == "LARGE")
				size = RESPONSE_FONT_PX_LARGE;
			else if (m_MenuConfig.FontStyle == "COMPACT")
				size = RESPONSE_FONT_PX_COMPACT;

			if (m_MenuConfig.ScaleTextWithPanel)
				size = size * PanelTextScale();
		}

		float playerScale = DialogueClientSettings.Get().TextScale;
		if (playerScale != DialogueClientSettings.TEXT_SCALE_SERVER)
			size = size * playerScale;

		return size;
	}

	protected float PanelTextScale()
	{
		if (!m_MenuConfig || m_MenuConfig.PanelWidth <= 0)
			return 1.0;

		float scale = m_MenuConfig.PanelWidth / PANEL_REFERENCE_WIDTH;

		if (scale < PANEL_SCALE_MIN)
			scale = PANEL_SCALE_MIN;
		if (scale > PANEL_SCALE_MAX)
			scale = PANEL_SCALE_MAX;

		return scale;
	}

	protected int ResponseLinesNeeded(string text, float widthPx, float fontSize)
	{
		if (widthPx <= 0 || fontSize <= 0)
			return 1;

		float charWidth = fontSize * RESPONSE_CHAR_RATIO;

		float perLine = (widthPx / charWidth) * RESPONSE_WRAP_SAFETY;
		if (perLine < 4)
			perLine = 4;

		int length = text.Length();
		int lines = 1;
		float consumed = perLine;

		while (consumed < length)
		{
			consumed = consumed + perLine;
			lines = lines + 1;
		}

		return lines;
	}

	protected float SizeResponseButton(Widget button, TextWidget label, string text, bool hasIcon)
	{
		if (!button || !label || !m_DialoguePanel)
			return 0;

		float panelW;
		float panelH;
		m_DialoguePanel.GetScreenSize(panelW, panelH);

		if (panelW <= 0)
		{
			Print("[DialogueFramework] [UI] Response button: panel not measured yet, keeping the layout default size.");
			return 0;
		}

		float textFraction = 0.96;
		if (hasIcon)
			textFraction = 0.85;

		float textW = panelW * m_ScrollW * 0.98 * textFraction;

		float fontSize = ResponseBaseFontSize();
		int lines = ResponseLinesNeeded(text, textW, fontSize);

		//! Growing is fine up to a point -- past that, shrink rather than let
		//! one option swallow the list. Never cap the line count afterwards:
		//! a button that grows too tall is a cosmetic problem, one that cuts
		//! a sentence in half is the bug being fixed here.
		while (lines > RESPONSE_GROW_LINES && fontSize > RESPONSE_MIN_FONT_PX)
		{
			fontSize = fontSize - 1;
			lines = ResponseLinesNeeded(text, textW, fontSize);
		}

		float neededH = lines * fontSize * RESPONSE_LINE_SPACING + RESPONSE_VPAD_PX;
		if (neededH < RESPONSE_MIN_HEIGHT_PX)
			neededH = RESPONSE_MIN_HEIGHT_PX;

		label.SetTextExactSize(fontSize);
		button.SetSize(0.98, neededH);
		label.Update();

		//! Read the height straight back. If the spacer or the layout has
		//! overridden it, this is the line that says so -- guessing at why
		//! buttons still overlap has already cost two rebuilds.
		//! Log the first button that actually wraps -- a one-line button tells
		//! us nothing, which is what the first version of this logged.
		if (!m_SizingDiagLogged && lines > 1)
		{
			m_SizingDiagLogged = true;

			float checkW;
			float checkH;
			button.GetSize(checkW, checkH);

			string diag = "[DialogueFramework] [UI] Response sizing: panel=" + panelW + "px scrollW=" + m_ScrollW;
			diag = diag + " textCol=" + textW + "px font=" + fontSize + " lines=" + lines;
			diag = diag + " askedH=" + neededH + " gotBackH=" + checkH;
			Print(diag);
		}

		return neededH;
	}

	protected bool m_SizingDiagLogged;

	protected Widget CreateResponseButton(string text, bool visited = false, string iconName = "")
	{
		if (!m_ResponseList)
			return null;

		Widget button = GetGame().GetWorkspace().CreateWidgets(LayoutPath("dialogue_response_button"), m_ResponseList);
		if (!button)
			return null;

		string display = DialogueFW_FormatText(text);

		TextWidget label = TextWidget.Cast(button.FindAnyWidget("DialogueResponseButtonText"));
		if (!label && !m_LabelCastWarned)
		{
			m_LabelCastWarned = true;
			Print("[DialogueFramework] [UI] [ERROR] DialogueResponseButtonText did not cast to TextWidget -- response buttons will be blank. The .layout in the built PBO is out of step with the scripts; repack the DialogueFramework PBO.");
		}

		if (label)
		{
			label.SetText(display);
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

		bool hasIcon = ApplyResponseIcon(button, iconName);
		ApplyBorderColor(button);

		m_ContentHeightPx = m_ContentHeightPx + SizeResponseButton(button, label, display, hasIcon);

		m_SizedButtons.Insert(button);
		m_SizedTexts.Insert(display);
		m_SizedIcons.Insert(hasIcon);

		button.SetUserID(m_ResponseButtons.Count());

		return button;
	}

	protected void ResetIconDiagnostics()
	{
		m_IconDiagLogged = false;
		m_SizingDiagLogged = false;
	}

	protected void ClearButtons()
	{
		foreach (Widget w : m_ResponseButtons)
		{
			if (w)
				w.Unlink();
		}
		m_ResponseButtons.Clear();
		m_ScrollPos01 = 0;
		m_ContentHeightPx = 0;
		m_SizedButtons.Clear();
		m_SizedTexts.Clear();
		m_SizedIcons.Clear();

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

		if (w == m_SettingsButton)
		{
			Print("[DialogueFramework] [DIAG] OnClick -> Settings button.");
			if (m_ShowingSettings)
				ReturnToRootDeferred();
			else
				ShowSettingsScreen();
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
		else if (m_ShowingSettings)
		{
			if (idx >= m_SettingRows.Count())
			{
				Print("[DialogueFramework] [DIAG] OnClick -> settings closed.");
				m_ShowingSettings = false;
				ReturnToRootDeferred();
				return true;
			}

			Print("[DialogueFramework] [DIAG] OnClick -> setting cycled: " + m_SettingRows[idx]);
			CycleSetting(m_SettingRows[idx]);
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

	override bool OnMouseWheel(Widget w, int x, int y, int wheel)
	{
		if (!OverResponseList(w))
			return false;

		if (!m_ResponseScroll || m_ContentHeightPx <= 0)
			return false;

		float viewW;
		float viewH;
		m_ResponseScroll.GetScreenSize(viewW, viewH);

		float scrollable = m_ContentHeightPx - viewH;
		if (scrollable <= 0)
			return false;

		float step = RESPONSE_SCROLL_STEP_PX / scrollable;

		m_ScrollPos01 = m_ScrollPos01 - wheel * step;
		if (m_ScrollPos01 < 0)
			m_ScrollPos01 = 0;
		if (m_ScrollPos01 > 1)
			m_ScrollPos01 = 1;

		m_ResponseScroll.VScrollToPos01(m_ScrollPos01);
		return true;
	}

	//! Only take the wheel over the option list -- the spoken line has its own
	//! scroll and the engine handles that one fine.
	protected bool OverResponseList(Widget w)
	{
		if (!w)
			return false;

		if (w == m_ResponseScroll || w == m_ResponseList)
			return true;

		foreach (Widget button : m_ResponseButtons)
		{
			if (!button)
				continue;
			if (w == button)
				return true;
			if (button.FindAnyWidget(w.GetName()) == w)
				return true;
		}

		return false;
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

		GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(ReapplyResponseSizes);
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(ExecuteReturnToRoot);
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(ShowSettingsScreen);

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
