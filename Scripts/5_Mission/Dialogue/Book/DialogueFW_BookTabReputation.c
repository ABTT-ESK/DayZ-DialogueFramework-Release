#ifdef EXPANSIONMODBOOK
//! A page in Expansion's book listing where the player stands with everyone.
//!
//! The icon this tab asks for is registered into Expansion's set over in
//! 3_Game -- see DialogueBookIcon.c for why it cannot be done from here.
//!
//! Everything it needs is already on the client: the trees arrive at login
//! and carry each character's reputation variable and ranks, and the values
//! themselves are the same ones the conversation window reads. So the page is
//! built from what is already there and needs nothing from the server.
class DialogueFW_BookRepRow : ExpansionScriptView
{
	ref DialogueFW_BookRepRowController m_RowController;

	void DialogueFW_BookRepRow(string who, string rank, string score)
	{
		if (!m_RowController)
			m_RowController = DialogueFW_BookRepRowController.Cast(GetController());

		m_RowController.RepWho = who;
		m_RowController.NotifyPropertyChanged("RepWho");

		m_RowController.RepRank = rank;
		m_RowController.NotifyPropertyChanged("RepRank");

		m_RowController.RepScore = score;
		m_RowController.NotifyPropertyChanged("RepScore");
	}

	override string GetLayoutFile()
	{
		return "DialogueFramework/GUI/layouts/book/dialoguefw_book_rep_row.layout";
	}

	override typename GetControllerType()
	{
		return DialogueFW_BookRepRowController;
	}
};

class DialogueFW_BookRepRowController : ExpansionViewController
{
	string RepWho;
	string RepRank;
	string RepScore;
};

class DialogueFW_BookTabReputation : ExpansionBookMenuTabBase
{
	ref DialogueFW_BookTabReputationController m_RepController;

	void DialogueFW_BookTabReputation(ExpansionBookMenu book_menu)
	{
		m_BookMenu = book_menu;

		if (!m_RepController)
			m_RepController = DialogueFW_BookTabReputationController.Cast(GetController());

		SetView();
	}

	override string GetLayoutFile()
	{
		return "DialogueFramework/GUI/layouts/book/dialoguefw_book_tab_reputation.layout";
	}

	override typename GetControllerType()
	{
		return DialogueFW_BookTabReputationController;
	}

	override string GetTabIconName()
	{
		return DialogueFW_BookIcon.NAME;
	}

	//! The server's own wording if it set any, the mod's otherwise. A server
	//! that sets this is choosing one wording for everybody, so the fallback
	//! is the translated one.
	override string GetTabName()
	{
		DialogueMenuConfig config = DialogueManager.GetInstance().GetMenuConfig();
		if (config && config.BookTabName != "")
			return config.BookTabName;

		return "#STR_DIALOGUEFW_BOOK_TAB";
	}

	override int GetTabColor()
	{
		return ARGB(255, 0, 0, 0);
	}

	//! Hidden on a server that has no reputations set up at all, rather than
	//! offering a page that would always be empty.
	override bool CanShow()
	{
		return CountReputations() > 0;
	}

	override bool IsParentTab()
	{
		return true;
	}

	override void OnShow()
	{
		super.OnShow();
		//! Rebuilt every time it is opened: the values move while the book is
		//! shut.
		SetView();
	}

	protected int CountReputations()
	{
		DialogueManager manager = DialogueManager.GetInstance();
		if (!manager)
			return 0;

		array<ref DialogueTree> trees = manager.GetAllTrees();
		if (!trees)
			return 0;

		ref array<string> seen = new array<string>;

		foreach (DialogueTree tree : trees)
		{
			if (!tree || tree.ReputationVar == "")
				continue;
			if (seen.Find(tree.ReputationVar) == -1)
				seen.Insert(tree.ReputationVar);
		}

		return seen.Count();
	}

	//! The server's wording if it set any, the mod's own otherwise. Same
	//! fallback the window uses: with no stringtable in the PBO the engine
	//! hands the key straight back, and plain English beats a raw key.
	protected string Wording(string chosen, string key, string plain)
	{
		if (chosen != "")
			return chosen;

		string text = Widget.TranslateString(key);
		if (text == "" || text.IndexOf("STR_DIALOGUEFW_") != -1)
			return plain;

		return text;
	}

	//! Whose book this is. Shown at the top so a player reading it knows the
	//! standings are their own and nobody else's.
	protected string PlayerName()
	{
		Man man = GetGame().GetPlayer();
		if (man && man.GetIdentity())
			return man.GetIdentity().GetName();

		return "";
	}

	void SetView()
	{
		if (!m_RepController)
			return;

		DialogueMenuConfig config = DialogueManager.GetInstance().GetMenuConfig();

		string chosenTitle = "";
		string chosenName = "";
		string chosenStatus = "";
		string chosenScore = "";
		if (config)
		{
			chosenTitle = config.BookPageTitle;
			chosenName = config.BookColumnName;
			chosenStatus = config.BookColumnStatus;
			chosenScore = config.BookColumnReputation;
		}

		m_RepController.PageTitle = Wording(chosenTitle,
			"#STR_DIALOGUEFW_BOOK_TITLE", "Where you stand");
		m_RepController.NotifyPropertyChanged("PageTitle");

		m_RepController.PlayerName = PlayerName();
		m_RepController.NotifyPropertyChanged("PlayerName");

		m_RepController.ColumnName = Wording(chosenName,
			"#STR_DIALOGUEFW_BOOK_COL_NAME", "Name");
		m_RepController.NotifyPropertyChanged("ColumnName");

		m_RepController.ColumnStatus = Wording(chosenStatus,
			"#STR_DIALOGUEFW_BOOK_COL_STATUS", "Status");
		m_RepController.NotifyPropertyChanged("ColumnStatus");

		m_RepController.ColumnScore = Wording(chosenScore,
			"#STR_DIALOGUEFW_BOOK_COL_REP", "Reputation");
		m_RepController.NotifyPropertyChanged("ColumnScore");

		m_RepController.RepRows.Clear();

		DialogueManager manager = DialogueManager.GetInstance();
		if (!manager)
			return;

		array<ref DialogueTree> trees = manager.GetAllTrees();
		if (!trees)
			return;

		DialoguePlayerState state = DialogueVars.GetInstance().GetClientState();
		ref array<string> seen = new array<string>;

		foreach (DialogueTree tree : trees)
		{
			if (!tree || tree.ReputationVar == "")
				continue;
			//! One row per character, not per conversation: a character with
			//! a story tree has several, all sharing the one reputation.
			if (seen.Find(tree.ReputationVar) != -1)
				continue;

			seen.Insert(tree.ReputationVar);

			int value = 0;
			if (state)
				value = state.Get(tree.ReputationVar);

			string who = DialogueRepName.Pretty(tree.ReputationVar);
			string rank = "";

			int tierIndex = DialogueRepTierList.LabelIndexFor(tree.ReputationTiers, value);
			if (tierIndex >= 0)
			{
				string label = tree.ReputationTiers[tierIndex].Label;
				if (label != "")
					rank = DialogueLoc.ForTree(tree, DialogueLocKeys.TreeList("ReputationTiers", tierIndex), label);
			}

			//! "10 / 100" when the character has a top set, so a player can
			//! see how far there is left to go rather than a bare number
			//! that means nothing on its own.
			string score = value.ToString();
			if (tree.ReputationMax > 0)
				score = score + " / " + tree.ReputationMax.ToString();

			DialogueFW_BookRepRow row = new DialogueFW_BookRepRow(who, rank, score);
			m_RepController.RepRows.Insert(row);
		}

		Print("[DialogueFramework] [BOOK] Reputation page built with " + m_RepController.RepRows.Count() + " character(s).");
	}
};

class DialogueFW_BookTabReputationController : ExpansionViewController
{
	string PageTitle;
	string PlayerName;
	string ColumnName;
	string ColumnStatus;
	string ColumnScore;
	ref ObservableCollection<ref DialogueFW_BookRepRow> RepRows = new ObservableCollection<ref DialogueFW_BookRepRow>(this);
};

modded class ExpansionBookMenuManager
{
	ref DialogueFW_BookTabReputation m_DialogueFW_RepTab;

	override void RegisterBookMenuTabs(ExpansionBookMenu book_menu)
	{
		super.RegisterBookMenuTabs(book_menu);

		m_DialogueFW_RepTab = new DialogueFW_BookTabReputation(book_menu);
		m_Tabs.Insert(m_DialogueFW_RepTab);
	}
};
#endif
