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

	protected ref array<ExpansionQuestRewardConfig> m_CurrentRewards;
	protected bool m_ShowingRewardList;

	protected ref map<string, bool> m_VisitedResponses;

	protected DialogueMenuConfig m_MenuConfig;
	protected Widget m_DialoguePanel;
	protected ScrollWidget m_ResponseScroll;
	protected Widget m_ConfirmPanel;
	protected RichTextWidget m_ConfirmText;
	protected Widget m_ConfirmYesButton;
	protected Widget m_ConfirmNoButton;

	protected ref array<EntityAI> m_RewardPreviewObjects;

	protected ref array<Widget> m_RewardDisplayWidgets;
	protected Widget m_RewardStrip;
	protected Widget m_RewardStripLabel;

	protected float m_ScrollX;
	protected float m_ScrollY;
	protected float m_ScrollW;
	protected float m_ScrollH;
	protected int m_SelectedRewardIndex = -1;

	protected bool m_OpeningTrader;

	protected ExpansionNPCBase m_TalkingNPC;
#ifdef EXPANSIONMODAI
	protected eAIBase m_TalkingNPCAI;
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
	}

	override Widget Init()
	{
		m_MenuConfig = DialogueManager.GetInstance().GetMenuConfig();

		string layoutPath = "DialogueFramework/GUI/layouts/dialogue_menu.layout";
		if (m_MenuConfig && m_MenuConfig.LayoutOverride != "")
		{
			Print("[DialogueFramework] [UI] Using layout override: " + m_MenuConfig.LayoutOverride);
			layoutPath = m_MenuConfig.LayoutOverride;
		}

		layoutRoot = GetGame().GetWorkspace().CreateWidgets(layoutPath);

		if (!layoutRoot && layoutPath != "DialogueFramework/GUI/layouts/dialogue_menu.layout")
		{
			Print("[DialogueFramework] [UI] [ERROR] Layout override failed to load -- falling back to the built-in layout.");
			layoutRoot = GetGame().GetWorkspace().CreateWidgets("DialogueFramework/GUI/layouts/dialogue_menu.layout");
		}

		m_SpeakerName = TextWidget.Cast(layoutRoot.FindAnyWidget("SpeakerName"));
		m_SpeakerLine = RichTextWidget.Cast(layoutRoot.FindAnyWidget("SpeakerLine"));
		m_ResponseList = WrapSpacerWidget.Cast(layoutRoot.FindAnyWidget("ResponseList"));
		m_CloseButton = layoutRoot.FindAnyWidget("CloseButton");
		m_ResponseScroll = ScrollWidget.Cast(layoutRoot.FindAnyWidget("ResponseScroll"));
		m_ConfirmPanel = layoutRoot.FindAnyWidget("ConfirmPanel");
		m_ConfirmText = RichTextWidget.Cast(layoutRoot.FindAnyWidget("ConfirmText"));
		m_RewardStrip = layoutRoot.FindAnyWidget("RewardStrip");
		m_RewardStripLabel = layoutRoot.FindAnyWidget("RewardStripLabel");

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

		DialogueNode root = FindNode(m_ActiveTree, m_ActiveTree.RootNodeID);
		RenderNode(root);

		Print("[DialogueFramework] [DIAG] OpenRootNode() completed -- buttons created=" + m_ResponseButtons.Count());

		array<string> greetingPool = m_ActiveTree.GreetingVoiceLineIDs;
		if (greetingPool)
			PlayRandomVoiceLine(greetingPool);

		Print("[DialogueFramework] [DIAG] OpenRootNode() greeting voice attempted, done.");
	}

	protected DialogueNode FindNode(DialogueTree tree, int nodeID)
	{
		if (!tree || !tree.Nodes)
			return null;

		foreach (DialogueNode node : tree.Nodes)
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
		m_SelectedRewardIndex = -1;
		HideRewardConfirm();
		HideRewardDisplay();

		array<ref DialogueResponse> visible = GetVisibleResponses(node);

		if (m_SpeakerName)
			m_SpeakerName.SetText(m_NPCName);
		if (m_SpeakerLine)
			m_SpeakerLine.SetText(node.SpeakerText);

		ClearButtons();

		m_CurrentResponses = visible;
		for (int i = 0; i < visible.Count(); i++)
		{
			bool wasVisited = m_VisitedResponses.Contains(node.ID.ToString() + ":" + i);
			m_ResponseButtons.Insert(CreateResponseButton(visible[i].Text, wasVisited));
		}

		StopDialogueVoice();

		array<string> nodeVoicePool = node.VoiceLineIDs;
		if (nodeVoicePool)
			PlayRandomVoiceLine(nodeVoicePool);
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
		if (response.RequiredQuestID <= 0)
			return true;

		ExpansionQuestPersistentData questData = ExpansionQuestModule.GetModuleInstance().GetClientQuestData();
		if (!questData)
			return false;

		return questData.GetQuestStateByQuestID(response.RequiredQuestID) == ExpansionQuestState.COMPLETED;
	}

	protected void OnDialogueResponseSelected(DialogueResponse response)
	{
		if (!response)
			return;

		m_PendingResponse = response;
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(ExecutePendingResponse, 0, false);
	}

	protected ref DialogueResponse m_PendingResponse;

	protected void ExecutePendingResponse()
	{
		DialogueResponse response = m_PendingResponse;
		m_PendingResponse = null;
		if (!response)
			return;

		Print("[DialogueFramework] [DIAG] ExecutePendingResponse() actionType=" + response.ActionType);

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

			case DialogueActionType.NONE:
			default:
				if (response.NextNodeID == -1)
				{
					EndConversation();
					return;
				}
				RenderNode(FindNode(m_ActiveTree, response.NextNodeID));
				break;
		}
	}

	protected void ShowLiveQuestList()
	{
		Print("[DialogueFramework] [DIAG] ShowLiveQuestList() entered.");
		StopDialogueVoice();
		m_ShowingQuestList = true;
		m_ShowingRewardList = false;

		array<ExpansionQuestConfig> validQuests = GetAvailableQuestsForNPC();
		Print("[DialogueFramework] [DIAG] ShowLiveQuestList() got " + validQuests.Count() + " valid quests.");

		if (m_SpeakerName)
			m_SpeakerName.SetText(m_NPCName);
		if (m_SpeakerLine)
			m_SpeakerLine.SetText("What do you need done?");

		ClearButtons();

		m_CurrentQuests = validQuests;
		if (validQuests.Count() == 0)
		{
			m_ResponseButtons.Insert(CreateResponseButton("Nothing for you right now."));
		}
		else
		{
			foreach (ExpansionQuestConfig quest : validQuests)
			{
				int remaining;
				string title = quest.GetTitle();
				if (IsQuestOnCooldown(quest, remaining))
					title = title + "  (" + ExpansionStatic.GetTimeString(remaining) + ")";

				m_ResponseButtons.Insert(CreateResponseButton(title, IsQuestOnCooldown(quest, remaining)));
			}
		}
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
		}
		else if (state == ExpansionQuestState.STARTED)
		{
			detail.SpeakerText = GetDescriptionSafe(descriptions, 1);
			detail.VoiceLineIDs.Insert("Quest_" + quest.GetID() + "_InProgress");

			AddQuestResponses(detail, questText.InProgressTexts, "Still working on it.", DialogueActionType.END_CONVERSATION);
		}
		else
		{
			int remaining;
			if (IsQuestOnCooldown(quest, remaining))
			{
				detail.SpeakerText = GetDescriptionSafe(descriptions, 0) + "\n\nNot again yet. Come back in " + ExpansionStatic.GetTimeString(remaining) + ".";
				AddQuestResponses(detail, questText.NotYetTexts, "Another time, then.", DialogueActionType.END_CONVERSATION);

				Print("[DialogueFramework] [DIAG] Quest " + quest.GetID() + " is on cooldown, " + remaining + "s remaining -- accept suppressed.");

				RenderNode(detail);
				ShowItemDisplay(quest);
				return;
			}

			detail.SpeakerText = GetDescriptionSafe(descriptions, 0);
			detail.VoiceLineIDs.Insert("Quest_" + quest.GetID() + "_Start");

			AddQuestResponses(detail, questText.AcceptTexts, "I'll take it.", DialogueActionType.ACCEPT_QUEST);
			AddQuestResponses(detail, questText.DeclineTexts, "Not interested.", DialogueActionType.DECLINE_QUEST);
		}

		RenderNode(detail);

		ShowItemDisplay(quest);
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
			return;

		m_RewardStrip.Show(true);
		if (m_RewardStripLabel)
			m_RewardStripLabel.Show(true);

		Print("[DialogueFramework] [DIAG] ShowItemDisplay -- " + m_RewardDisplayWidgets.Count() + " tile(s).");
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

		Widget tile = GetGame().GetWorkspace().CreateWidgets("DialogueFramework/GUI/layouts/dialogue_reward_display.layout", m_RewardStrip);
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
			tileAmount.SetText("x" + amount + " " + suffix);

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

		ExpansionQuestConfig quest = ExpansionQuestModule.GetModuleInstance().GetQuestConfigByID(m_ActiveQuestID);
		if (quest && quest.NeedToSelectReward() && quest.GetRewards() && quest.GetRewards().Count() > 1)
		{
			Print("[DialogueFramework] [DIAG] TurnInActiveQuest() -- quest " + m_ActiveQuestID + " needs reward selection, showing picker.");
			ShowRewardSelection(quest);
			return;
		}

		ExpansionQuestModule.GetModuleInstance().RequestTurnInQuestClient(m_ActiveQuestID, false, null, -1);

		m_ActiveQuestID = -1;
		EndConversation();
	}

	protected void ShowRewardSelection(ExpansionQuestConfig quest)
	{
		StopDialogueVoice();

		m_ShowingQuestList = false;
		m_ShowingRewardList = true;
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
			m_SpeakerName.SetText(m_NPCName);
		if (m_SpeakerLine)
			m_SpeakerLine.SetText(PickRewardPrompt());

		ClearButtons();
		m_SelectedRewardIndex = -1;

		for (int r = 0; r < m_CurrentRewards.Count(); r++)
			m_ResponseButtons.Insert(CreateRewardButton(m_CurrentRewards[r]));
	}

	protected Widget CreateRewardButton(ExpansionQuestRewardConfig reward)
	{
		if (!m_ResponseList || !reward)
			return null;

		Widget button = GetGame().GetWorkspace().CreateWidgets("DialogueFramework/GUI/layouts/dialogue_reward_button.layout", m_ResponseList);
		if (!button)
			return null;

		string className = reward.GetClassName();

		TextWidget nameLabel = TextWidget.Cast(button.FindAnyWidget("DialogueRewardName"));
		if (nameLabel)
			nameLabel.SetText(GetItemDisplayName(className));

		TextWidget amountLabel = TextWidget.Cast(button.FindAnyWidget("DialogueRewardAmount"));
		if (amountLabel)
			amountLabel.SetText("x" + reward.GetAmount());

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
			displayName = className;

		return displayName;
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

		Print("[DialogueFramework] [DIAG] ExecutePendingRewardSelection() -- turning in quest " + m_ActiveQuestID + " with reward " + reward.GetClassName());

		if (m_ActiveQuestID != -1)
			ExpansionQuestModule.GetModuleInstance().RequestTurnInQuestClient(m_ActiveQuestID, true, reward, -1);

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

	protected Widget CreateResponseButton(string text, bool visited = false)
	{
		if (!m_ResponseList)
			return null;

		Widget button = GetGame().GetWorkspace().CreateWidgets("DialogueFramework/GUI/layouts/dialogue_response_button.layout", m_ResponseList);
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

		ApplyBorderColor(button);

		button.SetUserID(m_ResponseButtons.Count());

		return button;
	}

	protected void ClearButtons()
	{
		foreach (Widget w : m_ResponseButtons)
		{
			if (w)
				w.Unlink();
		}
		m_ResponseButtons.Clear();

		DestroyRewardPreviews();

		foreach (Widget rewardTile : m_RewardDisplayWidgets)
		{
			if (rewardTile)
				rewardTile.Unlink();
		}
		m_RewardDisplayWidgets.Clear();

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

		if (m_ShowingRewardList)
		{
			if (idx >= m_CurrentRewards.Count())
				return true;

			Print("[DialogueFramework] [DIAG] OnClick -> reward highlighted idx=" + idx);
			SelectReward(idx);
		}
		else if (m_ShowingQuestList)
		{
			if (idx >= m_CurrentQuests.Count())
				return true;

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
